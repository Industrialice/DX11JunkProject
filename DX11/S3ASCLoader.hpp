#ifndef __S3ASCLOADER_HPP__
#define __S3ASCLOADER_HPP__

#include "Geometry.hpp"

namespace S3ASCLoader
{
    bln Load( const char *cp_buffer, ui32 bufLen, VertexFmt::vertexFmt_t **vertexFmts, ui32 *attribsCount, ui16 *vertexCount, f32 **vbuf, ui32 *vbufSize, ui32 *indexCount, ui16 **ibuf );
    bln LoadFromFile( const char *cp_pnn, VertexFmt::vertexFmt_t **vertexFmts, ui32 *attribsCount, ui16 *vertexCount, f32 **vbuf, ui32 *vbufSize, ui32 *indexCount, ui16 **ibuf );
}

#endif __S3ASCLOADER_HPP__