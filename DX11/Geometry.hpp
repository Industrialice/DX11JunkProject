#ifndef __GEOMETRY_HPP__
#define __GEOMETRY_HPP__

#include "LayoutsManager.hpp"

namespace VertexFmt
{
    CONSTS_OPED( vertexFmt_t, Position = BIT( 0 ),
                              Normal = BIT( 1 ),
                              Texcoord = BIT( 2 ),
                              Tangent = BIT( 3 ),
                              Binormal = BIT( 4 ) );
}

struct SGeometry
{
    void (*ComputeXAABB)( const SGeometry *cpo_geo, const m4x3 *cpo_w, f32 a_xaabb[ 3 ][ 2 ] ) = 0;
    ui32 instancesCount = 0;
    ui32 verticesCount = 0;
    ID3D11Buffer *i_ibuf = 0;
    ui32 indicesCount = 0;
    LayoutsManager::BufferDesc_t description;
    D3D11_PRIMITIVE_TOPOLOGY topo = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    bln is_32bitIndices = false;
    ui8 vbufsCount = 0;
    ID3D11Buffer *i_vbufs[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    ui32 strides[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
    ui32 offsets[ D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT ];
};

struct SGeometrySlice
{
    ui32 startVertex;
    ui32 startIndex;
    ui32 verticesCount;
    ui32 indicesCount;
    DBGCODE( bln is_defined; );

    SGeometrySlice()
    {
        DBGCODE( is_defined = false; );
    }

    SGeometrySlice( ui32 startVertexIn, ui32 startIndexIn, ui32 verticesCountIn, ui32 indicesCountIn )
    {
        startVertex = startVertexIn;
        startIndex = startIndexIn;
        verticesCount = verticesCountIn;
        indicesCount = indicesCountIn;
        DBGCODE( is_defined = true; );
    }
};

namespace Geometry
{
    DX11_EXPORT void Box( SGeometry *po_geo, bln is_center = true );
    DX11_EXPORT void BoxTN( SGeometry *po_geo, bln is_center = true );
    DX11_EXPORT void BoxTNIndexed( SGeometry *po_geo );
    DX11_EXPORT void BoxTNIndexedBottomless( SGeometry *po_geo );
    DX11_EXPORT void BoxIndexedBottomless( SGeometry *po_geo );
    DX11_EXPORT void FlatTN( SGeometry *po_geo );
    DX11_EXPORT void Flat( SGeometry *po_geo );
    DX11_EXPORT void FlatStrip( SGeometry *po_geo );
    DX11_EXPORT void Flat2( SGeometry *po_geo );
    DX11_EXPORT void FlatHalo( SGeometry *po_geo );
    DX11_EXPORT void TriCircle( SGeometry *po_geo, f32 radius );
    DX11_EXPORT void Make( SGeometry *po_geo, VertexFmt::vertexFmt_t *fmts, ui32 attribsCount, f32 *vbuf, ui32 vbufSize, ui32 verticesCount, ui16 *ibuf, ui32 indicesCount, D3D_PRIMITIVE_TOPOLOGY topo );
    DX11_EXPORT void StreamData( SGeometry *po_geo, CCRefVec < VertexBufferFieldDesc > desc, void *vertices, uiw verticesCount, uiw vertexSize );
}

#endif __GEOMETRY_HPP__