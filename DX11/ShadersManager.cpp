#include "PreHeader.hpp"
#include "ShadersManager.hpp"
#include "Globals.hpp"
#include <FileIO.hpp>
#include "Camera.hpp"
#include <Misc.hpp>
#include <Files.hpp>
#include "RendererGlobals.hpp"
#include <deprecatedCFramedStore.hpp>

struct ShaderStruct
{
    ID3D11VertexShader *i_vs;
    ID3D11GeometryShader *i_gs;
    ID3D11PixelShader *i_ps;
    LayoutsManager::ShaderInputDesc_t compiledShaderInputDesc;
    CStr p_name;
    shaderCode_t o_code;
    bln is_useScreenConsts;

    void Delete()
    {
    }

    ShaderStruct()
    {
        i_vs = 0;
    }

    ShaderStruct( ID3D11VertexShader *i_vsIn, ID3D11GeometryShader *i_gsIn, ID3D11PixelShader *i_psIn, bln is_useScreenConstsIn, shaderCode_t *codeIn, const char *cp_nameIn )
    {
        i_vs = i_vsIn;
        i_gs = i_gsIn;
        i_ps = i_psIn;
        o_code = *codeIn;
        p_name = cp_nameIn;
        is_useScreenConsts = is_useScreenConstsIn;
    }
};

namespace
{
    struct ShaderStruct o_DummyShader;

    sdrhdl CurrShader = &o_DummyShader;

    struct SShaderStore
    {
        ShaderStruct o_shader;
        uiw users;

        SShaderStore()
        {
        }

        SShaderStore( const ShaderStruct &o_shaderIn )
        {
            o_shader = o_shaderIn;
            users = 0;
        }

        SShaderStore( const ShaderStruct &o_shaderIn, uiw usersIn )
        {
            o_shader = o_shaderIn;
            users = usersIn;
        }
    };

    CFramedStore < SShaderStore, ID3D11VertexShader *, 0, offsetof( ShaderStruct, i_vs ) > o_Shaders;

    bln is_PSSkipped;
    ID3D11VertexShader *i_AppliedVS;
    ID3D11GeometryShader *i_AppliedGS;
    ID3D11PixelShader *i_AppliedPS;

    struct SShaderCacheEntry
    {
        ui32 checksum;
        ui32 len;
        ui32 fileOffset;
        ui32 totalSize;
    };
    CVec < SShaderCacheEntry > ShaderCaches;

    #ifdef DEBUG
        #define CACHEDIR L"Cache_debug"
    #else
        #define CACHEDIR L"Cache_release"
    #endif

    DefaultFileType ShaderCachesFile;
    DefaultFileType ShaderCachesInfoFile;

    bln is_DoNotUseCache;
}

static bln GetCompiledCache( const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, shaderCode_t *po_sdrCode );

void ShadersManager::ApplyShader( sdrhdl shader, bln is_skipPS )
{
    ID3D11PixelShader *i_ps = is_skipPS ? nullptr : shader->i_ps;
    if( i_AppliedPS != i_ps )
    {
        RendererGlobals::i_ImContext->PSSetShader( i_AppliedPS = i_ps, 0, 0 );
    }
    is_PSSkipped = is_skipPS;

    if( CurrShader != shader )
    {
        if( i_AppliedVS != shader->i_vs )
        {
            RendererGlobals::i_ImContext->VSSetShader( i_AppliedVS = shader->i_vs, 0, 0 );
        }

        if( i_AppliedGS != shader->i_gs )
        {
            RendererGlobals::i_ImContext->GSSetShader( i_AppliedGS = shader->i_gs, 0, 0 );
        }

        CurrShader = shader;
    }
}

sdrhdl ShadersManager::CurrentShader()
{
    return CurrShader;
}

#pragma optimize( "s", on )

bln ShadersManager::Create( const char *cp_name, const byte *cp_vsCode, uiw vsCodeLen, const byte *cp_gsCode, uiw gsCodeLen, const byte *cp_psCode, uiw psCodeLen, bln is_screenConsts, CCRefVec < CStr > shaderInputsDesc, const D3D11_SO_DECLARATION_ENTRY *cpo_soDesc, uiw soDescLen, const UINT *cp_strides, UINT stridesCount, UINT rasterizedStream )
{
    ASSUME( cp_name && cp_vsCode && vsCodeLen );

    ID3D11VertexShader *i_vs;
    ID3D11GeometryShader *i_gs = 0;
    ID3D11PixelShader *i_ps = 0;
    shaderCode_t o_vsCode, o_othersCode;
    #if defined(DEBUG)
        const UINT flags0 = D3DCOMPILE_DEBUG;
    #else
        const UINT flags0 = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_SKIP_VALIDATION;
    #endif
    D3D_SHADER_MACRO ao_macros[ 5 ] = {};
    ui32 macroIndex = 0;

    SENDLOG( CLogger::Tag::info, "ShadersManager::Create creating %s\n", cp_name );

    char a_screenMacroBufs[ 4 ][ 64 ];
    if( is_screenConsts )
    {
        ao_macros[ macroIndex ].Name = "SCREEN_WIDTH";
        Funcs::F32ToStr( RendererGlobals::RenderingWidth, a_screenMacroBufs[ 0 ] );
        ao_macros[ macroIndex ].Definition = a_screenMacroBufs[ 0 ];
        ++macroIndex;

        ao_macros[ macroIndex ].Name = "SCREEN_HEIGHT";
        Funcs::F32ToStr( RendererGlobals::RenderingHeight, a_screenMacroBufs[ 1 ] );
        ao_macros[ macroIndex ].Definition = a_screenMacroBufs[ 1 ];
        ++macroIndex;

        ao_macros[ macroIndex ].Name = "PIXEL_SIZE_X";
        Funcs::F32ToStr( 1.f / RendererGlobals::RenderingWidth, a_screenMacroBufs[ 2 ] );
        ao_macros[ macroIndex ].Definition = a_screenMacroBufs[ 2 ];
        ++macroIndex;

        ao_macros[ macroIndex ].Name = "PIXEL_SIZE_Y";
        Funcs::F32ToStr( 1.f / RendererGlobals::RenderingHeight, a_screenMacroBufs[ 3 ] );
        ao_macros[ macroIndex ].Definition = a_screenMacroBufs[ 3 ];
        ++macroIndex;
    }

    bln compileGet = GetCompiledCache( cp_vsCode, vsCodeLen, "VS", "vs_5_0", flags0, ao_macros, &o_vsCode );
    if( !compileGet )
    {
        SENDLOG( CLogger::Tag::error, "ShadersManager::Create failed to compile vs shader for %s\n", cp_name );
        ::FatalAppExitA( 1, "failed to compile vs" );
    }
    DXHRCHECK( RendererGlobals::i_Device->CreateVertexShader( o_vsCode.Data(), o_vsCode.Size(), 0, &i_vs ) );

    if( cpo_soDesc && !cp_gsCode )
    {
        DXHRCHECK( RendererGlobals::i_Device->CreateGeometryShaderWithStreamOutput( o_vsCode.Data(), o_vsCode.Size(), cpo_soDesc, soDescLen, cp_strides, stridesCount, rasterizedStream, 0, &i_gs ) );
    }

    if( cp_gsCode )
    {
        bln compileGet = GetCompiledCache( cp_gsCode, gsCodeLen, "GS", "gs_5_0", flags0, ao_macros, &o_othersCode );
        if( !compileGet )
        {
            SENDLOG( CLogger::Tag::error, "ShadersManager::Create failed to compile gs shader for %s\n", cp_name );
            ::FatalAppExitA( 1, "failed to compile gs" );
        }
        if( cpo_soDesc )
        {
            DXHRCHECK( RendererGlobals::i_Device->CreateGeometryShaderWithStreamOutput( o_othersCode.Data(), o_othersCode.Size(), cpo_soDesc, soDescLen, cp_strides, stridesCount, rasterizedStream, 0, &i_gs ) );
        }
        else
        {
            DXHRCHECK( RendererGlobals::i_Device->CreateGeometryShader( o_othersCode.Data(), o_othersCode.Size(), 0, &i_gs ) );
        }
    }

    if( cp_psCode )
    {
        bln compileGet = GetCompiledCache( cp_psCode, psCodeLen, "PS", "ps_5_0", flags0, ao_macros, &o_othersCode );
        if( !compileGet )
        {
            SENDLOG( CLogger::Tag::error, "ShadersManager::Create failed to compile ps shader for %s\n", cp_name );
            ::FatalAppExitA( 1, "failed to compile ps" );
        }
        DXHRCHECK( RendererGlobals::i_Device->CreatePixelShader( o_othersCode.Data(), o_othersCode.Size(), 0, &i_ps ) );
    }

    SShaderStore *store = o_Shaders.Add( SShaderStore( ShaderStruct( i_vs, i_gs, i_ps, is_screenConsts, &o_vsCode, cp_name ) ) );

    store->o_shader.compiledShaderInputDesc = RendererGlobals::DefLayoutsManager.CompileShaderInputDesc( shaderInputsDesc );

    return true;
}

bln ShadersManager::CreateFromFiles( const char *cp_name, const char *cp_vsFilePNN, const char *cp_gsFilePNN, const char *cp_psFilePNN, bln is_screenConsts, CCRefVec < CStr > shaderInputsDesc, const D3D11_SO_DECLARATION_ENTRY *cpo_soDesc, uiw soDescLen, const UINT *cp_strides, UINT stridesCount, UINT rasterizedStream )
{
	wchar_t vsFilePNN[ MAX_PATH_LENGTH ];
	::mbstowcs( vsFilePNN, cp_vsFilePNN, MAX_PATH_LENGTH );
    DefaultFileType vsFile( vsFilePNN, FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead );
    if( !vsFile.IsOpened() )
    {
        char a_buf[ 1024 ];
        Funcs::PrintToStr( a_buf, 1023, "CreateFromFiles can't open vs file %s", cp_vsFilePNN );
        ::FatalAppExitA( 1, a_buf );
    }
    uiw vsFileSize = vsFile.SizeGet();

    DefaultFileType gsFile;
    uiw gsFileSize = 0;
    if( cp_gsFilePNN )
    {
		wchar_t gsFilePNN[ MAX_PATH_LENGTH ];
		::mbstowcs( gsFilePNN, cp_gsFilePNN, MAX_PATH_LENGTH );
        gsFile.Open( gsFilePNN, FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead );
        if( !gsFile.IsOpened() )
        {
            char a_buf[ 1024 ];
            Funcs::PrintToStr( a_buf, 1023, "CreateFromFiles can't open gs file %s", cp_gsFilePNN );
            ::FatalAppExitA( 1, a_buf );
        }
        gsFileSize = gsFile.SizeGet();
    }

    DefaultFileType psFile;
    uiw psFileSize = 0;
    if( cp_psFilePNN )
    {
		wchar_t psFilePNN[ MAX_PATH_LENGTH ];
		::mbstowcs( psFilePNN, cp_psFilePNN, MAX_PATH_LENGTH );
        psFile.Open( psFilePNN, FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead );
        if( !psFile.IsOpened() )
        {
            char a_buf[ 1024 ];
            Funcs::PrintToStr( a_buf, 1023, "CreateFromFiles can't open ps file %s", cp_psFilePNN );
            ::FatalAppExitA( 1, a_buf );
        }
        psFileSize = psFile.SizeGet();
    }

    UniquePtr< byte, Heap::DefaultDeleter > p_vs = Heap::Alloc<byte>( vsFileSize );
    UniquePtr< byte, Heap::DefaultDeleter > p_gs = cp_gsFilePNN ? Heap::Alloc<byte>( gsFileSize ) : 0;
    UniquePtr< byte, Heap::DefaultDeleter > p_ps = cp_psFilePNN ? Heap::Alloc<byte>( psFileSize ) : 0;

    vsFile.Read( p_vs, vsFileSize, 0 );

    if( gsFileSize )
    {
        gsFile.Read( p_gs, gsFileSize, 0 );
    }

    if( psFileSize )
    {
        psFile.Read( p_ps, psFileSize, 0 );
    }

    bln result = ShadersManager::Create( cp_name, p_vs, vsFileSize, p_gs, gsFileSize, p_ps, psFileSize, is_screenConsts, shaderInputsDesc, cpo_soDesc, soDescLen, cp_strides, stridesCount, rasterizedStream );

    return result;
}

sdrhdl ShadersManager::AcquireByName( const char *cp_name )
{
    uiw frame = 0, index = 0;
    for( uiw enu = 0; enu < o_Shaders.Size(); ++enu )
    {
        SShaderStore &o_shaderStore = o_Shaders.Enumerate( &frame, &index );
        if( _StrEqual( cp_name, o_shaderStore.o_shader.p_name.CStr() ) )
        {
            ++o_shaderStore.users;
            return &o_shaderStore.o_shader;
        }
    }

    SOFTBREAK;
    return 0;
}

bln ShadersManager::TryToBlend( LayoutsManager::BufferDesc_t bufferDesc, sdrhdl shader, ID3D11InputLayout **pi_lo )
{
    ASSUME( shader != nullptr );
    return RendererGlobals::DefLayoutsManager.Blend( bufferDesc, shader->compiledShaderInputDesc, shader->o_code, pi_lo );
}

void ShadersManager::Private::Initialize( bln is_useCache )
{
	is_DoNotUseCache = !is_useCache;

	if( is_useCache )
	{
		if( !Files::IsFolder( L"Shaders\\" CACHEDIR ) )
		{
			if( !Files::CreateNewFolder( L"Shaders\\", CACHEDIR, true ) )
			{
				::FatalAppExitW( 1, L"can't create shader cache dir " CACHEDIR );
			}
			return;
		}

		ShaderCachesInfoFile.Open( "Shaders\\" CACHEDIR "\\info.sci", FileOpenMode::OpenExisting, FileProcMode::Read, FileCacheMode::LinearRead );
		if( ShaderCachesInfoFile.IsOpened() )
		{
			byte buffer[ 2048 ];
			ShaderCachesInfoFile.BufferSet( sizeof(buffer), buffer );
			ui32 fileSize = ShaderCachesInfoFile.SizeGet();
			SShaderCacheEntry newEntry;
			ui32 count = fileSize / (sizeof(newEntry.checksum) + sizeof(newEntry.len) + sizeof(newEntry.fileOffset) + sizeof(newEntry.totalSize));
			ShaderCaches.Resize( count );
			for( ui32 entry = 0; entry < count; ++entry )
			{
				ShaderCachesInfoFile.Read( &newEntry.checksum, sizeof(newEntry.checksum), 0 );
				ShaderCachesInfoFile.Read( &newEntry.len, sizeof(newEntry.len), 0 );
				ShaderCachesInfoFile.Read( &newEntry.fileOffset, sizeof(newEntry.fileOffset), 0 );
				ShaderCachesInfoFile.Read( &newEntry.totalSize, sizeof(newEntry.totalSize), 0 );
				ShaderCaches[ entry ] = newEntry;
			}

			ShaderCachesInfoFile.Close();
		}
		else if( Files::IsFile( "Shaders\\" CACHEDIR "\\info.sci" ) )
		{
			is_DoNotUseCache = true;
		}

		SENDLOG( CLogger::Tag::important, "cached shaders count %u\n", ShaderCaches.Size() );
	}
}

#pragma optimize( "", on )

void ShadersManager::Private::EndFrame()
{
}

void ShadersManager::Private::BeginFrame()
{
    struct SFrameConsts
    {
        vec4 camDt;
        m4x4 viewProjTrans;
        vec4 xvec, yvec, zvec;
        m4x4 projTrans;
    } frameConsts;

    frameConsts.camDt = vec4( RendererGlobals::MainCamera.PositionGet(), Globals::DT );
    frameConsts.viewProjTrans = *RendererGlobals::MainCamera.ViewProjectionTransposed();
    frameConsts.xvec = vec4( *RendererGlobals::MainCamera.XVec(), 1.f );
    frameConsts.yvec = vec4( *RendererGlobals::MainCamera.YVec(), 1.f );
    frameConsts.zvec = vec4( *RendererGlobals::MainCamera.ZVec(), 1.f );
    LiceMath::M4x4Transpose( &frameConsts.projTrans, RendererGlobals::MainCamera.Projection() );

    D3D11_MAPPED_SUBRESOURCE o_sr;

    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_PSShaderRegisters[ FRAME_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &frameConsts, sizeof(frameConsts) );
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_PSShaderRegisters[ FRAME_DATA_BUF ], 0 );

    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_GSShaderRegisters[ FRAME_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &frameConsts, sizeof(frameConsts) );
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_GSShaderRegisters[ FRAME_DATA_BUF ], 0 );

    DXHRCHECK( RendererGlobals::i_ImContext->Map( RendererGlobals::ai_VSShaderRegisters[ FRAME_DATA_BUF ], 0, D3D11_MAP_WRITE_DISCARD, 0, &o_sr ) );
    _MemCpy( o_sr.pData, &frameConsts, sizeof(frameConsts) );
    RendererGlobals::i_ImContext->Unmap( RendererGlobals::ai_VSShaderRegisters[ FRAME_DATA_BUF ], 0 );
}

static ui32 GenCheckSum( const byte *code, ui32 codeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros )
{
    ui32 sum = 0;
    for( ui32 offset = 0; codeLen; --codeLen )
    {
        if( offset == 24 )
        {
            offset = 0;
        }
        sum += *code << offset;
        ++code;
    }
    sum += Funcs::FNV32Hash( (byte *)entryPoint, _StrLen( entryPoint ) );
    sum += Funcs::FNV32Hash( (byte *)target, _StrLen( target ) );
    sum += flags;
    for( ui32 macro = 0; macros[ macro ].Name; ++macro )
    {
        sum += Funcs::FNV32Hash( (byte *)macros[ macro ].Name, _StrLen( macros[ macro ].Name ) );
        if( macros[ macro ].Definition )
        {
            sum += Funcs::FNV32Hash( (byte *)macros[ macro ].Definition, _StrLen( macros[ macro ].Definition ) );
        }
    }
    return sum;
}

#define ReadTo( target, source, size ) { _MemCpy( target, source, size ); (byte *&)source += size; }

static bln IsContentionMatch( const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, const char *testBuf, ui32 testBufLen, byte **cachedCode, ui32 *cachedCodeLen )
{
    ASSUME( sourceCode && entryPoint && target && macros && cachedCode && cachedCodeLen && testBuf );
    UINT cachedFlags;
	ReadTo( &cachedFlags, testBuf, sizeof(UINT) );
    if( cachedFlags != flags )
    {
        return false;
    }
    ui32 cachedTargetLen;
	ReadTo( &cachedTargetLen, testBuf, 4 );
    if( cachedTargetLen != _StrLen( target ) )
    {
        return false;
    }
    if( !_MemEquals( testBuf, target, cachedTargetLen ) )
    {
        return false;
    }
    testBuf += cachedTargetLen;
    ui32 cachedEntryPointLen;
	ReadTo( &cachedEntryPointLen, testBuf, 4 );
    if( cachedEntryPointLen != _StrLen( entryPoint ) )
    {
        return false;
    }
    if( !_MemEquals( testBuf, entryPoint, cachedEntryPointLen ) )
    {
        return false;
    }
    testBuf += cachedEntryPointLen;
    ui32 cachedMacroCount;
	ReadTo( &cachedMacroCount, testBuf, 4 );
    ui32 sourceMacroCount = 0;
    if( macros )
    {
        for( ; macros[ sourceMacroCount ].Name; ++sourceMacroCount );
    }
    if( cachedMacroCount != sourceMacroCount )
    {
        return false;
    }
    for( ui32 macroIndex = 0; macroIndex < cachedMacroCount; ++macroIndex )
    {
        ui32 cachedMacroNameLen;
		ReadTo( &cachedMacroNameLen, testBuf, 4 );
        if( cachedMacroNameLen != _StrLen( macros[ macroIndex ].Name ) )
        {
            return false;
        }
        if( !_MemEquals( testBuf, macros[ macroIndex ].Name, cachedMacroNameLen ) )
        {
            return false;
        }
        testBuf += cachedMacroNameLen;
        ui32 cachedMacroDefLen;
		ReadTo( &cachedMacroDefLen, testBuf, 4 );
        ui32 sourceMacroDefLen = 0;
        if( macros[ macroIndex ].Definition )
        {
            sourceMacroDefLen = _StrLen( macros[ macroIndex ].Definition );
        }
        if( cachedMacroDefLen != sourceMacroDefLen )
        {
            return false;
        }
        if( !_MemEquals( testBuf, macros[ macroIndex ].Definition, cachedMacroDefLen ) )
        {
            return false;
        }
        testBuf += cachedMacroDefLen;
    }
    ui32 cachedSourceCodeLen;
	ReadTo( &cachedSourceCodeLen, testBuf, 4 );
    if( cachedSourceCodeLen != sourceCodeLen )
    {
        SOFTBREAK;
        return false;
    }
    if( !_MemEquals( testBuf, sourceCode, cachedSourceCodeLen ) )
    {
        return false;
    }
    testBuf += cachedSourceCodeLen;
	ReadTo( cachedCodeLen, testBuf, 4 );
    *cachedCode = (byte *)testBuf;
    return true;
}

static void FlushCacheInfoEntry( SShaderCacheEntry *entry )
{
    ASSUME( ShaderCachesInfoFile.IsOpened() && entry );

    ShaderCachesInfoFile.Write( &entry->checksum, sizeof(entry->checksum) );
    ShaderCachesInfoFile.Write( &entry->len, sizeof(entry->len ) );
    ShaderCachesInfoFile.Write( &entry->fileOffset, sizeof(entry->fileOffset) );
    ShaderCachesInfoFile.Write( &entry->totalSize, sizeof(entry->totalSize) );

    ShaderCachesInfoFile.Flush();
}

static void AddCacheToFile( ui32 checksum, const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, const byte *compiledCode, ui32 compiledCodeLen )
{
    ASSUME( sourceCode && entryPoint && target && macros && compiledCode );

    if( !ShaderCachesInfoFile.IsOpened() )
    {
        FileIO::fileError error;
        ShaderCachesInfoFile.Open( "Shaders\\" CACHEDIR "\\info.sci", FileOpenMode::CreateIfDoesNotExist, FileProcMode::WriteAppend, FileCacheMode::Default, &error );
        if( ShaderCachesInfoFile.IsOpened() )
        {
            ShaderCachesInfoFile.BufferSet( 2048 );
        }
        else
        {
            SENDLOG( CLogger::Tag::important, "#AddCacheToFile failed to open shader cache info.sci to write, can't add a new cache entry, reason %s:%s\n", error.Description(), error.Addition() );
            return;
        }
    }

    i64 fileOffset = ShaderCachesFile.OffsetSet( FileOffsetMode::FromEnd, 0, 0 );
    ShaderCachesFile.Write( &flags, sizeof(UINT) );
    ui32 targetLen = _StrLen( target );
    ShaderCachesFile.Write( &targetLen, 4 );
    ShaderCachesFile.Write( target, targetLen );
    ui32 entryPointLen = _StrLen( entryPoint );
    ShaderCachesFile.Write( &entryPointLen, 4 );
    ShaderCachesFile.Write( entryPoint, entryPointLen );
    ui32 macroCount = 0;
    if( macros )
    {
        for( ; macros[ macroCount ].Name; ++macroCount );
    }
    ShaderCachesFile.Write( &macroCount, 4 );
    for( ui32 macroIndex = 0; macroIndex < macroCount; ++macroIndex )
    {
        ui32 nameLen = _StrLen( macros[ macroIndex ].Name );
        ShaderCachesFile.Write( &nameLen, 4 );
        ShaderCachesFile.Write( macros[ macroIndex ].Name, nameLen );
        ui32 defLen = 0;
        if( macros[ macroIndex ].Definition )
        {
            defLen = _StrLen( macros[ macroIndex ].Definition );
        }
        ShaderCachesFile.Write( &defLen, 4 );
        ShaderCachesFile.Write( macros[ macroIndex ].Definition, defLen );
    }
    ShaderCachesFile.Write( &sourceCodeLen, 4 );
    ShaderCachesFile.Write( sourceCode, sourceCodeLen );
    ShaderCachesFile.Write( &compiledCodeLen, 4 );
    ShaderCachesFile.Write( compiledCode, compiledCodeLen );

    SShaderCacheEntry entry;
    entry.fileOffset = fileOffset;
    entry.len = sourceCodeLen;
    entry.checksum = checksum;
    entry.totalSize = ShaderCachesFile.OffsetGet( FileOffsetMode::FromBegin ) - fileOffset;
    ShaderCaches.Append( entry );

    FlushCacheInfoEntry( &entry );
}

#pragma optimize( "", on )

static bln TryToGetCachedCode( const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, shaderCode_t *po_code, ui32 checksum )
{
    if( !ShaderCachesFile.IsOpened() )
    {
        FileIO::fileError o_error;
        ShaderCachesFile.Open( L"Shaders\\" CACHEDIR "\\caches.scu", FileOpenMode::CreateIfDoesNotExist, FileProcMode::Write | FileProcMode::Read, FileCacheMode::LinearRead, &o_error );
        if( !ShaderCachesFile.IsOpened() )
        {
			char narrowPath[ MAX_PATH_LENGTH ];
			::wcstombs( narrowPath, L"Shaders\\" CACHEDIR "\\caches.scu", MAX_PATH_LENGTH );
            SENDLOG( CLogger::Tag::important, "#TryToGetCachedCode failed to open file %s\n", narrowPath );
            return false;
        }
        ShaderCachesFile.BufferSet( 2048 );
    }

    for( ui32 cache = 0; cache < ShaderCaches.Size(); ++cache )
    {
        if( sourceCodeLen == ShaderCaches[ cache ].len && checksum == ShaderCaches[ cache ].checksum )
        {
            ui32 totalCacheSize = ShaderCaches[ cache ].totalSize;
            UniquePtr< char, Heap::DefaultDeleter > fileBuf = Heap::Alloc<char>( totalCacheSize );
            ui32 readed = 0;
            ShaderCachesFile.OffsetSet( FileOffsetMode::FromBegin, ShaderCaches[ cache ].fileOffset, 0 );
            ShaderCachesFile.Read( fileBuf, totalCacheSize, &readed );
            if( readed != totalCacheSize )
            {
                SENDLOG( CLogger::Tag::important, "#TryToGetCachedCode requested read size %u did not match readed size %u\n", totalCacheSize, readed );
                continue;
            }
            byte *codeInCache;
            ui32 codeInCacheLen;
            if( !IsContentionMatch( sourceCode, sourceCodeLen, entryPoint, target, flags, macros, fileBuf, totalCacheSize, &codeInCache, &codeInCacheLen ) )
            {
                continue;
            }
            po_code->Assign( codeInCache, codeInCacheLen );
            return true;
        }
    }

    return false;
}

static bln TryToCompileShader( const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, shaderCode_t *po_code )
{
    SENDLOG( CLogger::Tag::important, "#TryToCompileShader compiling a new shader:\n%[*]s\n", sourceCodeLen, (char *)sourceCode );
    ID3DBlob *i_compiled, *i_errors;
    HRESULT hr = ::D3DCompile( sourceCode, sourceCodeLen, 0, macros, 0, entryPoint, target, flags, 0, &i_compiled, &i_errors );
    if( FAILED( hr ) )
    {
        ASSUME( i_compiled == 0 );
        if( i_errors )
        {
            SENDLOG( CLogger::Tag::important, "#TryToCompileShader failed to compile, errors %[*]s\n", i_errors->GetBufferSize(), (char *)i_errors->GetBufferPointer() );
            i_errors->Release();
        }
        else
        {
            SENDLOG( CLogger::Tag::important, "#TryToCompileShader failed to compile, error code %i\n", hr );
        }
        return false;
    }
    po_code->Assign( (byte *)i_compiled->GetBufferPointer(), i_compiled->GetBufferSize() );
    i_compiled->Release();
    return true;
}

bln GetCompiledCache( const byte *sourceCode, ui32 sourceCodeLen, const char *entryPoint, const char *target, UINT flags, D3D_SHADER_MACRO *macros, shaderCode_t *po_code )
{
    ui32 checksum;

    if( !is_DoNotUseCache )
    {
        checksum = GenCheckSum( sourceCode, sourceCodeLen, entryPoint, target, flags, macros );

        if( TryToGetCachedCode( sourceCode, sourceCodeLen, entryPoint, target, flags, macros, po_code, checksum ) )
        {
            return true;
        }
    }

    if( TryToCompileShader( sourceCode, sourceCodeLen, entryPoint, target, flags, macros, po_code ) )
    {
        if( ShaderCachesFile.IsOpened() )
        {
            ASSUME( !is_DoNotUseCache );
            AddCacheToFile( checksum, sourceCode, sourceCodeLen, entryPoint, target, flags, macros, po_code->Data(), po_code->Size() );
        }
        return true;
    }

    return false;
}