#include "PreHeader.hpp"
#include "CParticleSystemGPU.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

CParticleSystemGPU::~CParticleSystemGPU()
{
}

CParticleSystemGPU::CParticleSystemGPU( const vec3 &o_pos, const vec3 &o_rot, const vec3 &o_size, CVec < SMaterial, void > &&o_mats ) : CObject( o_pos, o_rot, o_size, std::move( o_mats ) )
{
    if( _o_mats.Size() < 2 )
    {
        SOFTBREAK;
        return;
    }
    _is_inFrustum = true;
    _o_mats[ 0 ].is_inFrustum = true;
    _o_mats[ 1 ].is_inFrustum = false;
    _o_mats[ 1 ].is_enabled = false;
}

void CParticleSystemGPU::Update()  //  virtual
{
    if( _o_mats.Size() < 2 )
    {
        SOFTBREAK;
        return;
    }

    ASSUME( _o_mats[ 1 ].po_geo->vbufsCount == 2 );
    ASSUME( _o_mats[ 1 ].instanceCount == 1 );

    Funcs::Swap( &_o_mats[ 1 ].po_geo->i_vbufs[ 1 ], &_o_mats[ 1 ].po_geo->i_vbufs[ 0 ] );

    if( _o_mats[ 1 ].is_geoShaderDefined )
    {
        RendererGlobals::i_ImContext->IASetInputLayout( _o_mats[ 1 ].i_lo );

        RendererGlobals::i_ImContext->IASetVertexBuffers( 0, 1, &_o_mats[ 1 ].po_geo->i_vbufs[ 0 ], &_o_mats[ 1 ].po_geo->strides[ 0 ], &_o_mats[ 1 ].po_geo->offsets[ 0 ] );
        RendererGlobals::i_ImContext->SOSetTargets( 1, &_o_mats[ 1 ].po_geo->i_vbufs[ 1 ], &_o_mats[ 1 ].po_geo->offsets[ 1 ] );

        ShadersManager::ApplyShader( _o_mats[ 1 ].shader, true );

        RendererGlobals::SetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

        RendererGlobals::i_ImContext->Draw( _o_mats[ 0 ].instanceCount, 0 );

        RendererGlobals::UnbindSOTargets( 1 );
    }
}

void CParticleSystemGPU::Draw( bln is_stepTwo )  //  virtual
{
    if( _o_mats.Size() < 2 )
    {
        SOFTBREAK;
        return;
    }

    _o_mats[ 0 ].po_geo->vbufsCount = 2;
    _o_mats[ 0 ].po_geo->strides[ 1 ] = _o_mats[ 1 ].po_geo->strides[ 0 ];
    _o_mats[ 0 ].po_geo->offsets[ 1 ] = _o_mats[ 1 ].po_geo->offsets[ 0 ];
    _o_mats[ 0 ].po_geo->i_vbufs[ 1 ] = _o_mats[ 1 ].po_geo->i_vbufs[ 0 ];

    CObject::Draw( is_stepTwo );
}