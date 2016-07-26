#include "PreHeader.hpp"
#include "CHalos.hpp"
#include "Globals.hpp"
#include "Camera.hpp"
#include "Bloom.hpp"
#include "ShadersManager.hpp"
#include "RendererGlobals.hpp"

namespace
{
    LayoutsManager::BufferDesc_t InstanceBufferDesc = LayoutsManager::BufferDesc_t();
}

CHalos::~CHalos()
{
    //  nothing to destruct?
}

CHalos::CHalos( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats, bln is_occlude ) : CObject( o_pos, o_rot, o_size, std::move( o_mats ) )
{
    _is_occlude = is_occlude;
    _i_vbDraw = 0;
    _visibleHalos = 0;
    _visibleReserved = 0;
    _is_inFrustum = true;
    _is_glowing = true;

    D3D11_BLEND_DESC o_invertBlend = {};
    o_invertBlend.AlphaToCoverageEnable = false;
    o_invertBlend.IndependentBlendEnable = false;
    o_invertBlend.RenderTarget[ 0 ].BlendEnable = true;
    o_invertBlend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_ONE;
    o_invertBlend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_ONE;
    o_invertBlend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_invertBlend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_invertBlend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_invertBlend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_invertBlend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    _i_blend = BlendStatesManager::GetState( &o_invertBlend );

    static const VertexBufferFieldDesc sca_particleHaloDesc[] =
    {
        { "LOCATION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 1 },
        { "RADIUS0", DXGI_FORMAT_R32_FLOAT, 12, 1 },
        { "COLOR0", DXGI_FORMAT_R32G32B32A32_FLOAT, 16, 1 }
    };

    if( InstanceBufferDesc.IsNull() )
    {
        InstanceBufferDesc = RendererGlobals::DefLayoutsManager.CompileBufferDesc( sca_particleHaloDesc, 1 );
    }
    
    for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
    {
        LayoutsManager::BufferDesc_t unitedBufferDesc = RendererGlobals::DefLayoutsManager.UniteCompiledBufferDescs( CVec < LayoutsManager::BufferDesc_t >( { _o_mats[ mat ].po_geo->description, InstanceBufferDesc } ) );
        _o_mats[ mat ].is_geoShaderDefined = ShadersManager::TryToBlend( unitedBufferDesc, _o_mats[ mat ].shader, &_o_mats[ mat ].i_lo );
    }
}

PROPERTYG( CHalos::IsOcclude, _is_occlude, bln );

void CHalos::IsOccludeSet( bln is_occlude )
{
    if( _is_occlude == is_occlude )
    {
        return;
    }

    _is_occlude = is_occlude;

    if( !_is_occlude )
    {
        _o_oqueries.Clear();
    }
}

void CHalos::Clear()
{
    _o_halos.Clear();
    _visibleHalos = 0;
    _visibleReserved = 0;
    _i_vbDraw.Release();
    _o_oqueries.Clear();
}

void CHalos::Append( const vec3 &o_pos, f32 intensity, const f128color &o_color, CObjectBase *testObject )
{
    SHalo o_halo;
    o_halo.o_pos = o_pos;
    o_halo.intensity = intensity;
    o_halo.o_color = o_color;
    o_halo.testObject = testObject;
    _o_halos.Append( o_halo );
}

void CHalos::RemoveAssociated( CObjectBase *testObject )
{
}

void CHalos::NonOccludeUpdate()
{
    _visibleHalos = 0;

    D3D11_MAPPED_SUBRESOURCE o_sr;
    DXHRCHECK( RendererGlobals::i_ImContext->Map( _i_vbDraw, 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    CVecArr < byte > target( (byte *)o_sr.pData, 0, sizeof(vec4) * 4096 );

    for( ui32 index = 0; index < _o_halos.Size(); ++index )
    {
        const m4x4 *cpo_m = RendererGlobals::CurrentCamera->ViewProjection();
        f32 z = _o_halos[ index ].o_pos.x * cpo_m->e02 + _o_halos[ index ].o_pos.y * cpo_m->e12 + _o_halos[ index ].o_pos.z * cpo_m->e22 + cpo_m->e32;
        f32 w = _o_halos[ index ].o_pos.x * cpo_m->e03 + _o_halos[ index ].o_pos.y * cpo_m->e13 + _o_halos[ index ].o_pos.z * cpo_m->e23 + cpo_m->e33;
        z /= w;
        if( z < 0.f || z > 1.f )
        {
            continue;
        }

        //f32 x = _o_halos[ index ].o_pos.x * cpo_m->e00 + _o_halos[ index ].o_pos.y * cpo_m->e10 + _o_halos[ index ].o_pos.z * cpo_m->e20 + cpo_m->e30;
        //if( x < -w || x > w )
        //{
        //    continue;
        //}
        //f32 y = _o_halos[ index ].o_pos.x * cpo_m->e01 + _o_halos[ index ].o_pos.y * cpo_m->e11 + _o_halos[ index ].o_pos.z * cpo_m->e21 + cpo_m->e31;
        //if( y < -w || y > w )
        //{
        //    continue;
        //}

        f32 widthExpected = Globals::Width / 2 * _o_halos[ index ].intensity;
        f32 heightExpected = Globals::Height / 2 * _o_halos[ index ].intensity;

        f32 pixelsExpected = (widthExpected * heightExpected) / w / 3;

        if( pixelsExpected < 1 )
        {
            continue;
        }
    
        target.Append( (byte *)&_o_halos[ index ].o_pos, sizeof(vec3) );
		target.Append( (byte *)&_o_halos[ index ].intensity, sizeof(f32) );
		target.Append( (byte *)&_o_halos[ index ].o_color, sizeof(f128color) );
    
        ++_visibleHalos;
    }

    RendererGlobals::i_ImContext->Unmap( _i_vbDraw, 0 );
}

void CHalos::Update()
{
    if( !_o_halos.Size() )
    {
        return;
    }

    CObject::Update();

    if( _visibleReserved < _o_halos.Size() )
    {
        _i_vbDraw.Release();

        D3D11_BUFFER_DESC o_vbufDesc =
        {
            sizeof(SHalo) * _o_halos.Size(),
            D3D11_USAGE_DYNAMIC,
            D3D11_BIND_VERTEX_BUFFER,
            D3D11_CPU_ACCESS_WRITE,
            0,
            0
        };
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &o_vbufDesc, 0, _i_vbDraw.AddrModifiable() ) );

        _visibleReserved = _o_halos.Size();
    }

    if( !_is_occlude )
    {
        NonOccludeUpdate();
    }
}

void CHalos::Draw( bln is_stepTwo )
{
    if( _o_halos.Size() == 0 || _o_mats.Size() < 1 || (!_is_occlude && _visibleHalos == 0) || !is_stepTwo )
    {
        return;
    }

    ASSUME( _i_vbDraw );

    if( _is_occlude )
    {
        ui32 queriesCount = 0;

        for( ui32 halo = 0; halo < _o_halos.Size(); ++halo )
        {
            if( !_o_halos[ halo ].testObject )
            {
                continue;
            }

            CObjectBase *obj = _o_halos[ halo ].testObject;
            if( !obj->IsInFrustumGet() || !obj->IsGlowingGet() )
            {
                continue;
            }

            if( _o_oqueries.Size() <= queriesCount )
            {
                ASSUME( _o_oqueries.Size() + 1 > queriesCount );
                _o_oqueries.Resize( _o_oqueries.Size() + 1 );

                D3D11_QUERY_DESC o_queryDesc;
                o_queryDesc.Query = D3D11_QUERY_OCCLUSION;
                o_queryDesc.MiscFlags = 0;
                DXHRCHECK( RendererGlobals::i_Device->CreateQuery( &o_queryDesc, _o_oqueries[ queriesCount ].i_query.AddrModifiable() ) );
            }

            _o_oqueries[ queriesCount ].index = halo;
        
            RendererGlobals::i_ImContext->Begin( _o_oqueries[ queriesCount ].i_query );
            obj->Draw( true );
            RendererGlobals::i_ImContext->End( _o_oqueries[ queriesCount ].i_query );

            ++queriesCount;
        }

        if( !queriesCount )
        {
            return;
        }

        f32 screenSizePixels = Globals::Width * Globals::Height;

        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( _i_vbDraw, 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > target( (byte *)o_sr.pData, 0, sizeof( vec4 ) * 4096 );

        _visibleHalos = 0;

        for( ui32 query = 0; query < queriesCount; ++query )
        {
            UINT64 pixels;
            while( RendererGlobals::i_ImContext->GetData( _o_oqueries[ query ].i_query, &pixels, sizeof(pixels), 0 ) != S_OK )
            {
            }
            if( !pixels )
            {
                continue;
            }

            ui32 index = _o_oqueries[ query ].index;

            f32 onScreenIntensity = (f32)pixels / screenSizePixels;
            f32 haloIntensity = _o_halos[ index ].intensity * onScreenIntensity;
    
			target.Append( (byte *)&_o_halos[ index ].o_pos, sizeof(vec3) );
			target.Append( (byte *)&haloIntensity, sizeof(f32) );
			target.Append( (byte *)&_o_halos[ index ].o_color, sizeof(f128color) );

            ++_visibleHalos;
        }

        RendererGlobals::i_ImContext->Unmap( _i_vbDraw, 0 );
    }

    if( !_visibleHalos )
    {
        return;
    }

    for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
    {
        if( !_o_mats[ mat ].is_geoShaderDefined || !_o_mats[ mat ].is_enabled )
        {
            continue;
        }

        if( !_o_mats[ mat ].rstates )
        {
            continue;
        }

        RendererGlobals::i_ImContext->IASetInputLayout( _o_mats[ mat ].i_lo );

        RendererGlobals::MainBloom.RenderingStatesSet( _o_mats[ mat ].rstates );
    
        D3D11_MAPPED_SUBRESOURCE o_sr;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ OBJECT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
        m4x4 o_w = m4x4( _o_w );
        LiceMath::M4x4TransposeInplace( &o_w );
        _MemCpy( o_sr.pData, &o_w, sizeof(m4x4) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ OBJECT_DATA_BUF ], 0 );
    
        ID3D11Buffer *bufs[ 2 ] = { _o_mats[ mat ].po_geo->i_vbufs[ 0 ], _i_vbDraw };
        ui32 offsets[ 2 ] = {};
        ui32 strides[ 2 ] = { _o_mats[ mat ].po_geo->strides[ 0 ], sizeof(vec3) + sizeof(f32) + sizeof(f128color) };
        RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 2, bufs, strides, offsets );

        RendererGlobals::SetPrimitiveTopology( _o_mats[ mat ].po_geo->topo );

        RendererGlobals::i_ImContext->OMSetBlendState( _i_blend, 0, 0xFFFFFFFF );

        ShadersManager::ApplyShader( _o_mats[ mat ].shader, false );
    
        RendererGlobals::i_ImContext->DrawInstanced( 4, _visibleHalos, 0, 0 );

        RendererGlobals::UnbindVBuffers( 0, 2 );
    }
}