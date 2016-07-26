#include "PreHeader.hpp"
#include "S3ASCLoader.hpp"
#include <FileIO.hpp>
#include "Heap.hpp"

static const char *NewLine( const char *buf )
{
    const char *bufNew = _StrChr( buf, '\n' );
    ASSUME( bufNew );
    return bufNew + 1;
}

bln S3ASCLoader::Load( const char *cp_buffer, ui32 bufLen, VertexFmt::vertexFmt_t **vertexFmts, ui32 *attribsCount, ui16 *vertexCount, f32 **vbuf, ui32 *vbufSize, ui32 *indexCount, ui16 **ibuf )
{
    ASSUME( cp_buffer && vertexFmts && vertexCount && vbuf && vbufSize && indexCount && ibuf );
    *vbuf = 0;
    *ibuf = 0;
    *vertexFmts = 0;
    const char *bufPos = cp_buffer;
    if( !Funcs::MemEquals( bufPos, "vrtf", 4 ) )
    {
		SOFTBREAK;
        goto failedExit;
    }
    bufPos += 5;
    ui32 attrsCount = Funcs::StrDecToUI32( bufPos, Funcs::StrNotChrs( bufPos, "0123456789" ) - bufPos );
    bufPos = NewLine( bufPos );
    *vertexFmts = Heap::Alloc<VertexFmt::vertexFmt_t>( attrsCount );
    *attribsCount = 0;
    ui32 vertexSize = 0;
    for( ui32 va = 0; va < attrsCount; ++va )
    {
        ui32 vertexNum = Funcs::StrDecToUI32( bufPos, uiw_max, ' ' );
        if( vertexNum != va )
        {
			SOFTBREAK;
            goto failedExit;
        }
        bufPos = _StrChr( bufPos, ' ' ) + 1;
        ui32 rawFmt = Funcs::StrDecToUI32( bufPos, uiw_max, ' ' );;
        if( rawFmt == 0 )
        {
            (*vertexFmts)[ *attribsCount ] = VertexFmt::Position;
            (*attribsCount)++;
            vertexSize += sizeof(f32) * 3;
        }
        else if( rawFmt == 1 )
        {
            (*vertexFmts)[ *attribsCount ] = VertexFmt::Normal;
            (*attribsCount)++;
            vertexSize += sizeof(f32) * 3;
        }
        else if( rawFmt == 2 )
        {
            (*vertexFmts)[ *attribsCount ] = VertexFmt::Texcoord;
            (*attribsCount)++;
            vertexSize += sizeof(f32) * 2;
        }
        bufPos = NewLine( bufPos );
    }
    const char *vbufStart = _StrStr( cp_buffer, "vbuf" );
    if( !vbufStart )
    {
		SOFTBREAK;
        goto failedExit;
    }
    const char *ibufStart = _StrStr( vbufStart, "ibuf" );
    if( !ibufStart )
    {
		SOFTBREAK;
        goto failedExit;
    }
    bufPos = vbufStart + sizeof("vbuf");
    *vertexCount = Funcs::StrDecToUI32( bufPos, Funcs::StrNotChrs( bufPos, "0123456789" ) - bufPos );
    bufPos = NewLine( bufPos );
    *vbuf = (f32 *)Heap::Alloc( *vertexCount * vertexSize );
    f32 *vbufWrite = *vbuf;
    for( ui32 vertex = 0; vertex < *vertexCount; ++vertex )
    {
        for( ui32 va = 0; va < attrsCount; ++va )
        {
            ui32 vertexNum = Funcs::StrDecToUI32( bufPos, uiw_max, ' ' );
            if( vertexNum != vertex )
            {
				SOFTBREAK;
                goto failedExit;
            }
            bufPos = _StrChr( bufPos, ' ' ) + 1;
            ui32 vaCur = Funcs::StrDecToUI32( bufPos, uiw_max, ' ' );
            if( vaCur != 0 && vaCur != 1 && vaCur != 2 )
            {
                bufPos = NewLine( bufPos );
            }
            else
            {
                bufPos = _StrChr( bufPos, ' ' ) + 1;
                for( ; ; )
                {
                    char *end = Funcs::StrNotChrs( bufPos, "0123456789eE+-." );
                    char tempBuf[ 1024 ];
                    _MemCpy( tempBuf, bufPos, end - bufPos );
                    tempBuf[ end - bufPos ] = '\0';
                    *vbufWrite = Funcs::StrToF32( tempBuf );
                    /*if( vaCur == 1 )
                    {
                        *vbufWrite = -*vbufWrite;
                    }*/
                    ++vbufWrite;
                    if( *end != ' ' )
                    {
                        bufPos = NewLine( bufPos );
                        break;
                    }
                    else
                    {
                        bufPos = end + 1;
                    }
                }
            }
        }
    }
    *vbufSize = (vbufWrite - *vbuf) * sizeof(f32);
    bufPos = ibufStart + sizeof("ibuf");
    ui32 triCount = Funcs::StrDecToUI32( bufPos, Funcs::StrNotChrs( bufPos, "0123456789" ) - bufPos );
    *indexCount = triCount * 3;
    bufPos = NewLine( bufPos );
    *ibuf = Heap::Alloc<ui16>( *indexCount );
    ui16 *ibufWrite = *ibuf;
    for( ui32 tri = 0; tri < triCount; ++tri )
    {
        ui32 triNumber = Funcs::StrDecToUI32( bufPos, uiw_max, ' ' );
        if( triNumber != tri )
        {
			SOFTBREAK;
            goto failedExit;
        }
        bufPos = _StrChr( bufPos, ' ' ) + 1;
        char *next = _StrChr( bufPos, ' ' );
        *ibufWrite++ = Funcs::StrDecToUI32( bufPos, next - bufPos );
        bufPos = next + 1;
        next = _StrChr( bufPos, ' ' );
        *ibufWrite++ = Funcs::StrDecToUI32( bufPos, next - bufPos );
        bufPos = next + 1;
        next = Funcs::StrNotChrs( bufPos, "0123456789" );
        *ibufWrite++ = Funcs::StrDecToUI32( bufPos, next - bufPos );
        bufPos = NewLine( bufPos );
    }

    return true;

failedExit:
    Heap::Free( *vbuf );
    Heap::Free( *ibuf );
    Heap::Free( *vertexFmts );
    return false;
}

bln S3ASCLoader::LoadFromFile( const char *cp_pnn, VertexFmt::vertexFmt_t **vertexFmts, ui32 *attribsCount, ui16 *vertexCount, f32 **vbuf, ui32 *vbufSize, ui32 *indexCount, ui16 **ibuf )
{
    ASSUME( cp_pnn && vertexFmts && vertexCount && vbuf && vbufSize && indexCount && ibuf );
	wchar_t widePath[ MAX_PATH_LENGTH ];
	::mbstowcs( widePath, cp_pnn, MAX_PATH_LENGTH );
    DefaultFileType file( widePath, FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead );
    if( !file.IsOpened() )
    {
        return false;
    }
    ui32 fileSize = file.SizeGet();
    UniquePtr< char, Heap::DefaultDeleter > buf = Heap::Alloc<char>( fileSize );
    file.Read( buf, fileSize, 0 );
    file.Close();
    bln result = Load( buf, fileSize, vertexFmts, attribsCount, vertexCount, vbuf, vbufSize, indexCount, ibuf );
    return result;
}