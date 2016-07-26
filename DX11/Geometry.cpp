#include "PreHeader.hpp"
#include "Geometry.hpp"
#include "LayoutsManager.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"

static void ComputeXAABBForBox( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] );
static void ComputeXAABBForFlat( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] );
static void ComputeXAABBDummy( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] );

#pragma optimize( "s", on )

void Geometry::Box( SGeometry *po_geo, bln is_center )
{
	*po_geo = SGeometry();

    static const ui16 sca_indices[ 36 ] = 
    {
        //  front
        0, 1, 2,
        3, 0, 2,

        //  right
        4, 5, 6,
        7, 4, 6,

        //  back
        8, 9, 10,
        11, 8, 10,

        //  left
        12, 13, 14,
        15, 12, 14,

        //  up
        16, 17, 18,
        19, 16, 18,

        //  down
        20, 21, 22,
        23, 20, 22
    };

	f32 minPos, maxPos;
	if( is_center )
	{
		minPos = -0.5f;
		maxPos = 0.5f;
	}
	else
	{
		minPos = 0.0f;
		maxPos = 1.0f;
	}

    static const struct
    {
        vec3 o_pos;
    } scao_vertices[ 24 ] =
    {
        //  front
        { vec3( minPos, minPos, minPos ) },
        { vec3( minPos, maxPos, minPos ) },
        { vec3( maxPos, maxPos, minPos ) },
        { vec3( maxPos, minPos, minPos ) },

        //  right
        { vec3( maxPos, minPos, minPos ) },
        { vec3( maxPos, maxPos, minPos ) },
        { vec3( maxPos, maxPos, maxPos ) },
        { vec3( maxPos, minPos, maxPos ) },

        //  back
        { vec3( maxPos, minPos, maxPos ) },
        { vec3( maxPos, maxPos, maxPos ) },
        { vec3( minPos, maxPos, maxPos ) },
        { vec3( minPos, minPos, maxPos ) },

        //  left
        { vec3( minPos, minPos, maxPos ) },
        { vec3( minPos, maxPos, maxPos ) },
        { vec3( minPos, maxPos, minPos ) },
        { vec3( minPos, minPos, minPos ) },

        //  up
        { vec3( minPos, maxPos, minPos ) },
        { vec3( minPos, maxPos, maxPos ) },
        { vec3( maxPos, maxPos, maxPos ) },
        { vec3( maxPos, maxPos, minPos ) },

        //  down
        { vec3( minPos, minPos, maxPos ) },
        { vec3( minPos, minPos, minPos ) },
        { vec3( maxPos, minPos, minPos ) },
        { vec3( maxPos, minPos, maxPos ) },
    };

    static const D3D11_BUFFER_DESC sco_bufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_bufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_ibufData;
    o_ibufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_ibufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc vertexFieldsDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( vertexFieldsDesc ) );
    po_geo->verticesCount = 24;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForBox;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
    po_geo->indicesCount = 36;
}

void Geometry::BoxTN( SGeometry *po_geo, bln is_center )
{
    *po_geo = SGeometry();

	f32 minPos, maxPos;
	if( is_center )
	{
		minPos = -0.5f;
		maxPos = 0.5f;
	}
	else
	{
		minPos = 0.0f;
		maxPos = 1.0f;
	}

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
        vec3 o_nor;
        vec3 o_tan;
    } scao_vertices[ 36 ] =
    {
        //  front
        { vec3( minPos, minPos, minPos ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, maxPos, minPos ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, minPos ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, minPos, minPos ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, minPos, minPos ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, minPos ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },

        //  right
        { vec3( maxPos, minPos, minPos ), vec2( 0.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( maxPos, maxPos, minPos ), vec2( 0.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( maxPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( maxPos, minPos, maxPos ), vec2( 1.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( maxPos, minPos, minPos ), vec2( 0.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( maxPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },

        //  back
        { vec3( maxPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, maxPos ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( minPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( minPos, minPos, maxPos ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( maxPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( minPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },

        //  left
        { vec3( minPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( minPos, maxPos, maxPos ), vec2( 0.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( minPos, maxPos, minPos ), vec2( 1.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( minPos, minPos, minPos ), vec2( 1.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( minPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( minPos, maxPos, minPos ), vec2( 1.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },

        //  up
        { vec3( minPos, maxPos, minPos ), vec2( 0.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, maxPos, maxPos ), vec2( 0.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, minPos ), vec2( 1.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, maxPos, minPos ), vec2( 0.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, maxPos, maxPos ), vec2( 1.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },

        //  down
        { vec3( minPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, minPos, minPos ), vec2( 0.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, minPos, minPos ), vec2( 1.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, minPos, maxPos ), vec2( 1.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( minPos, minPos, maxPos ), vec2( 0.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( maxPos, minPos, minPos ), vec2( 1.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) }
    };

    static const D3D11_BUFFER_DESC sco_bufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_bufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
        { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
        { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
    po_geo->verticesCount = 36;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForBox;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::BoxTNIndexed( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    static const ui16 sca_indices[ 36 ] = 
    {
        //  front
        0, 1, 2,
        3, 0, 2,

        //  right
        4, 5, 6,
        7, 4, 6,

        //  back
        8, 9, 10,
        11, 8, 10,

        //  left
        12, 13, 14,
        15, 12, 14,

        //  up
        16, 17, 18,
        19, 16, 18,

        //  down
        20, 21, 22,
        23, 20, 22
    };

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
        vec3 o_nor;
        vec3 o_tan;
    } scao_vertices[ 24 ] =
    {
        //  front
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },

        //  right
        { vec3( 1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },

        //  back
        { vec3( 1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },

        //  left
        { vec3( -1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },

        //  up
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },

        //  down
        { vec3( -1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 0.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 0.f, -1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
    };

    static const D3D11_BUFFER_DESC sco_bufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_bufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_ibufData;
    o_ibufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_ibufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
        { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
        { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
    po_geo->verticesCount = 24;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForBox;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
    po_geo->indicesCount = 36;
}

void Geometry::BoxTNIndexedBottomless( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    static const ui16 sca_indices[ 30 ] = 
    {
        //  front
        0, 1, 2,
        3, 0, 2,

        //  right
        4, 5, 6,
        7, 4, 6,

        //  back
        8, 9, 10,
        11, 8, 10,

        //  left
        12, 13, 14,
        15, 12, 14,

        //  up
        16, 17, 18,
        19, 16, 18
    };

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
        vec3 o_nor;
        vec3 o_tan;
    } scao_vertices[ 20 ] =
    {
        //  front
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },

        //  right
        { vec3( 1.f, -1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 0.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },
        { vec3( 1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 1.f, 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ) },

        //  back
        { vec3( 1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },
        { vec3( -1.f, -1.f, 1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ) },

        //  left
        { vec3( -1.f, -1.f, 1.f ), vec2( 0.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 1.f, 0.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },
        { vec3( -1.f, -1.f, -1.f ), vec2( 1.f, 1.f ), vec3( -1.f, 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ) },

        //  up
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 0.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 1.f ), vec3( 0.f, 1.f, 0.f ), vec3( 1.f, 0.f, 0.f ) }
    };

    static const D3D11_BUFFER_DESC sco_bufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_bufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_ibufData;
    o_ibufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_ibufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
        { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
        { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
    po_geo->verticesCount = 20;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForBox;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
    po_geo->indicesCount = 30;
}

void Geometry::BoxIndexedBottomless( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    static const ui16 sca_indices[ 30 ] = 
    {
        //  front
        0, 1, 2,
        3, 0, 2,

        //  right
        3, 2, 5,
        4, 3, 5,

        //  back
        4, 5, 6,
        7, 4, 6,

        //  left
        7, 6, 1,
        0, 7, 1,

        //  up
        1, 8, 9,
        2, 1, 9
    };

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
    } scao_vertices[ 10 ] =
    {
        { vec3( -1.f, -1.f, -1.f ), vec2( 0.f, 1.f ) },
        { vec3( -1.f, 1.f, -1.f ), vec2( 0.f, 0.f ) },
        { vec3( 1.f, 1.f, -1.f ), vec2( 1.f, 0.f ) },
        { vec3( 1.f, -1.f, -1.f ), vec2( 1.f, 1.f ) },

        { vec3( 1.f, -1.f, 1.f ), vec2( 0.f, 1.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 0.f, 0.f ) },
        { vec3( -1.f, 1.f, 1.f ), vec2( 1.f, 0.f ) },
        { vec3( -1.f, -1.f, 1.f ), vec2( 1.f, 1.f ) },
    
        { vec3( -1.f, 1.f, 1.f ), vec2( 0.f, 1.f ) },
        { vec3( 1.f, 1.f, 1.f ), vec2( 1.f, 1.f ) }
    };

    static const D3D11_BUFFER_DESC sco_bufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_bufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_ibufData;
    o_ibufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_ibufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
    po_geo->verticesCount = COUNTOF( scao_vertices );
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForBox;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
    po_geo->indicesCount = COUNTOF( sca_indices );
}

void Geometry::FlatTN( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
        vec3 o_tan;
        vec3 o_nor;
    } scao_vertices[ 4 ] =
    {
        { vec3( 0, 0, 0.f ), vec2( 0.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 0, 1.f, 0.f ), vec2( 0.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 0.f ), vec2( 1.f, 0.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) },
        { vec3( 1.f, 0, 0.f ), vec2( 1.f, 1.f ), vec3( 0.f, 0.f, -1.f ), vec3( 1.f, 0.f, 0.f ) }
    };

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const ui16 sca_indices[ 6 ] =
    {
        0, 1, 2,
        3, 0, 2
    };

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    o_bufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_bufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos3DTexNorTanDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 },
        { "NORMAL0", DXGI_FORMAT_R32G32B32_FLOAT, 20, 0 },
        { "TANGENT0", DXGI_FORMAT_R32G32B32_FLOAT, 32, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos3DTexNorTanDesc ) );
    po_geo->verticesCount = 4;
    po_geo->indicesCount = 6;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::Flat( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    const f32 scale = 0.5f;

    static const struct
    {
        vec2 o_pos;
        vec2 o_tex;
    } scao_vertices[ 4 ] =
    {
        { vec2( -scale, -scale ), vec2( 0.f, 1.f ) },
        { vec2( -scale, scale ), vec2( 0.f, 0.f ) },
        { vec2( scale, scale ), vec2( 1.f, 0.f ) },
        { vec2( scale, -scale ), vec2( 1.f, 1.f ) }
    };

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const ui16 sca_indices[ 6 ] =
    {
        0, 1, 2,
        3, 0, 2
    };

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    o_bufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_bufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos2DTexDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 8, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos2DTexDesc ) );
    po_geo->verticesCount = 4;
    po_geo->indicesCount = 6;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::FlatStrip( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    const f32 scale = 0.5f;

    static const struct
    {
        vec2 o_pos;
        vec2 o_tex;
    } scao_vertices[ 4 ] =
    {
        { vec2( -scale, -scale ), vec2( 0.f, 1.f ) },
        { vec2( -scale, scale ), vec2( 0.f, 0.f ) },
        { vec2( scale, -scale ), vec2( 1.f, 1.f ) },
        { vec2( scale, scale ), vec2( 1.f, 0.f ) }
    };

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const VertexBufferFieldDesc a_Pos2DTexDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 8, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos2DTexDesc ) );
    po_geo->verticesCount = 4;
    po_geo->indicesCount = 0;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::Flat2( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    static const struct
    {
        vec3 o_pos;
        vec2 o_tex;
    } scao_vertices[ 4 ] =
    {
        { vec3( 0, 0, 0 ), vec2( 0.f, 1.f ) },
        { vec3( 0, 1.f, 0 ), vec2( 0.f, 0.f ) },
        { vec3( 1.f, 1.f, 0 ), vec2( 1.f, 0.f ) },
        { vec3( 1.f, 0, 0 ), vec2( 1.f, 1.f ) }
    };

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const ui16 sca_indices[ 6 ] =
    {
        0, 1, 2,
        3, 0, 2
    };

    static const D3D11_BUFFER_DESC sco_ibufDesc =
    {
        sizeof(sca_indices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_INDEX_BUFFER,
        0,
        0,
        0
    };
    o_bufData.pSysMem = &sca_indices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_bufData, &po_geo->i_ibuf ) );

    static const VertexBufferFieldDesc a_Pos2DTexDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32B32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 12, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos2DTexDesc ) );
    po_geo->verticesCount = 4;
    po_geo->indicesCount = 6;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::FlatHalo( SGeometry *po_geo )
{
    *po_geo = SGeometry();

    const f32 scale = 0.5f;

    static const struct
    {
        vec2 o_pos;
        vec2 o_tex;
    } scao_vertices[ 4 ] =
    {
        { vec2( -scale, -scale ), vec2( 0.f, 1.f ) },
        { vec2( -scale, scale ), vec2( 0.f, 0.f ) },
        { vec2( scale, -scale ), vec2( 1.f, 1.f ) },
        { vec2( scale, scale ), vec2( 1.f, 0.f ) }
    };

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const VertexBufferFieldDesc a_Pos2DTexDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 8, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos2DTexDesc ) );
    po_geo->verticesCount = 4;
    po_geo->indicesCount = 0;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::TriCircle( SGeometry *po_geo, f32 radius )
{
    *po_geo = SGeometry();

    const struct
    {
        vec2 o_pos;
        vec2 o_tex;
    } scao_vertices[ 3 ] =
    {
        { vec2( -radius, -radius * 3 ), vec2( 0.f, 2.f ) },
        { vec2( -radius, radius ), vec2( 0.f, 0.f ) },
        { vec2( radius * 2, radius ), vec2( 1.5f, 0.f ) },
    };

    //  0: [-0.5; -1.5] [0.0; 2.0]
    //  1: [-0.5;  0.5] [0.0; 0.0]
    //  2: [ 1.0;  0.5] [1.5; 0.0]

    static const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        sizeof(scao_vertices),
        D3D11_USAGE_IMMUTABLE,
        D3D11_BIND_VERTEX_BUFFER,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = &scao_vertices;
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );

    static const VertexBufferFieldDesc a_Pos2DTexDesc[] =
    {
        { "POSITION0", DXGI_FORMAT_R32G32_FLOAT, 0, 0 },
        { "TEXCOORD0", DXGI_FORMAT_R32G32_FLOAT, 8, 0 }
    };

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( a_Pos2DTexDesc ) );
    po_geo->verticesCount = 3;
    po_geo->indicesCount = 0;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = ComputeXAABBForFlat;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = 1;
    po_geo->strides[ 0 ] = sizeof(*scao_vertices);
    po_geo->offsets[ 0 ] = 0;
}

static CStr MakeSemantic( const char *name, uiw index )
{
    char buf[ 128 ];
    char *secondPart = buf + Funcs::StrCpyAndCountAdv( buf, name, false, sizeof(buf) );
    Funcs::IntToStrDec( index, secondPart );
    return buf;
}

void Geometry::Make( SGeometry *po_geo, VertexFmt::vertexFmt_t *fmts, ui32 attribsCount, f32 *vbuf, ui32 vbufSize, ui32 verticesCount, ui16 *ibuf, ui32 indicesCount, D3D_PRIMITIVE_TOPOLOGY topo )
{
    ASSUME( po_geo && fmts && ((attribsCount == 0) == (vbuf == 0)) && ((indicesCount == 0) == (ibuf == 0)) );

    *po_geo = SGeometry();

    if( attribsCount )
    {
        ASSUME( attribsCount <= D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT );
        VertexBufferFieldDesc desc[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
        ui32 semanticIndices[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ] = {};
        ui32 vertexSize = 0;
        for( ui32 attr = 0; attr < attribsCount; ++attr )
        {
            desc[ attr ].instanceStep = 0;
            desc[ attr ].offset = vertexSize;
            if( fmts[ attr ] == VertexFmt::Position )
            {
                desc[ attr ].semantic = MakeSemantic( "POSITION", semanticIndices[ 0 ] );
                desc[ attr ].format = DXGI_FORMAT_R32G32B32_FLOAT;
                semanticIndices[ 0 ]++;
                vertexSize += sizeof(f32) * 3;
            }
            else if( fmts[ attr ] == VertexFmt::Normal )
            {
                desc[ attr ].semantic = MakeSemantic( "NORMAL", semanticIndices[ 1 ] );
                desc[ attr ].format = DXGI_FORMAT_R32G32B32_FLOAT;
                semanticIndices[ 1 ]++;
                vertexSize += sizeof(f32) * 3;
            }
            else if( fmts[ attr ] == VertexFmt::Texcoord )
            {
                desc[ attr ].semantic = MakeSemantic( "TEXCOORD", semanticIndices[ 2 ] );
                desc[ attr ].format = DXGI_FORMAT_R32G32_FLOAT;
                semanticIndices[ 2 ]++;
                vertexSize += sizeof(f32) * 2;
            }
            else if( fmts[ attr ] == VertexFmt::Binormal )
            {
                desc[ attr ].semantic = MakeSemantic( "BINORMAL", semanticIndices[ 3 ] );
                desc[ attr ].format = DXGI_FORMAT_R32G32B32_FLOAT;
                semanticIndices[ 3 ]++;
                vertexSize += sizeof(f32) * 3;
            }
            else  //  if( fmts[ attr ] == VertexFmt::Tangent )
            {
                desc[ attr ].semantic = MakeSemantic( "TANGENT", semanticIndices[ 4 ] );
                desc[ attr ].format = DXGI_FORMAT_R32G32B32_FLOAT;
                semanticIndices[ 4 ]++;
                vertexSize += sizeof(f32) * 3;
            }
        }

        ASSUME( verticesCount * vertexSize == vbufSize );
    
        po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( MakeRefVec( desc, attribsCount ) );
        po_geo->strides[ 0 ] = vertexSize;

        const D3D11_BUFFER_DESC sco_vbufDesc =
        {
            vbufSize,
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_VERTEX_BUFFER,
            0,
            0,
            0
        };
        D3D11_SUBRESOURCE_DATA o_bufData;
        o_bufData.pSysMem = vbuf;
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );
    }
    else
    {
        po_geo->description = LayoutsManager::BufferDesc_t();
    }

    if( ibuf )
    {
        const D3D11_BUFFER_DESC sco_ibufDesc =
        {
            sizeof(ui16) * indicesCount,
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_INDEX_BUFFER,
            0,
            0,
            0
        };
        D3D11_SUBRESOURCE_DATA o_bufData;
        o_bufData.pSysMem = ibuf;
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_ibufDesc, &o_bufData, &po_geo->i_ibuf ) );
    }

    po_geo->verticesCount = verticesCount;
    po_geo->indicesCount = indicesCount;
    po_geo->topo = topo;
    po_geo->is_32bitIndices = false;
    po_geo->ComputeXAABB = 0;
    po_geo->instancesCount = 1;
    po_geo->vbufsCount = vbuf != 0;
    po_geo->offsets[ 0 ] = 0;
}

void Geometry::StreamData( SGeometry *po_geo, CCRefVec < VertexBufferFieldDesc > desc, void *vertices, uiw verticesCount, uiw vertexSize )
{
    *po_geo = SGeometry();

    const D3D11_BUFFER_DESC sco_vbufDesc =
    {
        verticesCount * vertexSize,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT,
        0,
        0,
        0
    };
    D3D11_SUBRESOURCE_DATA o_bufData;
    o_bufData.pSysMem = vertices;

    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 0 ] ) );
    DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &sco_vbufDesc, &o_bufData, &po_geo->i_vbufs[ 1 ] ) );

    po_geo->description = RendererGlobals::DefLayoutsManager.CompileBufferDesc( desc );
    po_geo->strides[ 0 ] = vertexSize;
    po_geo->strides[ 1 ] = vertexSize;
    po_geo->verticesCount = verticesCount;
    po_geo->topo = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    po_geo->indicesCount = 0;
    po_geo->is_32bitIndices = false;
    po_geo->i_ibuf = 0;
    po_geo->ComputeXAABB = 0;
    po_geo->vbufsCount = 2;
    po_geo->instancesCount = 1;
    po_geo->offsets[ 0 ] = 0;
    po_geo->offsets[ 1 ] = 0;
}

#pragma optimize( "", on )

void ComputeXAABBForBox( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] )
{
    vec3 o_c( cpo_w->e30, cpo_w->e31, cpo_w->e32 );

    vec3 o_e;
    o_e.x = ::fabsf( cpo_w->e00 ) + ::fabsf( cpo_w->e10 ) + ::fabsf( cpo_w->e20 );
    o_e.y = ::fabsf( cpo_w->e01 ) + ::fabsf( cpo_w->e11 ) + ::fabsf( cpo_w->e21 );
    o_e.z = ::fabsf( cpo_w->e02 ) + ::fabsf( cpo_w->e12 ) + ::fabsf( cpo_w->e22 );

    a_xaabb[ 0 ][ 0 ] = o_c.x + o_e.x;
    a_xaabb[ 1 ][ 0 ] = o_c.y + o_e.y;
    a_xaabb[ 2 ][ 0 ] = o_c.z + o_e.z;

    a_xaabb[ 0 ][ 1 ] = o_c.x - o_e.x;
    a_xaabb[ 1 ][ 1 ] = o_c.y - o_e.y;
    a_xaabb[ 2 ][ 1 ] = o_c.z - o_e.z;
}

void ComputeXAABBForFlat( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] )
{
    vec3 o_c;
    o_c.x = (cpo_w->e00 + cpo_w->e10) * 0.5f + cpo_w->e30;
    o_c.y = (cpo_w->e01 + cpo_w->e11) * 0.5f + cpo_w->e31;
    o_c.z = (cpo_w->e02 + cpo_w->e12) * 0.5f + cpo_w->e32;

    vec3 o_e;
    o_e.x = 0.5f * (::fabsf( cpo_w->e00 ) + ::fabsf( cpo_w->e10 ));
    o_e.y = 0.5f * (::fabsf( cpo_w->e01 ) + ::fabsf( cpo_w->e11 ));
    o_e.z = 0.5f * (::fabsf( cpo_w->e02 ) + ::fabsf( cpo_w->e12 ));

    a_xaabb[ 0 ][ 0 ] = o_c.x + o_e.x;
    a_xaabb[ 1 ][ 0 ] = o_c.y + o_e.y;
    a_xaabb[ 2 ][ 0 ] = o_c.z + o_e.z;

    a_xaabb[ 0 ][ 1 ] = o_c.x - o_e.x;
    a_xaabb[ 1 ][ 1 ] = o_c.y - o_e.y;
    a_xaabb[ 2 ][ 1 ] = o_c.z - o_e.z;
}

void ComputeXAABBDummy( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] )
{
}