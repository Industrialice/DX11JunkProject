#include "PreHeader.hpp"
#include "Bloom.hpp"
#include "Globals.hpp"
#include "ShadersManager.hpp"
#include "StatesManagers.hpp"
#include "RendererGlobals.hpp"

ID3D11SamplerState *CBloom::_si_samp;
ID3D11DepthStencilState *CBloom::_si_depthTestOnly;
ID3D11DepthStencilState *CBloom::_si_depthWriteOnly;
ID3D11BlendState *CBloom::_si_blend;
sdrhdl CBloom::_s_flatterBloomFinalShader;
sdrhdl CBloom::_s_flatterBloomDoNothingShader;
sdrhdl CBloom::_s_flatterOnlySampleShader;

CBloom::~CBloom()
{
    //  nothing to destruct?
}

CBloom::CBloom( bloomQuality_t nearBloomQuality, bloomQuality_t mediumBloomQuality, bloomQuality_t wideBloomQuality, ui32 width, ui32 height, ID3D11RenderTargetView *i_rtv, ID3D11ShaderResourceView *i_srv, ID3D11DepthStencilView *i_dsv )
{
    _i_rtv = i_rtv;
    _i_srv = i_srv;
    _i_dsv = i_dsv;

    _width = width;
    _height = height;

    _o_nearBloom.amount = 1.5f;
    _o_mediumBloom.amount = 0.5f;
    _o_wideBloom.amount = 0.6f;

    _is_bloomEnabled = true;
    _is_depthEnabled = true;
    _is_targetEnabled = true;

    _is_verticalEnabled = true;

    _o_nearBloom.horizontalShader[ 2 ] = ShadersManager::AcquireByName( "flatter_bloom_near_horizontal_high" );
    _o_nearBloom.horizontalShader[ 1 ] = ShadersManager::AcquireByName( "flatter_bloom_near_horizontal_medium" );
    _o_nearBloom.horizontalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_near_horizontal_low" );
    _o_nearBloom.verticalShader[ 2 ] = ShadersManager::AcquireByName( "flatter_bloom_near_vertical_high" );
    _o_nearBloom.verticalShader[ 1 ] = ShadersManager::AcquireByName( "flatter_bloom_near_vertical_medium" );
    _o_nearBloom.verticalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_near_vertical_low" );
    //_o_nearBloom.verticalShader[ 1 ] = _s_flatterBloomDoNothingShader;

    _o_mediumBloom.horizontalShader[ 1 ] = ShadersManager::AcquireByName( "flatter_bloom_medium_horizontal_medium" );
    _o_mediumBloom.horizontalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_medium_horizontal_low" );
    _o_mediumBloom.verticalShader[ 1 ] = ShadersManager::AcquireByName( "flatter_bloom_medium_vertical_medium" );
    _o_mediumBloom.verticalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_medium_vertical_low" );

    _o_wideBloom.horizontalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_wide_horizontal_low" );
    _o_wideBloom.verticalShader[ 0 ] = ShadersManager::AcquireByName( "flatter_bloom_wide_vertical_low" );

    CreateRT( &_glowmapTexs[ 0 ], width, height );
    CreateRT( &_glowmapTexs[ 1 ], width / 2, height / 2 );
    CreateRT( &_glowmapTexs[ 2 ], width / 4, height / 4 );

    D3D11_VIEWPORT vp;
    vp.MaxDepth = 1.f;
    vp.MinDepth = 0.f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;

    vp.Height = height;
    vp.Width = width;
    _vps[ 0 ] = vp;
    vp.Height = height / 2;
    vp.Width = width / 2;
    _vps[ 1 ] = vp;
    vp.Height = height / 4;
    vp.Width = width / 4;
    _vps[ 2 ] = vp;

    NearQualitySet( nearBloomQuality );
    MediumQualitySet( mediumBloomQuality );
    WideQualitySet( wideBloomQuality );

    RenderingStatesSet( RStates::target | RStates::depthTest | RStates::depthWrite, false, false );
}

void CBloom::RenderingStatesSet( RStates::rstate_t states, bln is_cache /* = true */, bln is_makeCurrentAsLast /* = true */ )
{
    if( is_makeCurrentAsLast )
    {
        _rstatesLast = _rstatesCur;
    }

    if( is_cache && _rstatesCur == states )
    {
        return;
    }

    bln is_depthNeedToBeEnabled = states & (RStates::depthTest | RStates::depthWrite);

    if( !is_cache || (_rstatesCur & (RStates::target | RStates::glowmap)) != (states & (RStates::target | RStates::glowmap)) || is_depthNeedToBeEnabled != _is_depthEnabled )
    {
        ID3D11RenderTargetView *ai_rts[ 2 ] = {};
        if( states & RStates::target )
        {
            ai_rts[ 0 ] = _is_targetEnabled ? _i_rtv.Get() : 0;
        }
        if( states & RStates::glowmap )
        {
            ai_rts[ (states & RStates::target) != 0 ] = _is_bloomActive ? _glowmapTexs[ 0 ].i_rtv.Get() : 0;
        }
        RendererGlobals::i_ImContext->OMSetRenderTargets( 2, ai_rts, is_depthNeedToBeEnabled ? _i_dsv.Get() : nullptr );
    }

    if( !is_cache || is_depthNeedToBeEnabled && (_rstatesCur & (RStates::depthTest | RStates::depthWrite)) != (states & (RStates::depthTest | RStates::depthWrite)) )
    {
        ID3D11DepthStencilState *i_dss;
        if( states & RStates::depthWrite )
        {
            i_dss = states & RStates::depthTest ? RendererGlobals::i_DepthStencilDefault : _si_depthWriteOnly;
        }
        else
        {
            i_dss = (states & RStates::depthTest) ? _si_depthTestOnly : nullptr;
        }
        RendererGlobals::i_ImContext->OMSetDepthStencilState( i_dss, 0xFFffFFff );
    }

    _rstatesCur = states;
    
    _is_depthEnabled = is_depthNeedToBeEnabled;
}

void CBloom::RenderingStatesRestore()
{
    RenderingStatesSet( _rstatesLast );
}

RStates::rstate_t CBloom::RenderingStatesGet() const
{
    return _rstatesCur;
}

void CBloom::IsTargetEnabledSet( bln is_enabled )
{
    if( _is_targetEnabled == is_enabled )
    {
        return;
    }
    _is_targetEnabled = is_enabled;
    RenderingStatesSet( _rstatesCur, false, false );
}

bln CBloom::IsTargetEnabledGet() const
{
    return _is_targetEnabled;
}

void CBloom::IsEnabledSet( bln is_enabled )
{
    if( _is_bloomEnabled == is_enabled )
    {
        return;
    }
    _is_bloomEnabled = is_enabled;
    _is_bloomActive = _is_bloomEnabled && (_o_nearBloom.quality != BloomQuality::disabled || _o_mediumBloom.quality != BloomQuality::disabled || _o_wideBloom.quality != BloomQuality::disabled);
    if( !is_enabled )
    {
        RenderingStatesSet( _rstatesCur, false, false );
    }
}

bln CBloom::IsEnabledGet() const
{
    return _is_bloomEnabled;
}
    
void CBloom::IsVerticalEnabledSet( bln is_enabled )
{
    _is_verticalEnabled = is_enabled;
}

bln CBloom::IsVerticalEnabledGet() const
{
    return _is_verticalEnabled;
}

void CBloom::NearQualitySet( bloomQuality_t quality )
{
    quality = quality > BloomQuality::high ? BloomQuality::high : quality;
    if( _o_nearBloom.quality != quality )
    {
        _o_nearBloom.quality = quality;
        _is_lowRequired = _o_nearBloom.quality == BloomQuality::low || _o_mediumBloom.quality == BloomQuality::low || _o_wideBloom.quality == BloomQuality::low;
        _is_mediumRequired = _o_nearBloom.quality == BloomQuality::medium || _o_mediumBloom.quality == BloomQuality::medium;
        _is_mediumRequired |= _is_lowRequired;
        _is_bloomActive = _is_bloomEnabled && (_o_nearBloom.quality != BloomQuality::disabled || _o_mediumBloom.quality != BloomQuality::disabled || _o_wideBloom.quality != BloomQuality::disabled);
        CreateSources( &_o_nearBloom );
    }
}

CBloom::bloomQuality_t CBloom::NearQualityGet() const
{
    return _o_nearBloom.quality;
}

void CBloom::NearAmountSet( f32 amount )
{
    amount = MAX( amount, 0 );
    _o_nearBloom.amount = amount;
}

f32 CBloom::NearAmountGet() const
{
    return _o_nearBloom.amount;
}

void CBloom::MediumQualitySet( bloomQuality_t quality )
{
    quality = quality > BloomQuality::medium ? BloomQuality::medium : quality;
    if( _o_mediumBloom.quality != quality )
    {
        _o_mediumBloom.quality = quality;
        _is_lowRequired = _o_nearBloom.quality == BloomQuality::low || _o_mediumBloom.quality == BloomQuality::low || _o_wideBloom.quality == BloomQuality::low;
        _is_mediumRequired = _o_nearBloom.quality == BloomQuality::medium || _o_mediumBloom.quality == BloomQuality::medium;
        _is_mediumRequired |= _is_lowRequired;
        _is_bloomActive = _is_bloomEnabled && (_o_nearBloom.quality != BloomQuality::disabled || _o_mediumBloom.quality != BloomQuality::disabled || _o_wideBloom.quality != BloomQuality::disabled);
        CreateSources( &_o_mediumBloom );
    }
}

CBloom::bloomQuality_t CBloom::MediumQualityGet() const
{
    return _o_mediumBloom.quality;
}

void CBloom::MediumAmountSet( f32 amount )
{
    amount = MAX( amount, 0 );
    _o_mediumBloom.amount = amount;
}

f32 CBloom::MediumAmountGet() const
{
    return _o_mediumBloom.amount;
}

void CBloom::WideQualitySet( bloomQuality_t quality )
{
    quality = quality > BloomQuality::low ? BloomQuality::low : quality;
    if( _o_wideBloom.quality != quality )
    {
        _o_wideBloom.quality = quality;
        _is_lowRequired = _o_nearBloom.quality == BloomQuality::low || _o_mediumBloom.quality == BloomQuality::low || _o_wideBloom.quality == BloomQuality::low;
        _is_mediumRequired = _o_nearBloom.quality == BloomQuality::medium || _o_mediumBloom.quality == BloomQuality::medium;
        _is_mediumRequired |= _is_lowRequired;
        _is_bloomActive = _is_bloomEnabled && (_o_nearBloom.quality != BloomQuality::disabled || _o_mediumBloom.quality != BloomQuality::disabled || _o_wideBloom.quality != BloomQuality::disabled);
        CreateSources( &_o_wideBloom );
    }
}

CBloom::bloomQuality_t CBloom::WideQualityGet() const
{
    return _o_wideBloom.quality;
}

void CBloom::WideAmountSet( f32 amount )
{
    amount = MAX( amount, 0 );
    _o_wideBloom.amount = amount;
}

f32 CBloom::WideAmountGet() const
{
    return _o_wideBloom.amount;
}

void CBloom::FlushToRT( ID3D11RenderTargetView *rt )
{
	if( rt == nullptr || rt == _i_rtv )
	{
		return;
	}

    static ID3D11ShaderResourceView *const ai_nullviews[ 3 ];
    RendererGlobals::i_ImContext->PSSetShaderResources( 0, 3, ai_nullviews );
	ShadersManager::ApplyShader( _s_flatterBloomDoNothingShader, false );
	RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFFFFFFF );
	RendererGlobals::SetViewports( 1, &RendererGlobals::o_ScreenViewport );
	RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &rt, 0 );
	RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, _i_srv.Addr() );
	RendererGlobals::i_ImContext->Draw( 3, 0 );
	RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, ai_nullviews );
	RendererGlobals::i_ImContext->ClearRenderTargetView( _i_rtv, Colora::Black.arr );
	RendererGlobals::SetViewports( 1, &_vps[ 0 ] );

    RenderingStatesSet( _rstatesCur, false, false );
}

void CBloom::FlushGlowMap()
{
    if( !_is_bloomActive )
    {
        return;
    }

    RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFFFFFFF );
    RendererGlobals::SetPrimitiveTopologyOr( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &_si_samp );

    if( _is_mediumRequired )
    {
        RenderTo( &_glowmapTexs[ 0 ], &_glowmapTexs[ 1 ], &_vps[ 1 ], _s_flatterBloomDoNothingShader );
    }
    if( _is_lowRequired )
    {
        RenderTo( &_glowmapTexs[ 1 ], &_glowmapTexs[ 2 ], &_vps[ 2 ], _s_flatterBloomDoNothingShader );
    }

    ApplyBlur( &_o_nearBloom );
    ApplyBlur( &_o_mediumBloom );
    ApplyBlur( &_o_wideBloom );

    f32 nearAmount = _o_nearBloom.amount;
    f32 mediumAmount = _o_mediumBloom.amount;
    f32 wideAmount = _o_wideBloom.amount;
    if( _o_mediumBloom.quality != BloomQuality::disabled )
    {
        nearAmount *= 0.75f;
    }
    if( _o_wideBloom.quality != BloomQuality::disabled )
    {
        nearAmount *= 0.75f;
        mediumAmount *= 0.75f;
    }
    
    D3D11_MAPPED_SUBRESOURCE o_sr;
    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &vec3( nearAmount, mediumAmount, wideAmount ), sizeof(vec3) );
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

    RendererGlobals::i_ImContext->OMSetBlendState( _si_blend, 0, 0xFFFFFFFF );
    ShadersManager::ApplyShader( _s_flatterBloomFinalShader, false );
    RendererGlobals::SetViewports( 1, &_vps[ 0 ] );
    RendererGlobals::i_ImContext->OMSetRenderTargets( 1, _i_rtv.Addr(), 0 );
    ID3D11ShaderResourceView *const ai_views[ 3 ] = 
    { 
        _is_verticalEnabled ? _o_nearBloom.verticalTex.i_srv : _o_nearBloom.horizontalTex.i_srv,
        _is_verticalEnabled ? _o_mediumBloom.verticalTex.i_srv : _o_mediumBloom.horizontalTex.i_srv,
        _is_verticalEnabled ? _o_wideBloom.verticalTex.i_srv : _o_wideBloom.horizontalTex.i_srv
    };
    RendererGlobals::i_ImContext->PSSetShaderResources( 0, 3, ai_views );
    RendererGlobals::i_ImContext->Draw( 3, 0 );
    static ID3D11ShaderResourceView *const ai_nullviews[ 3 ];
    RendererGlobals::i_ImContext->PSSetShaderResources( 0, 3, ai_nullviews );

    RendererGlobals::i_ImContext->ClearRenderTargetView( _glowmapTexs[ 0 ].i_rtv, Colora::Black.arr );

    RenderingStatesSet( _rstatesCur, false, false );
}

#pragma optimize( "s", on )

void CBloom::Private::Initialize()  //  static
{
    D3D11_SAMPLER_DESC o_samp;
    o_samp.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    o_samp.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    o_samp.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    o_samp.BorderColor[ 0 ] = 0.f;
    o_samp.BorderColor[ 1 ] = 0.f;
    o_samp.BorderColor[ 2 ] = 0.f;
    o_samp.BorderColor[ 3 ] = 0.f;
    o_samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_samp.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    o_samp.MaxAnisotropy = 1;
    o_samp.MaxLOD = FLT_MAX;
    o_samp.MinLOD = -FLT_MAX;
    o_samp.MipLODBias = 0.f;
    _si_samp = SamplersManager::GetState( &o_samp );

    const D3D11_DEPTH_STENCILOP_DESC o_defaultStencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
    D3D11_DEPTH_STENCIL_DESC o_dsd;
    o_dsd.BackFace = o_defaultStencilOp;
    o_dsd.DepthEnable = true;
    o_dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    o_dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    o_dsd.FrontFace = o_defaultStencilOp;
    o_dsd.StencilEnable = false;
    o_dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    o_dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    DXHRCHECK( RendererGlobals::i_Device->CreateDepthStencilState( &o_dsd, &_si_depthTestOnly ) );

    o_dsd.DepthFunc = D3D11_COMPARISON_ALWAYS;
    o_dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DXHRCHECK( RendererGlobals::i_Device->CreateDepthStencilState( &o_dsd, &_si_depthWriteOnly ) );

    D3D11_BLEND_DESC o_blend = {};
    o_blend.AlphaToCoverageEnable = false;
    o_blend.IndependentBlendEnable = false;
    o_blend.RenderTarget[ 0 ].BlendEnable = true;
    o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    _si_blend = BlendStatesManager::GetState( &o_blend );

    _s_flatterBloomFinalShader = ShadersManager::AcquireByName( "flatter_bloom_final" );
    _s_flatterBloomDoNothingShader = ShadersManager::AcquireByName( "flatter_bloom_nothing" );
	_s_flatterOnlySampleShader = ShadersManager::AcquireByName( "flatter_onlySample" );
}

NOINLINE void CBloom::CreateSources( SBloom *po_bloom )
{
    po_bloom->horizontalTex.i_rtv.Release();
    po_bloom->horizontalTex.i_srv.Release();
    po_bloom->verticalTex.i_rtv.Release();
    po_bloom->verticalTex.i_srv.Release();

    if( po_bloom->quality == CBloom::BloomQuality::disabled )
    {
        return;
    }

    CreateRT( &po_bloom->horizontalTex, _width / (4 / po_bloom->quality ), _height / (4 / po_bloom->quality ) );
    CreateRT( &po_bloom->verticalTex, _width / (4 / po_bloom->quality ), _height / (4 / po_bloom->quality ) );
}

NOINLINE void CBloom::CreateRT( SRT *rt, ui32 width, ui32 height )
{
    D3D11_TEXTURE2D_DESC o_td;
    o_td.ArraySize = 1;
    o_td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    o_td.CPUAccessFlags = 0;
    o_td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    o_td.Height = height;
    o_td.MipLevels = 1;
    o_td.MiscFlags = 0;
    o_td.Usage = D3D11_USAGE_DEFAULT;
    o_td.Width = width;
    o_td.SampleDesc.Count = 1;
    o_td.SampleDesc.Quality = 0;

    D3D11_SHADER_RESOURCE_VIEW_DESC o_rvd;
    o_rvd.Format = o_td.Format;
    o_rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    o_rvd.Texture1D.MipLevels = o_td.MipLevels;
    o_rvd.Texture1D.MostDetailedMip = 0;

    COMUniquePtr< ID3D11Texture2D > i_tex;
    DXHRCHECK( RendererGlobals::i_Device->CreateTexture2D( &o_td, 0, i_tex.AddrModifiable() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateRenderTargetView( i_tex, 0, rt->i_rtv.AddrModifiable() ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( i_tex, &o_rvd, rt->i_srv.AddrModifiable() ) );
}

#pragma optimize( "", on )

void CBloom::ApplyBlur( SBloom *po_bloom )
{
    if( po_bloom->quality == CBloom::BloomQuality::disabled )
    {
        return;
    }

    const CBloom::bloomQuality_t qualNor = CBloom::BloomQuality::high - po_bloom->quality;

    RenderTo( &_glowmapTexs[ qualNor ], &po_bloom->horizontalTex, &_vps[ qualNor ], po_bloom->horizontalShader[ po_bloom->quality - 1 ] );

    if( _is_verticalEnabled )
    {
        RenderTo( &po_bloom->horizontalTex, &po_bloom->verticalTex, &_vps[ qualNor ], po_bloom->verticalShader[ po_bloom->quality - 1 ] );
    }
}

void CBloom::RenderTo( SRT *sourceRt, SRT *destRt, const D3D11_VIEWPORT *vp, sdrhdl shader )
{
    RendererGlobals::SetViewports( 1, vp );

    ShadersManager::ApplyShader( shader, false );
    RendererGlobals::i_ImContext->OMSetRenderTargets( 1, destRt->i_rtv.Addr(), 0 );
    RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, sourceRt->i_srv.Addr() );
    RendererGlobals::i_ImContext->Draw( 3, 0 );
}