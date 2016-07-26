#include "PreHeader.hpp"
#include "PostProcess.hpp"
#include "Globals.hpp"
#include "ShadersManager.hpp"
#include "Bloom.hpp"
#include "StatesManagers.hpp"
#include "TextureLoader.hpp"
#include "RendererGlobals.hpp"

namespace
{
    CIDManagerSequential IdManager;

    CVec < byte > EffectsArray;
    ui32 EffectsRegistered;

    ID3D11ShaderResourceView *i_RandTex;
    const ui32 RandTexWidth = 32;
	const ui32 RandTexHeight = 64;
    sdrhdl FlatterNoiseShader;
    ID3D11BlendState *i_RandBlend;

    ID3D11ShaderResourceView *i_RandColoredTex;
    const ui32 RandColoredTexWidth = 32;
	const ui32 RandColoredTexHeight = 64;
    sdrhdl FlatterColoredNoiseShader;
    ID3D11BlendState *i_RandColoredBlend;

    sdrhdl FlatterOnlySampleShader;

    sdrhdl FlatterBlackWhiteShader;
    ID3D11RenderTargetView *i_BlackWhiteRTV;
    ID3D11ShaderResourceView *i_BlackWhiteSRV;
    ID3D11SamplerState *i_BlackWhiteSamp;

    ID3D11SamplerState *i_TexSamp;

    sdrhdl FlatterGrayShader;
    sdrhdl FlatterGrayFactorShader;

    ID3D11BlendState *i_InvertBlend;
    sdrhdl FlatterEmptyShader;

    ID3D11BlendState *i_PrettyBlend;

    sdrhdl FlatterColorShader;

    sdrhdl FlatterTextureShader;
}

static byte *GetEffectMemByIndex( ui32 index )
{
    ASSUME( index < EffectsRegistered );
    for( ui32 offset = 0; ; )
    {
        PostProcess::Effects::Private::EffectBase *effect = (PostProcess::Effects::Private::EffectBase *)&EffectsArray[ offset ];
        if( !index )
        {
            return (byte *)effect; 
        }
        offset += effect->_procFunc( effect, PostProcess::Effects::Private::ProcessActions::nothing );
        --index;
    }
}

static byte *GetEffectMemByID( id_t id )
{
    for( ui32 offset = 0; ; )
    {
        ASSUME( offset < EffectsArray.Size() );
        PostProcess::Effects::Private::EffectBase *effect = (PostProcess::Effects::Private::EffectBase *)&EffectsArray[ offset ];
        if( effect->_id == id )
        {
            return (byte *)effect;
        }
        offset += effect->_procFunc( effect, PostProcess::Effects::Private::ProcessActions::nothing );
    }
}

void *PostProcess::LockEffectByIndex( ui32 index )
{
    ASSUME( index < EffectsRegistered );
    return GetEffectMemByIndex( index );
}

void *PostProcess::LockEffectByID( id_t id )
{
    ASSUME( id != id_t_null );
    return GetEffectMemByID( id );
}

void PostProcess::Unlock()
{
}

void PostProcess::EffectSetByIndex( ui32 index, const void *effect )
{
    ASSUME( index < EffectsRegistered && effect );

    PostProcess::Effects::Private::EffectBase *curEffect = (PostProcess::Effects::Private::EffectBase *)GetEffectMemByIndex( index );
    PostProcess::Effects::Private::EffectBase *repEffect = (PostProcess::Effects::Private::EffectBase *)effect;

    uiw curEffectSize = curEffect->_procFunc( 0, Effects::Private::ProcessActions::nothing );
    uiw repEffectSize = repEffect->_procFunc( 0, Effects::Private::ProcessActions::nothing );

    if( curEffectSize != repEffectSize )
    {
        ui32 offsetToEffect = (byte *)curEffect - &EffectsArray.Front();

        if( curEffectSize > repEffectSize )
        {
            EffectsArray.Erase( offsetToEffect, curEffectSize - repEffectSize );
        }
        else
        {
            EffectsArray.InsertNum( offsetToEffect, repEffectSize - curEffectSize, false );
        }
    }

    _MemCpy( curEffect, repEffect, repEffectSize );
}

void PostProcess::EffectSetByID( id_t id, const void *effect )
{
    /*ASSUME( id != id_t_null && effect );
    */
}

id_t PostProcess::EffectPush( const void *raw )
{
    const Effects::Private::EffectBase *effect = (Effects::Private::EffectBase *)raw;
    uiw effectSize = effect->_procFunc( 0, Effects::Private::ProcessActions::nothing );
    uiw curEffectsArraySize = EffectsArray.Size();
    EffectsArray.Resize( curEffectsArraySize + effectSize );
    Effects::Private::EffectBase *placed = (Effects::Private::EffectBase *)(&EffectsArray[ 0 ] + curEffectsArraySize);
    _MemCpy( placed, effect, effectSize );
    ++EffectsRegistered;
    bln reserveResult = IdManager.ReserveID( placed->_id = IdManager.FindFirstFreeID() );
    ASSUME( reserveResult );
    return placed->_id;
}

void PostProcess::EffectDeleteByIndex( ui32 index )
{
}

void PostProcess::EffectDeleteByID( id_t id )
{
    /*for( ui32 index = 0; index < o_Effects.Size(); ++index )
    {
        if( o_Effects[ index ].id == id )
        {
            o_Effects[ index ].Delete();
            o_Effects.Erase( index, 1 );
            return;
        }
    }
    DBGBREAK;*/
}

void PostProcess::EffectPop()
{
    /*if( o_Effects.Size() )
    {
        o_Effects.Back().Delete();
        o_Effects.Pop();
    }*/
}

void PostProcess::EffectsClear()
{
    /*for( ui32 index = 0; index < o_Effects.Size(); ++index )
    {
        o_Effects[ index ].Delete();
    }
    o_Effects.Clear();*/
}

ui32 PostProcess::EffectsCount()
{
    return EffectsRegistered;
}

void PostProcess::Private::Draw()
{
    if( !EffectsRegistered )
    {
        return;
    }

    RendererGlobals::CurrentBloom->RenderingStatesSet( RStates::target );

    RendererGlobals::SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    for( ui32 offset = 0; offset < EffectsArray.Size(); )
    {
        Effects::Private::EffectBase *effect = (Effects::Private::EffectBase *)&EffectsArray[ offset ];
        offset += effect->_procFunc( effect, effect->_is_enabled ? Effects::Private::ProcessActions::draw : Effects::Private::ProcessActions::nothing );
    }
}

#pragma optimize( "s", on )

void PostProcess::Private::Initialize()
{
    UniquePtr< ui8, Heap::DefaultDeleter > po_randData = Heap::Alloc<ui8>( RandTexWidth * RandTexHeight );

    for( ui32 h = 0; h < RandTexHeight; ++h )
    {
        for( ui32 w = 0; w < RandTexWidth; ++w )    
        {
            po_randData[ h * RandTexWidth + w ] = Funcs::RandomRangeUI32( 0, 15 ) << 4 | Funcs::RandomRangeUI32( 0, 15 );
        }
    }

    D3D11_SUBRESOURCE_DATA o_init;
    o_init.pSysMem = po_randData;
    o_init.SysMemPitch = sizeof(ui8) * RandTexWidth;
    o_init.SysMemSlicePitch = 0;

    D3D11_TEXTURE2D_DESC o_td;
    o_td.ArraySize = 1;
    o_td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    o_td.CPUAccessFlags = 0;
    o_td.Format = DXGI_FORMAT_R8_UINT;
    o_td.Height = RandTexHeight;
    o_td.MipLevels = 1;
    o_td.MiscFlags = 0;
    o_td.Usage = D3D11_USAGE_IMMUTABLE;
    o_td.Width = RandTexWidth;
    o_td.SampleDesc.Count = 1;
    o_td.SampleDesc.Quality = 0;

    ID3D11Texture2D *i_randomTex;
    DXHRCHECK( RendererGlobals::i_Device->CreateTexture2D( &o_td, &o_init, &i_randomTex ) );

    D3D11_SHADER_RESOURCE_VIEW_DESC o_rvd;
    o_rvd.Format = o_td.Format;
    o_rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    o_rvd.Texture1D.MipLevels = o_td.MipLevels;
    o_rvd.Texture1D.MostDetailedMip = 0;

    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( i_randomTex, &o_rvd, &i_RandTex ) );

    i_randomTex->Release();

    D3D11_BLEND_DESC o_randBlend = {};
    o_randBlend.AlphaToCoverageEnable = false;
    o_randBlend.IndependentBlendEnable = false;
    o_randBlend.RenderTarget[ 0 ].BlendEnable = true;
    o_randBlend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_randBlend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_randBlend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_randBlend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_randBlend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_randBlend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_randBlend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    i_RandBlend = BlendStatesManager::GetState( &o_randBlend );

	{
		UniquePtr< I32Color, Heap::DefaultDeleter > po_randData = Heap::Alloc<I32Color>( RandTexWidth * RandTexHeight );

		for( ui32 h = 0; h < RandColoredTexHeight; ++h )
		{
			for( ui32 w = 0; w < RandColoredTexWidth; ++w )    
			{
				ui32 r = Funcs::RandomRangeUI32( 0, ui8_max );
				ui32 g = Funcs::RandomRangeUI32( 0, ui8_max );
				ui32 b = Funcs::RandomRangeUI32( 0, ui8_max );
				po_randData[ h * RandColoredTexWidth + w ] = I32Color( r, g, b, 255 );
			}
		}

		D3D11_SUBRESOURCE_DATA o_init;
		o_init.pSysMem = po_randData;
		o_init.SysMemPitch = sizeof(I32Color) * RandColoredTexWidth;
		o_init.SysMemSlicePitch = 0;

		D3D11_TEXTURE2D_DESC o_td;
		o_td.ArraySize = 1;
		o_td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		o_td.CPUAccessFlags = 0;
		o_td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		o_td.Height = RandColoredTexHeight;
		o_td.MipLevels = 1;
		o_td.MiscFlags = 0;
		o_td.Usage = D3D11_USAGE_IMMUTABLE;
		o_td.Width = RandColoredTexWidth;
		o_td.SampleDesc.Count = 1;
		o_td.SampleDesc.Quality = 0;

		ID3D11Texture2D *i_randomTex;
		DXHRCHECK( RendererGlobals::i_Device->CreateTexture2D( &o_td, &o_init, &i_randomTex ) );

		D3D11_SHADER_RESOURCE_VIEW_DESC o_rvd;
		o_rvd.Format = o_td.Format;
		o_rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		o_rvd.Texture1D.MipLevels = o_td.MipLevels;
		o_rvd.Texture1D.MostDetailedMip = 0;

		DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( i_randomTex, &o_rvd, &i_RandColoredTex ) );

		i_randomTex->Release();

		D3D11_BLEND_DESC o_randBlend = {};
		o_randBlend.AlphaToCoverageEnable = false;
		o_randBlend.IndependentBlendEnable = false;
		o_randBlend.RenderTarget[ 0 ].BlendEnable = true;
		o_randBlend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		o_randBlend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		o_randBlend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
		o_randBlend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
		o_randBlend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
		o_randBlend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		o_randBlend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
		i_RandColoredBlend = BlendStatesManager::GetState( &o_randBlend );
	}

    D3D11_BLEND_DESC o_invertBlend = {};
    o_invertBlend.AlphaToCoverageEnable = false;
    o_invertBlend.IndependentBlendEnable = false;
    o_invertBlend.RenderTarget[ 0 ].BlendEnable = true;
    o_invertBlend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    o_invertBlend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    o_invertBlend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_SUBTRACT;
    o_invertBlend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_invertBlend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_invertBlend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_invertBlend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    DXHRCHECK( RendererGlobals::i_Device->CreateBlendState( &o_invertBlend, &i_InvertBlend ) );

    D3D11_BLEND_DESC o_prettyBlend = {};
    o_prettyBlend.AlphaToCoverageEnable = false;
    o_prettyBlend.IndependentBlendEnable = false;
    o_prettyBlend.RenderTarget[ 0 ].BlendEnable = true;
    o_prettyBlend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
    o_prettyBlend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_DEST_COLOR;
    o_prettyBlend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
    o_prettyBlend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_prettyBlend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_prettyBlend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_prettyBlend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    i_PrettyBlend = BlendStatesManager::GetState( &o_prettyBlend );

    //D3D11_TEXTURE2D_DESC o_td;
    o_td.ArraySize = 1;
    o_td.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    o_td.CPUAccessFlags = 0;
    o_td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    o_td.Height = Globals::Height;
    o_td.MipLevels = 1;
    o_td.MiscFlags = 0;
    o_td.Usage = D3D11_USAGE_DEFAULT;
    o_td.Width = Globals::Width;
    o_td.SampleDesc.Count = 1;
    o_td.SampleDesc.Quality = 0;

    ID3D11Texture2D *i_bwTex;
    DXHRCHECK( RendererGlobals::i_Device->CreateTexture2D( &o_td, 0, &i_bwTex ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateRenderTargetView( i_bwTex, 0, &i_BlackWhiteRTV ) );

    //D3D11_SHADER_RESOURCE_VIEW_DESC o_rvd;
    o_rvd.Format = o_td.Format;
    o_rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    o_rvd.Texture1D.MipLevels = o_td.MipLevels;
    o_rvd.Texture1D.MostDetailedMip = 0;

    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( i_bwTex, &o_rvd, &i_BlackWhiteSRV ) );

    D3D11_SAMPLER_DESC o_samp;
    o_samp.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_samp.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_samp.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
    o_samp.BorderColor[ 0 ] = 1.f;
    o_samp.BorderColor[ 1 ] = 1.f;
    o_samp.BorderColor[ 2 ] = 1.f;
    o_samp.BorderColor[ 3 ] = 1.f;
    o_samp.ComparisonFunc = D3D11_COMPARISON_NEVER;
    o_samp.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    o_samp.MaxAnisotropy = 16;
    o_samp.MaxLOD = FLT_MAX;
    o_samp.MinLOD = -FLT_MAX;
    o_samp.MipLODBias = 0.f;
    i_BlackWhiteSamp = SamplersManager::GetState( &o_samp );

    o_samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    i_TexSamp = SamplersManager::GetState( &o_samp );

    FlatterNoiseShader = ShadersManager::AcquireByName( "flatter_noise" );
    FlatterColoredNoiseShader = ShadersManager::AcquireByName( "flatter_colorednoise" );
    FlatterEmptyShader = ShadersManager::AcquireByName( "flatter_empty" );
    FlatterOnlySampleShader = ShadersManager::AcquireByName( "flatter_onlySample" );
    FlatterBlackWhiteShader = ShadersManager::AcquireByName( "flatter_blackwhite" );
    FlatterGrayShader = ShadersManager::AcquireByName( "flatter_gray" );
    FlatterGrayFactorShader = ShadersManager::AcquireByName( "flatter_grayFactor" );
    FlatterColorShader = ShadersManager::AcquireByName( "flatter_color" );
    FlatterTextureShader = ShadersManager::AcquireByName( "flatter_texture" );
}

#pragma optimize( "", on )

uiw PostProcess::Effects::FilmGrain::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::FilmGrain *effect = (PostProcess::Effects::FilmGrain *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        RendererGlobals::i_ImContext->OMSetBlendState( i_RandBlend, 0, 0xFFFFFFFF );

        ShadersManager::ApplyShader( FlatterNoiseShader, false );

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof(vec4) * 4096 );
        vec2 wh = vec2( (f32)Globals::Width / RandTexWidth, (f32)Globals::Height / RandTexHeight );
        m2x2 o_texTransform;
        LiceMath::M2x2ScaleRotate( &o_texTransform, wh.x, wh.y, Funcs::RandomRangeF32( 0, f32_pi * 2 ) );
		vsTarget.Append( (byte *)&vec4( o_texTransform.e00, o_texTransform.e10, Funcs::RandomF32(), Funcs::RandomF32() ), sizeof(vec4) );
		vsTarget.Append( (byte *)&vec2( o_texTransform.e01, o_texTransform.e11 ), sizeof(vec2) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
		psTarget.Append( (byte *)&vec2( RandTexWidth, RandTexHeight ), sizeof(vec2) );
		psTarget.Append( (byte *)&effect->_noiseScale, sizeof(f32) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_RandTex );

        RendererGlobals::i_ImContext->Draw( 3, 0 );
    }

    return sizeof(PostProcess::Effects::FilmGrain);
}

uiw PostProcess::Effects::FilmGrainColored::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::FilmGrainColored *effect = (PostProcess::Effects::FilmGrainColored *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        RendererGlobals::i_ImContext->OMSetBlendState( i_RandColoredBlend, 0, 0xFFFFFFFF );

        ShadersManager::ApplyShader( FlatterColoredNoiseShader, false );

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
        vec2 wh = vec2( (f32)Globals::Width / RandColoredTexWidth, (f32)Globals::Height / RandColoredTexHeight );
        m2x2 o_texTransform;
        LiceMath::M2x2ScaleRotate( &o_texTransform, wh.x, wh.y, Funcs::RandomRangeF32( 0, f32_pi * 2 ) );
		vsTarget.Append( (byte *)&vec4( o_texTransform.e00, o_texTransform.e10, Funcs::RandomF32(), Funcs::RandomF32() ), sizeof(vec4) );
		vsTarget.Append( (byte *)&vec2( o_texTransform.e01, o_texTransform.e11 ), sizeof(vec2) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
		psTarget.Append( (byte *)&vec2( RandColoredTexWidth, RandColoredTexHeight ), sizeof(vec2) );
		psTarget.Append( (byte *)&effect->_noiseScale, sizeof(f32) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );
		
        RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_TexSamp );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_RandColoredTex );

        RendererGlobals::i_ImContext->Draw( 3, 0 );
    }

    return sizeof(PostProcess::Effects::FilmGrain);
}

uiw PostProcess::Effects::BlackWhite::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::BlackWhite *effect = (PostProcess::Effects::BlackWhite *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
		psTarget.Append( (byte *)&effect->_o_colorLow, sizeof(f96color) );
		psTarget.Append( 0 );
		psTarget.Append( (byte *)&effect->_o_colorHigh, sizeof(f96color) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );

		vsTarget.Append( (byte *)&effect->_rect, sizeof(ScreenRect) );

        vec4 *v = (vec4 *)(vsTarget.Data() + vsTarget.Size());
		vsTarget.AppendNum( sizeof(vec4[ 3 ]) );
        v[ 0 ][ 0 ] = effect->_transformation.e00;
        v[ 0 ][ 1 ] = effect->_transformation.e10;
        v[ 0 ][ 2 ] = effect->_transformation.e20;
        v[ 1 ][ 0 ] = effect->_transformation.e01;
        v[ 1 ][ 1 ] = effect->_transformation.e11;
        v[ 1 ][ 2 ] = effect->_transformation.e21;
        v[ 2 ][ 0 ] = effect->_transformation.e02;
        v[ 2 ][ 1 ] = effect->_transformation.e12;
        v[ 2 ][ 2 ] = effect->_transformation.e22;

        ui32 is_cutTexcoord = 1;
		vsTarget.Append( (byte *)&is_cutTexcoord, 4 );

        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        ShadersManager::ApplyShader( FlatterBlackWhiteShader, false );

        RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFFFFFFF );
        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &i_BlackWhiteRTV, 0 );
        RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_BlackWhiteSamp );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &RendererGlobals::i_MainSRV );

        RendererGlobals::i_ImContext->Draw( 4, 0 );

        static ID3D11ShaderResourceView *i_nullView;
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_nullView );

        ShadersManager::ApplyShader( FlatterOnlySampleShader, false );
    
        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &RendererGlobals::i_MainRenderTargetView, 0 );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_BlackWhiteSRV );

        RendererGlobals::i_ImContext->Draw( 4, 0 );

        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_nullView );
    }

    return sizeof(PostProcess::Effects::BlackWhite);
}

uiw PostProcess::Effects::InvertColors::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::InvertColors *effect = (PostProcess::Effects::InvertColors *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );

		vsTarget.Append( (byte *)&effect->_rect, sizeof(ScreenRect) );

        vec4 *v = (vec4 *)(vsTarget.Data() + vsTarget.Size());
		vsTarget.AppendNum( sizeof(vec4[ 3 ]));
        v[ 0 ][ 0 ] = effect->_transformation.e00;
        v[ 0 ][ 1 ] = effect->_transformation.e10;
        v[ 0 ][ 2 ] = effect->_transformation.e20;
        v[ 1 ][ 0 ] = effect->_transformation.e01;
        v[ 1 ][ 1 ] = effect->_transformation.e11;
        v[ 1 ][ 2 ] = effect->_transformation.e21;
        v[ 2 ][ 0 ] = effect->_transformation.e02;
        v[ 2 ][ 1 ] = effect->_transformation.e12;
        v[ 2 ][ 2 ] = effect->_transformation.e22;

        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        ShadersManager::ApplyShader( FlatterEmptyShader, false );

        RendererGlobals::i_ImContext->OMSetBlendState( i_InvertBlend, 0, 0xFFFFFFFF );

        RendererGlobals::i_ImContext->Draw( 4, 0 );
    }

    return sizeof(PostProcess::Effects::InvertColors);
}

uiw PostProcess::Effects::PrettyColors::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::PrettyColors *effect = (PostProcess::Effects::PrettyColors *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
		psTarget.Append( (byte *)&effect->o_color, sizeof(effect->o_color) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );

		vsTarget.Append( (byte *)&effect->_rect, sizeof(ScreenRect) );

        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        ShadersManager::ApplyShader( FlatterColorShader, false );

        RendererGlobals::i_ImContext->OMSetBlendState( i_PrettyBlend, 0, 0xFFFFFFFF );

        RendererGlobals::i_ImContext->Draw( 4, 0 );
    }

    return sizeof(PostProcess::Effects::PrettyColors);
}

uiw PostProcess::Effects::Gray::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    //PostProcess::Effects::Gray *effect = (PostProcess::Effects::Gray *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ShadersManager::ApplyShader( FlatterGrayShader, false );

        RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFFFFFFF );
        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &i_BlackWhiteRTV, 0 );
        RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_BlackWhiteSamp );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &RendererGlobals::i_MainSRV );

        RendererGlobals::i_ImContext->Draw( 3, 0 );

        ShadersManager::ApplyShader( FlatterOnlySampleShader, false );

        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &RendererGlobals::i_MainRenderTargetView, 0 );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_BlackWhiteSRV );

        RendererGlobals::i_ImContext->Draw( 3, 0 );
    }

    return sizeof(PostProcess::Effects::Gray);
}

uiw PostProcess::Effects::GrayFactor::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::GrayFactor *effect = (PostProcess::Effects::GrayFactor *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );

        ui32 colors = effect->_colors - 1;

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
        _MemCpy( o_sr.pData, &colors, sizeof(ui32) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
		vsTarget.Append( (byte *)&effect->_rect, sizeof(ScreenRect) );

        ui32 is_cutTexcoord = 1;
		vsTarget.Append( (byte *)&is_cutTexcoord, 4 );

        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

        ShadersManager::ApplyShader( FlatterGrayFactorShader, false );

        RendererGlobals::i_ImContext->OMSetBlendState( RendererGlobals::i_NoBlend, 0, 0xFFFFFFFF );
        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &i_BlackWhiteRTV, 0 );
        RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_BlackWhiteSamp );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &RendererGlobals::i_MainSRV );

        RendererGlobals::i_ImContext->Draw( 4, 0 );

        ShadersManager::ApplyShader( FlatterOnlySampleShader, false );

        RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &RendererGlobals::i_MainRenderTargetView, 0 );
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_BlackWhiteSRV );

        RendererGlobals::i_ImContext->Draw( 4, 0 );
    
        static ID3D11ShaderResourceView *i_nullView;
        RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &i_nullView );
    }

    return sizeof(PostProcess::Effects::GrayFactor);
}

uiw PostProcess::Effects::Textured::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::Textured *effect = (PostProcess::Effects::Textured *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        ASSUME( effect );
        if( effect->i_blend && effect->i_tex )
        {
            D3D11_MAPPED_SUBRESOURCE o_sr;
            DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
			CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
			psTarget.Append( (byte *)&effect->o_mult, sizeof(effect->o_mult) );
            RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

            DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
			CVecArr < byte > vsTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
			vsTarget.Append( (byte *)&effect->_rect, sizeof(ScreenRect) );

            m2x2 o_texTransform;
            LiceMath::M2x2ScaleRotate( &o_texTransform, effect->texScale.x, effect->texScale.y, effect->texRotate );

            f32 offsetX = effect->texOffset.x + effect->texRotateCenter.x * effect->texScale.x;
            f32 offsetY = effect->texOffset.y + effect->texRotateCenter.y * effect->texScale.y;

            vec4 o_rots[ 2 ] =
            {
                vec4( o_texTransform.e00, o_texTransform.e01, effect->texRotateCenter.x, effect->texRotateCenter.y ),
                vec4( o_texTransform.e10, o_texTransform.e11, offsetX, offsetY )
            };

			vsTarget.Append( (byte *)&o_rots, sizeof(o_rots) );

            RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ 0 ], 0 );

            RendererGlobals::i_ImContext->OMSetBlendState( effect->i_blend, 0, 0xFFffFFff );

            ShadersManager::ApplyShader( FlatterTextureShader, false );

            RendererGlobals::i_ImContext->PSSetSamplers( 0, 1, &i_TexSamp );
            RendererGlobals::i_ImContext->PSSetShaderResources( 0, 1, &effect->i_tex );

            RendererGlobals::i_ImContext->Draw( 4, 0 );
        }
    }

    if( actions & Private::ProcessActions::destruct )
    {
        ASSUME( effect );
        if( effect->i_tex )
        {
            TextureLoader::Free( effect->i_tex );
        }
    }

    return sizeof(PostProcess::Effects::Textured);
}

uiw PostProcess::Effects::Colored::Process( void *mem, Private::ProcessActions::actionsEnum actions )
{
    PostProcess::Effects::Colored *effect = (PostProcess::Effects::Colored *)mem;

    if( actions & Private::ProcessActions::draw )
    {
        if( effect->i_blend )
        {
            D3D11_MAPPED_SUBRESOURCE o_sr;
            DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
			CVecArr < byte > psTarget( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );
			psTarget.Append( (byte *)&effect->o_color, sizeof(effect->o_color) );
            RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ 0 ], 0 );

            RendererGlobals::i_ImContext->OMSetBlendState( effect->i_blend, 0, 0xFFffFFff );

            ShadersManager::ApplyShader( FlatterColorShader, false );

            RendererGlobals::i_ImContext->Draw( 3, 0 );
        }
    }

    return sizeof(PostProcess::Effects::Colored);
}