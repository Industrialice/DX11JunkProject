#include "PreHeader.hpp"
#include "CObject.hpp"
#include "Globals.hpp"
#include "Camera.hpp"
#include "Bloom.hpp"
#include "RendererGlobals.hpp"

CObject::~CObject()
{
}

CObject::CObject( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats ) : CObjectBase( o_pos, o_rot, o_size, std::move( o_mats ) )
{
}

void CObject::Update()  //  virtual
{
    LiceMath::M4x3Scale3DRotateXYZTranslate3DVec( &_o_w, &_o_size, &_o_rot, &_o_pos );
    LiceMath::M4x3RotateXYZDoNotIdentity( &_o_wrot, _o_rot.x, _o_rot.y, _o_rot.z );
    _is_inFrustum = false;
    for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
    {
        CHECK( _o_mats[ mat ].is_defined );

        if( !_o_mats[ mat ].is_enabled || !_o_mats[ mat ].po_geo )
        {
            continue;
        }
        if( _o_mats[ mat ].po_geo->ComputeXAABB )
        {
            f32 a_xaabb[ 3 ][ 2 ];
            _o_mats[ mat ].po_geo->ComputeXAABB( _o_mats[ mat ].po_geo, &_o_w, a_xaabb );
            f32 dist;
            bln is_inFrustum = RendererGlobals::CurrentCamera->IsVisible( a_xaabb, &dist );
            _o_mats[ mat ].is_inFrustum = is_inFrustum;
            _is_inFrustum |= is_inFrustum;
        }
        else
        {
            _o_mats[ mat ].is_inFrustum = true;
            _is_inFrustum = true;
        }
    }
}

void CObject::Draw( bln is_stepTwo )  //  virtual
{
    struct
    {
        m4x4 o_wvp;
        m3x4 o_w;
        m3x4 o_wrot;
        m3x4 o_wit;
        vec4 o_pos;
        vec4 o_size;
    } o_mats;

    if( _o_lights.Size() )
    {
        ASSUME( _is_lightable );

        LiceMath::M3x4Inverse4x3( &o_mats.o_wit, &_o_w );

        D3D11_MAPPED_SUBRESOURCE o_srPS, o_srVS;
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ LIGHT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_srVS ) );
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ LIGHT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_srPS ) );
        for( ui32 index = 0; index < _o_lights.Size(); ++index )
        {
            struct
            {
                vec4 o_pos;
                f96color o_col;
                f32 power;
            } o_light = { vec4( _o_lights[ index ]->PositionGet(), 1 ), _o_lights[ index ]->ColorGet(), _o_lights[ index ]->PowerGet() };
    
            _MemCpy( (byte *)o_srVS.pData + index * sizeof(o_light), &o_light, sizeof(o_light) );
            _MemCpy( (byte *)o_srPS.pData + index * sizeof(o_light), &o_light, sizeof(o_light) );
        }
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ LIGHT_DATA_BUF ], 0 );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ LIGHT_DATA_BUF ], 0 );
    }

    LiceMath::M4x3AsM4x4LastIdenMultM4x4( &o_mats.o_wvp, &_o_w, RendererGlobals::CurrentCamera->ViewProjection() );
    LiceMath::M4x4TransposeInplace( &o_mats.o_wvp );

    LiceMath::M4x3Transpose( &o_mats.o_w, &_o_w );

    LiceMath::M4x3Transpose( &o_mats.o_wrot, &_o_wrot );

    o_mats.o_pos = vec4( _o_pos, 1 );
    o_mats.o_size = vec4( _o_size, 0 );

    D3D11_MAPPED_SUBRESOURCE o_sr;
    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ OBJECT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &o_mats, sizeof(o_mats) );
    if( _o_additionalShaderData.targetShaders & SShaderData::vertex )
    {
        ASSUME( _o_additionalShaderData.data && _o_additionalShaderData.dataSize );
        _MemCpy( (byte *)o_sr.pData + sizeof(o_mats), _o_additionalShaderData.data, _o_additionalShaderData.dataSize );
    }
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ OBJECT_DATA_BUF ], 0 );

    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_GSShaderRegisters[ OBJECT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &o_mats, sizeof(o_mats) );
    if( _o_additionalShaderData.targetShaders & SShaderData::geometry )
    {
        ASSUME( _o_additionalShaderData.data && _o_additionalShaderData.dataSize );
        _MemCpy( (byte *)o_sr.pData + sizeof(o_mats), _o_additionalShaderData.data, _o_additionalShaderData.dataSize );
    }
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_GSShaderRegisters[ OBJECT_DATA_BUF ], 0 );

    if( _o_additionalShaderData.targetShaders & SShaderData::pixel )
    {
        ASSUME( _o_additionalShaderData.data && _o_additionalShaderData.dataSize );
        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ OBJECT_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
        _MemCpy( (byte *)o_sr.pData, _o_additionalShaderData.data, _o_additionalShaderData.dataSize );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ OBJECT_DATA_BUF ], 0 );
    }

    for( ui32 mat = 0; mat < _o_mats.Size(); ++mat )
    {
        CHECK( _o_mats[ mat ].is_defined );

        if( !_o_mats[ mat ].is_inFrustum || !_o_mats[ mat ].is_geoShaderDefined || !_o_mats[ mat ].is_enabled || !_o_mats[ mat ].rstates )
        {
            continue;
        }

        if( ((_o_mats[ mat ].rstates & RStates::glowmap) != 0) != is_stepTwo )
        {
            continue;
        }

        RendererGlobals::i_ImContext->IASetInputLayout( _o_mats[ mat ].i_lo );

        RendererGlobals::CurrentBloom->RenderingStatesSet( _o_mats[ mat ].rstates );

        if( _o_mats[ mat ].po_geo->vbufsCount )
        {
            RendererGlobals::i_ImContext->IASetVertexBuffers( 0, _o_mats[ mat ].po_geo->vbufsCount, _o_mats[ mat ].po_geo->i_vbufs, _o_mats[ mat ].po_geo->strides, _o_mats[ mat ].po_geo->offsets );
        }
        if( _o_mats[ mat ].po_geo->i_ibuf )
        {
            RendererGlobals::i_ImContext->IASetIndexBuffer( _o_mats[ mat ].po_geo->i_ibuf, _o_mats[ mat ].po_geo->is_32bitIndices ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0 );
        }

        RendererGlobals::SetPrimitiveTopology( _o_mats[ mat ].po_geo->topo );

        RendererGlobals::i_ImContext->OMSetBlendState( _o_mats[ mat ].i_blend, 0, 0xFFffFFff );
        RendererGlobals::i_ImContext->RSSetState( _o_mats[ mat ].i_rasterizerState );

        ID3D11ShaderResourceView *ai_texs[ 16 ];
        ID3D11SamplerState *ai_samps[ 16 ];

        if( _o_mats[ mat ].o_textures.Size() )
        {
            DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ MATERIAL_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
			CVecArr < byte > target( (byte *)o_sr.pData, 0, sizeof(vec4) * 4096 );
            for( ui32 tex = 0; tex < _o_mats[ mat ].o_textures.Size(); ++tex )
            {
                ai_texs[ tex ] = _o_mats[ mat ].o_textures[ tex ].i_tex;
                ai_samps[ tex ] = _o_mats[ mat ].o_textures[ tex ].i_sampler;

                m2x2 o_texTransform;
                LiceMath::M2x2ScaleRotate( &o_texTransform, _o_mats[ mat ].o_textures[ tex ].o_texMult.x, _o_mats[ mat ].o_textures[ tex ].o_texMult.y, _o_mats[ mat ].o_textures[ tex ].rotAngleRads );

                f32 offsetX = _o_mats[ mat ].o_textures[ tex ].o_texOffset.x + _o_mats[ mat ].o_textures[ tex ].o_rotCenter.x * _o_mats[ mat ].o_textures[ tex ].o_texMult.x;
                f32 offsetY = _o_mats[ mat ].o_textures[ tex ].o_texOffset.y + _o_mats[ mat ].o_textures[ tex ].o_rotCenter.y * _o_mats[ mat ].o_textures[ tex ].o_texMult.y;

                vec4 o_rots[ 2 ] =
                {
                    vec4( o_texTransform.e00, o_texTransform.e01, _o_mats[ mat ].o_textures[ tex ].o_rotCenter.x, _o_mats[ mat ].o_textures[ tex ].o_rotCenter.y ),
                    vec4( o_texTransform.e10, o_texTransform.e11, offsetX, offsetY )
                };

				target.Append( (byte *)&o_rots, sizeof(o_rots) );
            }
            RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ MATERIAL_DATA_BUF ], 0 );
        
            RendererGlobals::i_ImContext->PSSetShaderResources( 0, _o_mats[ mat ].o_textures.Size(), ai_texs );
            RendererGlobals::i_ImContext->PSSetSamplers( 0, _o_mats[ mat ].o_textures.Size(), ai_samps );
        }

        DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ MATERIAL_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
		CVecArr < byte > target( (byte *)o_sr.pData, 0, sizeof(vec4) * 4096 );
        target.Append( (byte *)&_o_mats[ mat ].o_difColor, sizeof(f128color) );
		target.Append( (byte *)&_o_mats[ mat ].o_specColor, sizeof(f128color) );
		target.Append( (byte *)&_o_mats[ mat ].o_ambColor, sizeof(f96color) );
		target.Append( (byte *)&_o_mats[ mat ].externalLightPower, sizeof(f32) );
        RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ MATERIAL_DATA_BUF ], 0 );
    
        ShadersManager::ApplyShader( _o_mats[ mat ].shader, false );

        ui32 instances = _o_mats[ mat ].instanceCount * _o_mats[ mat ].po_geo->instancesCount;

        if( _o_mats[ mat ].po_geo->i_ibuf )
        {
            if( instances > 1 )
            {
                RendererGlobals::i_ImContext->DrawIndexedInstanced( _o_mats[ mat ].o_geoSlice.indicesCount, instances, _o_mats[ mat ].o_geoSlice.startIndex, _o_mats[ mat ].o_geoSlice.startVertex, _o_mats[ mat ].startInstance );
            }
            else
            {
                RendererGlobals::i_ImContext->DrawIndexed( _o_mats[ mat ].o_geoSlice.indicesCount, _o_mats[ mat ].o_geoSlice.startIndex, _o_mats[ mat ].o_geoSlice.startVertex );
            }
        }
        else
        {
            if( instances > 1 )
            {
                RendererGlobals::i_ImContext->DrawInstanced( _o_mats[ mat ].o_geoSlice.verticesCount, instances, _o_mats[ mat ].o_geoSlice.startVertex, _o_mats[ mat ].startInstance );
            }
            else
            {
                RendererGlobals::i_ImContext->Draw( _o_mats[ mat ].o_geoSlice.verticesCount, _o_mats[ mat ].o_geoSlice.startVertex );
            }
        }

        RendererGlobals::UnbindVBuffers( 0, _o_mats[ mat ].po_geo->vbufsCount );
    }
}