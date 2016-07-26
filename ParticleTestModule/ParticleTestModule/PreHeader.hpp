#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#ifdef DEBUG
	#pragma comment( lib, "DXGUID.LIB" )
#endif

#include <StdAbstractionLib.hpp>
#include <CString.hpp>
#include <CVector.hpp>
#include <CLogger.hpp>
#include <LiceMathFuncs.hpp>
#include <FileMapping.hpp>
#include <RandomizingFuncs.hpp>

#pragma comment( lib, "StdCoreLib_Static.lib" )
#pragma comment( lib, "StdHelperLib_Static.lib" )
#pragma comment( lib, "StdAbstractionLib_Static.lib" )
#ifdef STDLIB_DYNAMIC
	#pragma comment( lib, "StdAbstractionLib_Dynamic.lib" )
	#pragma comment( lib, "StdCoreLib_Dynamic.lib" )
	//#pragma comment( lib, "StdHelperLib_Dynamic.lib" )
#endif

using namespace StdLib;

#include <BasicHeader.hpp>
#include <CObject.hpp>
#include <RendererGlobals.hpp>
#include <StatesManagers.hpp>
#include <TextureLoader.hpp>
#include <Globals.hpp>
#include <Misc.hpp>
#include <VectorFieldFileLoader.hpp>
#include <Bloom.hpp>
#include <Geometry.hpp>

#pragma comment( lib, "DX11.lib" )

#include <unordered_map>
#include <map>
#include <memory>

namespace std 
{
	template <> struct hash < StdLib::CStr >
	{
		size_t operator()( const StdLib::CStr& val ) const
		{
			return Funcs::FNVWordHash( (byte *)val.CStr(), val.Size() );
		}
	};
}

template < typename Type > inline void PTCOMDeleter( Type *object )
{
	if( object )
	{
		object->Release();
	}
}

struct alignas(32) SParticle
{
	vec3 position;
	f32 size;
	f128color color;
};

inline bln CompileShader( const wchar_t *path, std::shared_ptr < ID3D11ComputeShader > *ptr )
{
	FileIO::CFile file( path, FileOpenMode::OpenExisting, FileProcMode::Read );
	if( !file.IsOpened() )
	{
		SENDLOG( CLogger::Tag::error, "failed to open file with compute shader\n" );
		return false;
	}

	FileMapping::Mapping mapping( &file, 0, uiw_max, false );
	if( !mapping.IsOpened() )
	{
		SENDLOG( CLogger::Tag::error, "failed to create file mapping for compute shader\n" );
		return false;
	}

	COMUniquePtr < ID3DBlob > code;
    COMUniquePtr < ID3DBlob > errors;

    D3D_SHADER_MACRO macros[ 1 ] = {};

    HRESULT hr = ::D3DCompile( mapping.CMemory(), file.SizeGet(), 0, macros, 0, "Main", "cs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, code.AddrModifiable(), errors.AddrModifiable() );
    if( FAILED( hr ) )
    {
        if( errors != 0 )
        {
            SENDLOG( CLogger::Tag::error, "failed to d3dcompile compute shader with error %[*]s\n", errors->GetBufferSize(), (char *)errors->GetBufferPointer() );
        }
        else
        {
            SENDLOG( CLogger::Tag::error, "failed to d3dcompile compute shader\n" );
        }

        return false;
    }

	ID3D11ComputeShader *cs;

	DXHRCHECK( RendererGlobals::i_Device->CreateComputeShader( code->GetBufferPointer(), code->GetBufferSize(), 0, &cs ) );

#ifdef DEBUG
	char cname[ MAX_PATH ];
	wcstombs( cname, path, MAX_PATH );
	(cs)->SetPrivateData( WKPDID_D3DDebugObjectName, _StrLen( cname ), cname );
#endif

	ptr->reset( cs, PTCOMDeleter < ID3D11ComputeShader > );

	return true;
}