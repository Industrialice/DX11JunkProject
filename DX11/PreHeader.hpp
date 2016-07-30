#include <math.h>
#include <memory>

#include <StdCoreLib.hpp>
#include <LiceMathFuncs.hpp>
#include <CLogger.hpp>
#include <FileCFILEStream.hpp>
#include <RandomizingFuncs.hpp>

#include <CVector.hpp>
#include <CString.hpp>
#include "Heap.hpp"

#include <d3dcompiler.h>
#include <d3d11.h>

#include <D3DX11.h>
#pragma comment( lib, "d3dx11.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "StdCoreLib_Static.lib" )
#pragma comment( lib, "StdHelperLib_Static.lib" )
#pragma comment( lib, "StdAbstractionLib_Static.lib" )
#ifdef STDLIB_DYNAMIC
	#pragma comment( lib, "StdAbstractionLib_Dynamic.lib" )
	#pragma comment( lib, "StdCoreLib_Dynamic.lib" )
	//#pragma comment( lib, "StdHelperLib_Dynamic.lib" )
#endif

#ifdef DEBUG
	#pragma comment( lib, "DXGUID.LIB" )
#endif

using namespace StdLib;

#include "BasicHeader.hpp"

namespace Globals
{
    extern heapid DHeap;
}

FORCEINLINE void *__CRTDECL operator new( std::size_t size ) NOEXCEPT
{
    return Heap::Alloc( size );
}

FORCEINLINE void *__CRTDECL operator new[]( std::size_t size ) NOEXCEPT
{
    return Heap::Alloc( size );
}

FORCEINLINE void *__CRTDECL operator new( std::size_t size, const std::nothrow_t &tag ) NOEXCEPT
{
    return Heap::Alloc( size );
}

FORCEINLINE void *__CRTDECL operator new[]( std::size_t size, const std::nothrow_t &tag ) NOEXCEPT
{
    return Heap::Alloc( size );
}

FORCEINLINE void __CRTDECL operator delete( void *mem ) NOEXCEPT
{
    Heap::Free( mem );
}

FORCEINLINE void __CRTDECL operator delete[]( void *mem ) NOEXCEPT
{
    Heap::Free( mem );
}

FORCEINLINE void __CRTDECL operator delete( void *mem, const std::nothrow_t &tag ) NOEXCEPT
{
    Heap::Free( mem );
}

FORCEINLINE void __CRTDECL operator delete[]( void *mem, const std::nothrow_t &tag ) NOEXCEPT
{
    Heap::Free( mem );
}

#define PROPERTY( name, var, type ) \
    type CONCAT( name, Get )() const \
    { \
        return var; \
    } \
    \
    void CONCAT( name, Set )( type CONCAT( $, var ) ) \
    { \
        var = CONCAT( $, var ); \
    }

#define PROPERTYG( name, var, type ) \
    type CONCAT( name, Get )() const \
    { \
        return var; \
    }

#define PROPERTYS( name, var, type ) \
    void CONCAT( name, Set )( type CONCAT( $, var ) ) \
    { \
        var = CONCAT( $, var ); \
    }

#define PROPERTYC( name, var, retType, sourceType, returnCode, setCode ) \
    retType CONCAT( name, Get )() const \
    { \
        return returnCode; \
    } \
    \
    void CONCAT( name, Set )( sourceType source ) \
    { \
        setCode; \
    }

#define PROPERTYCG( name, var, retType, returnCode ) \
    retType CONCAT( name, Get )() const \
    { \
        return returnCode; \
    }

#define PROPERTYSC( name, var, sourceType, setCode ) \
    void CONCAT( name, Set )( sourceType source ) \
    { \
        setCode; \
    }

#define rep( x ) for( uiw __i_n_d_e_x__ = 0; __i_n_d_e_x__ < (x); ++__i_n_d_e_x__ )
#define rep_index __i_n_d_e_x__

typedef i16 sct;
#define SCT_SIZE 16

#ifdef DEBUG
    #define dnull = 0
#else
    #define dnull
#endif

namespace Console
{
    enum class color_t { red, green, blue, white, black, yellow, cyan, magenta };

    static void Write( const char *text, ui32 len, color_t foreColor, color_t backColor )
    {
        static const WORD foreColorConsts[ 8 ] = { FOREGROUND_RED, 
                                                   FOREGROUND_GREEN,
                                                   FOREGROUND_BLUE,
                                                   FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
                                                   0,
                                                   FOREGROUND_RED | FOREGROUND_GREEN,
                                                   FOREGROUND_GREEN | FOREGROUND_BLUE,
                                                   FOREGROUND_RED | FOREGROUND_BLUE };
        static const WORD backColorConsts[ 8 ] = { BACKGROUND_RED, 
                                                   BACKGROUND_GREEN,
                                                   BACKGROUND_BLUE,
                                                   BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
                                                   0,
                                                   BACKGROUND_RED | BACKGROUND_GREEN,
                                                   BACKGROUND_GREEN | BACKGROUND_BLUE,
                                                   BACKGROUND_RED | BACKGROUND_BLUE };

        WORD colors = foreColorConsts[ (int)foreColor ] | backColorConsts[ (int)backColor ];
        if( foreColor != color_t::black )
        {
            colors |= FOREGROUND_INTENSITY;
        }
        if( backColor != color_t::black )
        {
            colors |= BACKGROUND_INTENSITY;
        }

        HANDLE hdl = ::GetStdHandle( STD_OUTPUT_HANDLE );
        if( hdl != INVALID_HANDLE_VALUE )
        {
            ::SetConsoleTextAttribute( hdl, colors );
            ::WriteConsoleA( hdl, text, len, 0, 0 );
        }
    }
}

using DefaultFileType = FileCFILEStream;