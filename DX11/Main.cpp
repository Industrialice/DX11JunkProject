#include "PreHeader.hpp"
#include "Globals.hpp"
#include "ObjectsManager.hpp"
#include "ShadersManager.hpp"
#include <FileIO.hpp>
#include <Misc.hpp>
#include "Map.hpp"
#include "TextureLoader.hpp"
#include "Camera.hpp"
#include "PostProcess.hpp"
#include "Bloom.hpp"
#include "StatesManagers.hpp"
#include "CommandsManager.hpp"
#include "EngineCommands.hpp"
#include "IDManager.hpp"
#include "SplashScreen.hpp"
#include "The Test.hpp"
#include "Skybox.hpp"
#include "MouseKeyboard.hpp"
#include "hidusage.h"
#include "VKeys.hpp"
#include "RendererGlobals.hpp"
#include "HeapRef.hpp"
#include "HeapWin.hpp"
#include <xmmintrin.h>
#include <StdAbstractionLib.hpp>
#include <FileMemoryStream.hpp>

#define SPLASH_SCREEN

namespace
{
    DefaultFileType StatsFile;
    bln is_InConsoleMode;
    CVec < char > ConsoleBuf;
    CVec < CVec < char > > CommandsHistory;
    ui32 CommandsHistoryIndex;
    int CursorDeltaX, CursorDeltaY;
	const bln is_DrawOnBothMonitors = false;
	bln is_StopControlsInterception = true;
}

#define DELCONSOLETEXT( count ) for( ui32 index = 0; index < count; ++index ) Console::Write( "\b \b", 3, Console::color_t::black, Console::color_t::black )

static bln InitWindow();
static bln InitD3D();
static void OnResize();
static LRESULT WINAPI MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
static void LoopFunc();
static void FileLogFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len );
static void VSOutputLogFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len );
static void ConsoleOutputFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len );

#ifdef DEBUG
    #include <crtdbg.h>
#endif

#pragma optimize( "s", on )

static void LoadShaders();

static int MainFunc()
{
    #if defined(_M_AMD64) || _M_IX86_FP >= 2
        _MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
        _mm_setcsr( _mm_getcsr() | 0x8040 );
    #endif

    #if defined(DEBUG)
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_1024_DF );
    #endif

    StdAbstractionLib_Initialize();

    TimeMoment loadingTC = TimeMoment::CreateCurrent();

    HeapWin::Create( Globals::DHeap, 0, 1024 * 1024 * 1024, 0 );
    //HeapRef::Create( Globals::DHeap );

    Globals::DefaultLogger = CLogger::Create( "default_logger", true, true );

    Globals::DefaultLogger->DirectionAdd( FileLogFunc );
    Globals::DefaultLogger->DirectionAdd( VSOutputLogFunc );
    Globals::DefaultLogger->DirectionAdd( ConsoleOutputFunc );

	if( ::GetSystemMetrics( SM_CMONITORS ) > 1 )
	{
		if( !::AllocConsole() )
		{
			::FatalAppExitA( 1, "failed to alloc console" );
		}
		::MoveWindow( ::GetConsoleWindow(), ::GetSystemMetrics( SM_CXSCREEN ) + 250, 0, 1400, 900, TRUE );
	}

    Globals::Width = ::GetSystemMetrics( SM_CXSCREEN );
	if( is_DrawOnBothMonitors )
	{
		Globals::Width *= 2;
	}
    Globals::Height = ::GetSystemMetrics( SM_CYSCREEN );
    Globals::is_Windowed = true;
    Globals::is_UseWARP = false;

    ::srand( ::GetTickCount() );

    StatsFile.Open( L"Logs\\info.txt", FileOpenMode::CreateAlways, FileProcMode::Write, FileCacheMode::Default );
    if( !StatsFile.IsOpened() )
    {
        ::FatalAppExitA( 1, "can't create Logs\\info.txt" );
    }
    StatsFile.BufferSet( 16384 );

    SENDLOG( CLogger::Tag::important, "starting %u-bit " DBGCODE( "debug" ) RELCODE( "release" ) " build\n", WORD_SIZE );

#ifdef _M_IX86_FP
	#if _M_IX86_FP == 0
		SENDLOG( CLogger::Tag::important, "SSE is disabled\n" );
	#elif _M_IX86_FP == 1
		SENDLOG( CLogger::Tag::important, "SSE 1 is used\n" );
	#else
		SENDLOG( CLogger::Tag::important, "SSE 2 or higher is used\n" );
	#endif
#endif

    EngineCommands::Register();

    ::ShowCursor( false );

    POINT pt = { Globals::Width / 2, Globals::Height / 2 };
    ::ClientToScreen( Globals::h_Wnd, &pt );
    ::SetCursorPos( pt.x, pt.y );

    if( !InitWindow() )
    {
        ::MessageBoxA( 0, "InitWindow failed", 0, 0 );
        return 1;
    }

    RAWINPUTDEVICE o_hids[ 2 ];

    o_hids[ 0 ].usUsagePage = HID_USAGE_PAGE_GENERIC;
    o_hids[ 0 ].usUsage = HID_USAGE_GENERIC_MOUSE;
    o_hids[ 0 ].dwFlags = RIDEV_INPUTSINK;
    o_hids[ 0 ].hwndTarget = Globals::h_Wnd;

    o_hids[ 1 ].usUsagePage = HID_USAGE_PAGE_GENERIC;
    o_hids[ 1 ].usUsage = HID_USAGE_GENERIC_KEYBOARD;
    o_hids[ 1 ].dwFlags = RIDEV_INPUTSINK;
    o_hids[ 1 ].hwndTarget = Globals::h_Wnd;

    if( !::RegisterRawInputDevices( o_hids, COUNTOF(o_hids), sizeof(*o_hids) ) )
    {
        ::MessageBoxA( 0, "failed to register raw input devices", 0, 0 );
        return false;
    }

    if( !InitD3D() )
    {
        ::MessageBoxA( 0, "InitD3D failed", 0, 0 );
        return 1;
    }

    for( ui32 index = 0; index < 14; ++index )
    {
        D3D11_BUFFER_DESC o_bd;
        o_bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        o_bd.ByteWidth = D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16;
        o_bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        o_bd.MiscFlags = 0;
        o_bd.StructureByteStride = 0;
        o_bd.Usage = D3D11_USAGE_DYNAMIC;
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &o_bd, 0, RendererGlobals::ai_VSShaderRegisters + index ) );
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &o_bd, 0, RendererGlobals::ai_GSShaderRegisters + index ) );
        DXHRCHECK( RendererGlobals::i_Device->CreateBuffer( &o_bd, 0, RendererGlobals::ai_PSShaderRegisters + index ) );
    }
    RendererGlobals::i_ImContext->VSSetConstantBuffers( 0, 14, RendererGlobals::ai_VSShaderRegisters );
    RendererGlobals::i_ImContext->GSSetConstantBuffers( 0, 14, RendererGlobals::ai_GSShaderRegisters );
    RendererGlobals::i_ImContext->PSSetConstantBuffers( 0, 14, RendererGlobals::ai_PSShaderRegisters );

    D3D11_RASTERIZER_DESC o_rsDesc;
    o_rsDesc.AntialiasedLineEnable = false;
    o_rsDesc.CullMode = D3D11_CULL_BACK;
    o_rsDesc.DepthBias = 0;
    o_rsDesc.DepthBiasClamp = 0.f;
    o_rsDesc.DepthClipEnable = true;
    o_rsDesc.FillMode = D3D11_FILL_SOLID;
    o_rsDesc.FrontCounterClockwise = false;
    o_rsDesc.MultisampleEnable = false;
    o_rsDesc.ScissorEnable = false;
    o_rsDesc.SlopeScaledDepthBias = 0.f;

    RendererGlobals::i_RS = RasterizerStatesManager::GetState( &o_rsDesc );
    RendererGlobals::i_ImContext->RSSetState( RendererGlobals::i_RS );

    D3D11_BLEND_DESC o_blend = {};
    o_blend.AlphaToCoverageEnable = false;
    o_blend.IndependentBlendEnable = false;
    o_blend.RenderTarget[ 0 ].BlendEnable = false;
    o_blend.RenderTarget[ 0 ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    o_blend.RenderTarget[ 0 ].BlendOp = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].SrcBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].DestBlendAlpha = D3D11_BLEND_ZERO;
    o_blend.RenderTarget[ 0 ].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    o_blend.RenderTarget[ 0 ].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED | D3D11_COLOR_WRITE_ENABLE_GREEN | D3D11_COLOR_WRITE_ENABLE_BLUE;
    RendererGlobals::i_NoBlend = BlendStatesManager::GetState( &o_blend );

    const D3D11_DEPTH_STENCILOP_DESC o_defaultStencilOp = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
    D3D11_DEPTH_STENCIL_DESC o_dsd;
    o_dsd.BackFace = o_defaultStencilOp;
    o_dsd.DepthEnable = true;
    o_dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    o_dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    o_dsd.FrontFace = o_defaultStencilOp;
    o_dsd.StencilEnable = false;
    o_dsd.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    o_dsd.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    DXHRCHECK( RendererGlobals::i_Device->CreateDepthStencilState( &o_dsd, &RendererGlobals::i_DepthStencilDefault ) );

    RendererGlobals::MainCamera = CCamera( 0.075f, 5000.f, 75.f );
    RendererGlobals::MainCamera.PositionSet( vec3( 3, 4.5, -12 ) );
    RendererGlobals::MainCamera.SpeedSet( 2 );

    RendererGlobals::CurrentCamera = &RendererGlobals::MainCamera;

    TimeMoment LoadShadersCounter = TimeMoment::CreateCurrent();
    LoadShaders();
    Globals::TotalShaderLoadingTime = TimeMoment::CreateCurrent().SinceSec32( LoadShadersCounter );

    RendererGlobals::i_WhiteTex = TextureLoader::Load( "Textures/white.dds" );
    RendererGlobals::i_BlackTex = TextureLoader::Load( "Textures/black.dds" );

    CBloom::Private::Initialize();

    RendererGlobals::CurrentBloom = &RendererGlobals::MainBloom;
    //  TODO: da fuck
    RendererGlobals::MainBloom = std::move( CBloom( CBloom::BloomQuality::low, CBloom::BloomQuality::low, CBloom::BloomQuality::low, Globals::Width, Globals::Height, RendererGlobals::i_MainRenderTargetView, RendererGlobals::i_MainSRV, RendererGlobals::i_DepthStencilView ) );
    /*RendererGlobals::MainBloom.NearAmountSet( 0.5f );
    RendererGlobals::MainBloom.MediumAmountSet( 0.5f );
    RendererGlobals::MainBloom.WideAmountSet( 0.5f );*/

    PostProcess::Private::Initialize();
    //PostProcess::EffectPush( PostProcess::Effects::Gray, 0 );

    #ifdef SPLASH_SCREEN
        SplashScreen::Create( false );
    #endif

    m3x3 o_trans;
    LiceMath::M3x3RotateXYZ( &o_trans, 0, 0, 0.5 );
    //PostProcess::EffectPush( &PostProcess::Effects::InvertColors( ScreenRect( -0.5, 0.5, 0.5, -0.5 ), o_trans ) );
    //PostProcess::EffectPush( &PostProcess::Effects::GrayFactor( 16 ) );
    //PostProcess::EffectPush( &PostProcess::Effects::BlackWhite( (f96color)Color::Black, (f96color)Color::White, ScreenRect( -0.5, 0.5, 0.5, -0.5 ), o_trans ) );
    //PostProcess::EffectPush( &PostProcess::Effects::PrettyColors( f128color( 0.01, 0.01, 0.01, 0 ) ) );

    Globals::FilmGrainId = PostProcess::EffectPush( &PostProcess::Effects::FilmGrain( 0.003 ) );
	//Globals::FilmGrainId = PostProcess::EffectPush( &PostProcess::Effects::FilmGrainColored( 0.05 ) );

    Map::Create();

    Skybox::Create( "Textures\\skybox_test.dds" );

    TheTest::Create();

    SENDLOG( CLogger::Tag::important, "loading complete for %f, shader loading time %f texture loading time %f\n", TimeMoment::CreateCurrent().SinceSec32( loadingTC ), Globals::TotalShaderLoadingTime, Globals::TotalTextureLoadingTime );

    LoopFunc();

    FileIO::SStats o_stats;
    StatsFile.Flush();
    //StatsFile.StatsGet( &o_stats );
    //SENDLOG( CLogger::Tag::important, "logger file statistics:\nwritesToBufferCount %v\nwritesToFileCount %v\nreadsFromBufferCount %v\nreadsFromFileCount %v\nbytesFromBufferReaded %v\nbytesFromFileReaded %v\nbytesToBufferWritten %v\nbytesToFileWritten %v\nbufferedWrites %v\nunbufferedWrites %v\nbufferedReads %v\nunbufferedReads %v\n", o_stats.writesToBufferCount, o_stats.writesToFileCount, o_stats.readsFromBufferCount, o_stats.readsFromFileCount, o_stats.bytesFromBufferReaded, o_stats.bytesFromFileReaded, o_stats.bytesToBufferWritten, o_stats.bytesToFileWritten, o_stats.bufferedWrites, o_stats.unbufferedWrites, o_stats.bufferedReads, o_stats.unbufferedReads );

	return 0;
}

void LoadShaders()
{
#ifdef DEBUG
    ShadersManager::Private::Initialize( false );
#else
    ShadersManager::Private::Initialize( false );  //  shader cache is broken in release
#endif

    static const D3D11_SO_DECLARATION_ENTRY sca_matrixSODecl[] =
    {
        { 0, "LOCATION", 0, 0, 3, 0 },
        { 0, "AMPPARAMS", 0, 0, 2, 0 }
    };

    ShadersManager::CreateFromFiles( "light_l1", "Shaders/vs.hlsl", 0, "Shaders/ps_l1.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_l2", "Shaders/vs.hlsl", 0, "Shaders/ps_l2.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_l16", "Shaders/vs.hlsl", 0, "Shaders/ps_l16.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "lightless", "Shaders/lightless_vs.hlsl", 0, "Shaders/lightless_ps.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "lightless_glowing", "Shaders/lightless_vs.hlsl", 0, "Shaders/lightless_glowing_ps.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "lightless_texturless", "Shaders/lightless_texturless_vs.hlsl", 0, "Shaders/lightless_texturless_ps.hlsl", false, CVec < CStr >( { "POSITION0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "lightless_discard", "Shaders/lightless_vs.hlsl", 0, "Shaders/lightless_discard_ps.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0", "LOCATION0", "AMPPARAMS0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_instanced", "Shaders/vs_instanced.hlsl", 0, "Shaders/ps_instanced.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0", "W_ROW0", "W_ROW1", "W_ROW2", "W_ROW3", "WIT_COL0", "WIT_COL1", "WIT_COL2", "WIT_COL3" } ), 0, 0, 0, 0, 0 );
    const ui32 c_matrixStride = 20;
    ShadersManager::CreateFromFiles( "light_matrix_so", "Shaders/vs_matrix_so.hlsl", 0, 0, false, CVec < CStr >( { "LOCATION0", "AMPPARAMS0" } ), sca_matrixSODecl, COUNTOF( sca_matrixSODecl ), &c_matrixStride, 1, D3D11_SO_NO_RASTERIZED_STREAM );
    ShadersManager::CreateFromFiles( "light_matrix_so2", "Shaders/vs_matrix_so2.hlsl", 0, 0, false, CVec < CStr >( { "LOCATION0", "AMPPARAMS0" } ), sca_matrixSODecl, COUNTOF( sca_matrixSODecl ), &c_matrixStride, 1, D3D11_SO_NO_RASTERIZED_STREAM );
    ShadersManager::CreateFromFiles( "light_matrix_renderer", "Shaders/vs_matrix_renderer.hlsl", 0, "Shaders/ps_matrix_renderer.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0", "LOCATION0", "AMPPARAMS0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_matrix_renderer2", "Shaders/vs_matrix_renderer2.hlsl", 0, "Shaders/ps_matrix_renderer2.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "LOCATION0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_glowing", "Shaders/vs_light_glowing.hlsl", 0, "Shaders/ps_light_glowing.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0" } ), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "glowing", "Shaders/vs_glowing.hlsl", 0, "Shaders/ps_glowing.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0" } ), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "water", "Shaders/water_vs.hlsl", 0, "Shaders/water_ps.hlsl", false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "NORMAL0", "TANGENT0" } ), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "flatter_noise", "Shaders/flatter_noise_vs.hlsl", 0, "Shaders/flatter_noise_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_colorednoise", "Shaders/flatter_noise_vs.hlsl", 0, "Shaders/flatter_colorednoise_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_empty", "Shaders/flatter_empty_vs.hlsl", 0, "Shaders/flatter_empty_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_blackwhite", "Shaders/flatter_only_sample_vs.hlsl", 0, "Shaders/flatter_black_white_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_onlySample", "Shaders/flatter_only_sample_vs.hlsl", 0, "Shaders/flatter_texture_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_gray", "Shaders/flatter_gray_vs.hlsl", 0, "Shaders/flatter_gray_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_grayFactor", "Shaders/flatter_gray_factor_vs.hlsl", 0, "Shaders/flatter_gray_factor_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_color", "Shaders/flatter_color_vs.hlsl", 0, "Shaders/flatter_color_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_texture", "Shaders/flatter_texture_vs.hlsl", 0, "Shaders/flatter_texture_mult_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "flatter_bloom_final", "Shaders/flatter_bloom_final_vs.hlsl", 0, "Shaders/flatter_bloom_final_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_nothing", "Shaders/flatter_bloom_nothing_vs.hlsl", 0, "Shaders/flatter_bloom_nothing_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "flatter_bloom_near_horizontal_high", "Shaders/bloom_near/high_horizontal_vs.hlsl", 0, "Shaders/bloom_near/high_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_near_horizontal_medium", "Shaders/bloom_near/medium_horizontal_vs.hlsl", 0, "Shaders/bloom_near/medium_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_near_horizontal_low", "Shaders/bloom_near/low_horizontal_vs.hlsl", 0, "Shaders/bloom_near/low_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_near_vertical_high", "Shaders/bloom_near/high_vertical_vs.hlsl", 0, "Shaders/bloom_near/high_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_near_vertical_medium", "Shaders/bloom_near/medium_vertical_vs.hlsl", 0, "Shaders/bloom_near/medium_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_near_vertical_low", "Shaders/bloom_near/low_vertical_vs.hlsl", 0, "Shaders/bloom_near/low_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "flatter_bloom_medium_horizontal_medium", "Shaders/bloom_medium/medium_horizontal_vs.hlsl", 0, "Shaders/bloom_medium/medium_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_medium_horizontal_low", "Shaders/bloom_medium/low_horizontal_vs.hlsl", 0, "Shaders/bloom_medium/low_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_medium_vertical_medium", "Shaders/bloom_medium/medium_vertical_vs.hlsl", 0, "Shaders/bloom_medium/medium_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_medium_vertical_low", "Shaders/bloom_medium/low_vertical_vs.hlsl", 0, "Shaders/bloom_medium/low_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "flatter_bloom_wide_horizontal_low", "Shaders/bloom_wide/low_horizontal_vs.hlsl", 0, "Shaders/bloom_wide/low_horizontal_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "flatter_bloom_wide_vertical_low", "Shaders/bloom_wide/low_vertical_vs.hlsl", 0, "Shaders/bloom_wide/low_vertical_ps.hlsl", true, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    static const D3D11_SO_DECLARATION_ENTRY particle_stream_sodecl[] =
    {
        { 0, "POSITION", 0, 0, 3, 0 },
        { 0, "SPEEDSIZE", 0, 0, 4, 0 }
    };

    const UINT stride = sizeof(vec3) + sizeof(vec4);
    ShadersManager::CreateFromFiles( "particle_stream", "Shaders/particle_stream_vs.hlsl", 0, 0, false, CVec < CStr >( { "POSITION0", "SPEEDSIZE0" } ), particle_stream_sodecl, COUNTOF( particle_stream_sodecl ), &stride, 1, D3D11_SO_NO_RASTERIZED_STREAM );

    static const D3D11_SO_DECLARATION_ENTRY particle_stream_snow_sodecl[] =
    {
        { 0, "POSITION", 0, 0, 3, 0 }
    };

    const UINT pssstride = sizeof(vec3) + sizeof(vec4);
    ShadersManager::CreateFromFiles( "particle_stream_snow", "Shaders/particle_stream_snow_vs.hlsl", 0, 0, false, CVec < CStr >( { "POSITION0" } ), particle_stream_snow_sodecl, COUNTOF( particle_stream_snow_sodecl ), &pssstride, 1, D3D11_SO_NO_RASTERIZED_STREAM );

    static const D3D11_SO_DECLARATION_ENTRY particle_stream_fall_sodecl[] =
    {
        { 0, "POSITION", 0, 0, 3, 0 },
        { 0, "STARTPOSITION", 0, 0, 3, 0 },
        { 0, "LIFETIME", 0, 0, 2, 0 }
    };

    const UINT particle_stream_fall_stride = sizeof(vec3) + sizeof(vec3) + sizeof(vec2) + sizeof(vec4) + sizeof(vec4);
    ShadersManager::CreateFromFiles( "particle_stream_fall", "Shaders/particle_stream_fall_vs.hlsl", 0, 0, false, CVec < CStr >( { "POSITION0", "STARTPOSITION0", "LIFETIME0", "SPEEDSIZE0" } ), particle_stream_fall_sodecl, COUNTOF( particle_stream_fall_sodecl ), &particle_stream_fall_stride, 1, D3D11_SO_NO_RASTERIZED_STREAM );

    ShadersManager::CreateFromFiles( "particle", "Shaders/particle_vs.hlsl", 0/*"Shaders/particle_gs.hlsl"*/, "Shaders/particle_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    CVec < CStr > particlesInputDesc( { "POSITION0", "TEXCOORD0", "PPOSITION0", "PSPEEDSIZE0" } );

    ShadersManager::CreateFromFiles( "particle_instanced", "Shaders/particle_instanced_vs.hlsl", 0, "Shaders/particle_ps.hlsl", false, particlesInputDesc, 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "particle_instanced_glowing", "Shaders/particle_instanced_vs.hlsl", 0, "Shaders/particle_glowing_ps.hlsl", false, particlesInputDesc, 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "particle_instanced_glowing_discard", "Shaders/particle_instanced_vs.hlsl", 0, "Shaders/particle_glowing_discard_ps.hlsl", false, particlesInputDesc, 0, 0, 0, 0, 0 );
      
    ShadersManager::CreateFromFiles( "particle_instanced_glowing_discard_lifetime", "Shaders/particle_instanced_lifetime_vs.hlsl", 0, "Shaders/particle_glowing_discard_lifetime_ps.hlsl", false, 
        CVec < CStr >( { "POSITION0", "TEXCOORD0", "PPOSITION0", "LIFETIME0", "PSPEEDSIZE0", "PCOLOR0" } ), 0, 0, 0, 0, 0 );
      
    ShadersManager::CreateFromFiles( "particleTest", "Shaders/particleTest_vs.hlsl", 0, "Shaders/particleTest_ps.hlsl", false, 
        CVec < CStr >( { } ), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "particle_halo", "Shaders/particle_halo_vs.hlsl", 0, "Shaders/particle_halo_ps.hlsl", false, CCRefVec < CStr >(), 0, 0, 0, 0, 0 );

    CVec < CStr > particlesHaloInputDesc( { "POSITION0", "TEXCOORD0", "LOCATION0", "RADIUS0", "COLOR0" } );
    ShadersManager::CreateFromFiles( "particle_new_halo", "Shaders/particle_new_halo_vs.hlsl", 0, "Shaders/particle_new_halo_ps.hlsl", false, particlesHaloInputDesc, 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "particle_new_halo_intense", "Shaders/particle_new_halo_intense_vs.hlsl", 0, "Shaders/particle_new_halo_intense_ps.hlsl", false, particlesHaloInputDesc, 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "particle_new_ot_halo", "Shaders/particle_new_halo_ot_vs.hlsl", 0, 0, false, CVec < CStr >( { "POSITION0", "TEXCOORD0", "LOCATION0", "RADIUS0" } ), 0, 0, 0, 0, 0 );

    CVec < CStr > particlesBillboardInputDesc( { "POSITION0", "TEXCOORD0" } );
    ShadersManager::CreateFromFiles( "billboard", "Shaders/billboard_vs.hlsl", 0, "Shaders/particle_ps.hlsl", false, particlesBillboardInputDesc, 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "billboard_glowing", "Shaders/billboard_vs.hlsl", 0, "Shaders/particle_glowing_ps.hlsl", false, particlesBillboardInputDesc, 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "rect_shader", "Shaders/rect_vs.hlsl", 0, "Shaders/rect_ps.hlsl", true, CVec < CStr >( { "POSITION0", "TEXCOORD0", "TRANSFORM0", "LOCATION0" } ), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "circle_shader", "Shaders/circle_vs.hlsl", 0, "Shaders/circle_ps.hlsl", true, CVec < CStr >( { "POSITION0", "TEXCOORD0", "LOCATION0", "RADIUS" } ), 0, 0, 0, 0, 0 );

    ShadersManager::CreateFromFiles( "skybox", "Shaders/skybox_vs.hlsl", 0, "Shaders/skybox_ps.hlsl", false, CVec < CStr >( { "POSITION0" } ), 0, 0, 0, 0, 0 );

    CVec < CStr > lightSimpleInputDesc( { "POSITION0", "NORMAL0", "TEXCOORD0" } );
    ShadersManager::CreateFromFiles( "light_simple_l1", "Shaders/light_simple_vs.hlsl", 0, "Shaders/light_simple_l1_ps.hlsl", false, lightSimpleInputDesc, 0, 0, 0, 0, 0 );
    ShadersManager::CreateFromFiles( "light_simple_l2", "Shaders/light_simple_vs.hlsl", 0, "Shaders/light_simple_l2_ps.hlsl", false, lightSimpleInputDesc, 0, 0, 0, 0, 0 );
}

i32 WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, PSTR cmdLine, i32 showCmd )
{
    Globals::h_Inst = hInstance;
    return MainFunc();
}

#pragma optimize( "", on )

void LoopFunc()
{
    static MSG o_msg;
    static TimeMoment tc;

    tc = TimeMoment::CreateCurrent();

	do
	{
		if( ::PeekMessage( &o_msg, 0, 0, 0, PM_REMOVE ) )
		{
            ::TranslateMessage( &o_msg );
            ::DispatchMessageA( &o_msg );
		}
		/*else if( Globals::is_StopRendering || is_InConsoleMode )
        {
            ::Sleep( 1 );
            tc = TimeMoment::CreateCurrent();
        }*/
        else
        {
            RendererGlobals::i_ImContext->ClearRenderTargetView( RendererGlobals::i_MainRenderTargetView, Colora::Black.arr );
            RendererGlobals::i_ImContext->ClearDepthStencilView( RendererGlobals::i_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );

			TimeMoment currentMoment = TimeMoment::CreateCurrent();
            ui64 nsec = currentMoment.SinceUSec64( tc ) * 1000.0;
			tc = currentMoment;
            Globals::Time += nsec;
            Globals::DT = NSecTimeToF32( nsec );

            if( CursorDeltaX || CursorDeltaY )
            {
                RendererGlobals::MainCamera.RotationAroundX( CursorDeltaY * 10.333333333333333e-4f );
                RendererGlobals::MainCamera.RotationAroundY( CursorDeltaX * 10.333333333333333e-4f );
                CursorDeltaX = 0;
                CursorDeltaY = 0;

                POINT pt = { Globals::Width / 2, Globals::Height / 2 };
                ::ClientToScreen( Globals::h_Wnd, &pt );
                ::SetCursorPos( pt.x, pt.y );
            }

			if( MouseKeyboard::NumKeysDowned() )
			{
				f32 thruster = 1.f;
				if( MouseKeyboard::IsKeyDown( VKeys::Shift ) )
				{
					thruster *= 3.f;
				}
				if( MouseKeyboard::IsKeyDown( VKeys::Control ) )
				{
					thruster *= 0.33f;
				}

				thruster *= Globals::DT;

				if( MouseKeyboard::IsKeyDown( 'W' ) )
				{
					RendererGlobals::MainCamera.MoveZ( thruster );
				}

				if( MouseKeyboard::IsKeyDown( 'S' ) )
				{
					RendererGlobals::MainCamera.MoveZ( -thruster );
				}

				if( MouseKeyboard::IsKeyDown( 'A' ) )
				{
					RendererGlobals::MainCamera.MoveX( -thruster );
				}

				if( MouseKeyboard::IsKeyDown( 'D' ) )
				{
					RendererGlobals::MainCamera.MoveX( thruster );
				}

				if( MouseKeyboard::IsKeyDown( 'R' ) )
				{
					RendererGlobals::MainCamera.MoveY( thruster );
				}

				if( MouseKeyboard::IsKeyDown( 'F' ) )
				{
					RendererGlobals::MainCamera.MoveY( -thruster );
				}
			}

            RendererGlobals::MainCamera.Update();

            #ifdef SPLASH_SCREEN
                SplashScreen::Step();
            #endif

            Skybox::Update();

            Map::Update();

            TheTest::Update();

            ShadersManager::Private::BeginFrame();

            //Skybox::Draw();

            Map::Draw();

            TheTest::Draw();

            RendererGlobals::MainBloom.FlushGlowMap();

            //PostProcess::Private::Draw();

            RendererGlobals::i_SwapChain->Present( 1, 0 );

            ShadersManager::Private::EndFrame();
        }
    } while( o_msg.message != WM_QUIT );
}

bln InitWindow()
{
    WNDCLASSA wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = MsgProc; 
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = Globals::h_Inst;
    wc.hIcon         = ::LoadIconA( 0, IDI_APPLICATION );
    wc.hCursor       = ::LoadCursorA( 0, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)::GetStockObject( NULL_BRUSH );
	wc.lpszMenuName  = 0;
	wc.lpszClassName = "Crysis 4";

	if( !::RegisterClass(&wc) )
	{
		::MessageBoxA( 0, "RegisterClass Failed.", 0, 0 );
		return false;
	}

	DWORD style = Globals::is_Windowed ? WS_POPUP : WS_SYSMENU;

    RECT R = { 0, 0, Globals::Width, Globals::Height };
    ::AdjustWindowRect( &R, style, false );
	i32 width  = R.right - R.left;
	i32 height = R.bottom - R.top;

	Globals::h_Wnd = ::CreateWindowA( "Crysis 4", "Crysis 4", style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, Globals::h_Inst, 0 ); 
	if( !Globals::h_Wnd )
	{
		::MessageBoxA( 0, "CreateWindow Failed.", 0, 0 );
		return false;
	}

	::ShowWindow( Globals::h_Wnd, SW_SHOW );
	::UpdateWindow( Globals::h_Wnd );

	return true;
}

bln InitD3D()
{
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = Globals::Width;
	sd.BufferDesc.Height = Globals::Height;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	sd.BufferCount  = 2;
	sd.OutputWindow = Globals::h_Wnd;
	sd.Windowed     = Globals::is_Windowed;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

    #if defined(DEBUG) || defined(_DEBUG)  
        createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

	/*IDXGIFactory * pFactory = NULL;
    CreateDXGIFactory(__uuidof(IDXGIFactory) ,(void**)&pFactory);
    IDXGIAdapter *adapter;
    pFactory->EnumAdapters(0, &adapter);*/

    D3D_FEATURE_LEVEL a_levels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL featureLevel;
	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
			0,                 // default adapter
            Globals::is_UseWARP ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE,
			0,                 // no software device
			createDeviceFlags, 
            a_levels, COUNTOF( a_levels ),
			D3D11_SDK_VERSION,
            &sd,
            &RendererGlobals::i_SwapChain,
			&RendererGlobals::i_Device,
			&featureLevel,
			&RendererGlobals::i_ImContext );

	if( FAILED( hr ) )
	{
        char a_buf[ 1024 ];
        IntWithSize< sizeof(hr) * 8 >::uint_t hex;
        Funcs::BinAssign( &hex, hr );
        Funcs::IntToStrHex( true, true, false, a_buf, hex );
		::MessageBoxA( 0, a_buf, "D3D11CreateDevice Failed.", 0 );
		return false;
	}

	if( featureLevel == D3D_FEATURE_LEVEL_11_0 )
    {
        SENDLOG( CLogger::Tag::important, "feature level 11 has been used\n" );
    }
    else if( featureLevel == D3D_FEATURE_LEVEL_10_1 )
    {
        SENDLOG( CLogger::Tag::important, "feature level 10_1 has been used\n" );
    }
    else if( featureLevel == D3D_FEATURE_LEVEL_10_0 )
    {
        SENDLOG( CLogger::Tag::important, "feature level 10 has been used\n" );
    }
    else
    {
        SENDLOG( CLogger::Tag::important, "feature level unknown has been used\n" );
    }

    /*UINT quality;
    DXHRCHECK( RendererGlobals::i_Device->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality ) );

    SENDLOG( CLogger::Tag::important, "using multisampling %ux%u",, 4, quality );*/

#if 0
    UINT numLevels;
    DXHRCHECK( i_Device->CheckMultisampleQualityLevels( DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numLevels ) );

	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width  = Width;
	sd.BufferDesc.Height = Height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count   = 1;
    sd.SampleDesc.Quality = 0;
	sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount  = 1;
	sd.OutputWindow = h_Wnd;
	sd.Windowed     = is_Windowed;
	sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags        = 0;

	// To correctly create the swap chain, we must use the IDXGIFactory that was
	// used to create the device.  If we tried to use a different IDXGIFactory instance
	// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
	// This function is being called with a device from a different IDXGIFactory."

	IDXGIDevice *dxgiDevice;
    DXHRCHECK( i_Device->QueryInterface( __uuidof(IDXGIDevice), (void**)&dxgiDevice ) );
	  
	IDXGIAdapter *dxgiAdapter;
    DXHRCHECK( dxgiDevice->GetParent( __uuidof(IDXGIAdapter), (void**)&dxgiAdapter ) );

	IDXGIFactory *dxgiFactory;
    DXHRCHECK( dxgiAdapter->GetParent( __uuidof(IDXGIFactory), (void**)&dxgiFactory ) );

    DXHRCHECK( dxgiFactory->CreateSwapChain( i_Device, &sd, &i_SwapChain ) );

    //  needs to be called after IDXGIFactory::CreateSwapChain
    //DXHRCHECK( dxgiFactory->MakeWindowAssociation( h_Wnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN ) );

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();
#endif

	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the OnResize method here to avoid code duplication.

	OnResize();

    return true;
}

void OnResize()
{
    if( RendererGlobals::i_SwapChain == 0 )
    {
        return;
    }

	if( RendererGlobals::i_MainRenderTargetView )
    {
        RendererGlobals::i_MainRenderTargetView->Release();
    }
	if( RendererGlobals::i_DepthStencilView )
    {
        RendererGlobals::i_DepthStencilView->Release();
    }
	if( RendererGlobals::i_DepthStencilBuffer )
    {
        RendererGlobals::i_DepthStencilBuffer->Release();
    }

	// Resize the swap chain and recreate the render target view.

    DXHRCHECK( RendererGlobals::i_SwapChain->ResizeBuffers( 0, Globals::Width, Globals::Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0 ) );
	ID3D11Texture2D *i_backBuffer;
	DXHRCHECK( RendererGlobals::i_SwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&i_backBuffer) ) );
	DXHRCHECK( RendererGlobals::i_Device->CreateRenderTargetView( i_backBuffer, 0, &RendererGlobals::i_MainRenderTargetView ) );

    D3D11_SHADER_RESOURCE_VIEW_DESC o_rvd;
    o_rvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    o_rvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    o_rvd.Texture1D.MipLevels = 1;
    o_rvd.Texture1D.MostDetailedMip = 0;
    DXHRCHECK( RendererGlobals::i_Device->CreateShaderResourceView( i_backBuffer, &o_rvd, &RendererGlobals::i_MainSRV ) );

    i_backBuffer->Release();

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width     = Globals::Width;
	depthStencilDesc.Height    = Globals::Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format    = DXGI_FORMAT_D32_FLOAT;
    depthStencilDesc.SampleDesc.Count   = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags      = 0;

	DXHRCHECK( RendererGlobals::i_Device->CreateTexture2D( &depthStencilDesc, 0, &RendererGlobals::i_DepthStencilBuffer ) );
	DXHRCHECK( RendererGlobals::i_Device->CreateDepthStencilView( RendererGlobals::i_DepthStencilBuffer, 0, &RendererGlobals::i_DepthStencilView ) );

	// Bind the render target view and depth/stencil view to the pipeline.

	RendererGlobals::i_ImContext->OMSetRenderTargets( 1, &RendererGlobals::i_MainRenderTargetView, RendererGlobals::i_DepthStencilView );

	// Set the viewport transform.

	RendererGlobals::o_ScreenViewport.TopLeftX = 0.f;
	RendererGlobals::o_ScreenViewport.TopLeftY = 0.f;
	RendererGlobals::o_ScreenViewport.Width    = (f32)Globals::Width;
	RendererGlobals::o_ScreenViewport.Height   = (f32)Globals::Height;
	RendererGlobals::o_ScreenViewport.MinDepth = 0.f;
	RendererGlobals::o_ScreenViewport.MaxDepth = 1.f;

    RendererGlobals::SetViewports( 1, &RendererGlobals::o_ScreenViewport );
}

static void ToggleRendering()
{
    /*if(Globals:: is_StopRendering )
    {
        POINT pt = { Globals::Width / 2, Globals::Height / 2 };
        ::ClientToScreen( Globals::h_Wnd, &pt );
        ::SetCursorPos( pt.x, pt.y );
    }
    Globals::is_StopRendering = !Globals::is_StopRendering;*/
}

LRESULT WINAPI MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    /*if( msg == WM_NCHITTEST || msg == WM_SETCURSOR )
    {
        return 0;
    }*/

	if( !is_StopControlsInterception )
	{
		switch( msg )
		{
		case WM_INPUT:
			{
				RAWINPUT o_data;
				UINT dataSize = sizeof(o_data);
				if( ::GetRawInputData( (HRAWINPUT)lParam, RID_INPUT, &o_data, &dataSize, sizeof(RAWINPUTHEADER) ) == -1 )
				{
					return 0;
				}
				ASSUME( dataSize == sizeof(o_data) );
				if( o_data.header.dwType == RIM_TYPEMOUSE )
				{
					if( o_data.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE )
					{
						HARDBREAK;
					}
					if( o_data.data.mouse.usFlags & MOUSE_MOVE_NOCOALESCE )
					{
						HARDBREAK;
					}
					//ASSUME( o_data.data.mouse.usFlags & MOUSE_MOVE_RELATIVE );

					CursorDeltaX += o_data.data.mouse.lLastX;
					CursorDeltaY += o_data.data.mouse.lLastY;

					if( o_data.data.mouse.lLastX || o_data.data.mouse.lLastY )
					{
						MouseKeyboard::MouseMove( o_data.data.mouse.lLastX, o_data.data.mouse.lLastY );
					}

					if( o_data.data.mouse.usButtonFlags )
					{
						static const ui32 sca_mouseKeysDowns[ 5 ] = { RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_5_DOWN };
						static const ui32 sca_mouseKeysUps[ 5 ] = { RI_MOUSE_BUTTON_1_UP, RI_MOUSE_BUTTON_2_UP, RI_MOUSE_BUTTON_3_UP, RI_MOUSE_BUTTON_4_UP, RI_MOUSE_BUTTON_5_UP };
						STATIC_CHECK( COUNTOF( sca_mouseKeysDowns ) == COUNTOF( sca_mouseKeysUps ), "mouse keys arrays missmatch" );
						for( ui32 key = 0; key < COUNTOF( sca_mouseKeysDowns ); ++key )
						{
							if( o_data.data.mouse.usButtonFlags & sca_mouseKeysDowns[ key ] )
							{
								MouseKeyboard::KeyDown( VKeys::MButton0 + key );
							}
							else if( o_data.data.mouse.usButtonFlags & sca_mouseKeysUps[ key ] )
							{
								MouseKeyboard::KeyUp( VKeys::MButton0 + key );
							}
						}

						if( o_data.data.mouse.usButtonFlags & RI_MOUSE_WHEEL )
						{
							MouseKeyboard::MouseWheelMove( o_data.data.mouse.usButtonData / WHEEL_DELTA );
						}
					}
				}
				else if( o_data.header.dwType == RIM_TYPEKEYBOARD )
				{
					ui32 addKey = ui32_max;
					bln is_e1 = o_data.data.keyboard.Flags & RI_KEY_E1 != 0;
					bln is_e0 = o_data.data.keyboard.Flags & RI_KEY_E0 != 0;

					if( o_data.data.keyboard.VKey == VK_SHIFT )
					{
						addKey = (o_data.data.keyboard.MakeCode == 0x2A) ? VK_LSHIFT : VK_RSHIFT;
					}
					else if( o_data.data.keyboard.VKey == VK_CONTROL )
					{
						addKey = is_e0 ? VK_RCONTROL : VK_LCONTROL;  //  TODO: fix
					}
					else if( o_data.data.keyboard.VKey == VK_MENU )
					{
						addKey = is_e0 ? VK_RMENU : VK_LMENU;  //  TODO: fix
					}
					else if( o_data.data.keyboard.VKey == VK_RETURN )
					{
						//flags |= is_e0 ? KEY_FLAG_RIGHT : KEY_FLAG_LEFT;  //  TODO: fix
					}

					ui32 mainKey = VKeys::GetPlatformMappingStruct()[ o_data.data.keyboard.VKey ];
					if( addKey != ui32_max )
					{
						addKey = VKeys::GetPlatformMappingStruct()[ addKey ];
					}

					if( o_data.data.keyboard.Flags & RI_KEY_BREAK )
					{
						MouseKeyboard::KeyUp( mainKey );
						if( addKey != ui32_max )
						{
							MouseKeyboard::KeyUp( addKey );
						}
					}
					else
					{
						MouseKeyboard::KeyDown( mainKey );
						if( addKey != ui32_max )
						{
							MouseKeyboard::KeyDown( addKey );
						}
					}
				}
			}
			return 0;

		case WM_SETCURSOR:
		case WM_MOUSEMOVE:
			return 0;

		case WM_KEYDOWN:
			if( is_InConsoleMode )
			{
				if( wParam == VK_TAB )
				{
					::MessageBeep( 0xFFFFFFFF );
					is_InConsoleMode = false;
					return 0;
				}
				if( wParam == VK_RETURN )
				{
					if( !ConsoleBuf.Size() )
					{
						return 0;
					}
					char newLine = '\n';
					Console::Write( &newLine, 1, Console::color_t::black, Console::color_t::black );
					ConsoleBuf.Append( '\0' );
					CArguments args;
					args.AddAgsAL( &ConsoleBuf[ 0 ] );
					ui32 cmdNameLen = args.AgsLength( 0 );
					CStr cmdName( args.AgS( 0 ), cmdNameLen );
					args.Remove( 0 );
					SENDLOG( CLogger::Tag::info, "executing command %s\n", cmdName.CStr() );
					CommandsManager::ExecuteCommand( cmdName.CStr(), &args );
					CommandsHistory.Append( ConsoleBuf );
					if( CommandsHistory.Size() > 256 )
					{
						CommandsHistory.Erase( CommandsHistory.begin() );
					}
					CommandsHistoryIndex = CommandsHistory.Size();
					ConsoleBuf.Clear();
					return 0;
				}
				if( wParam == VK_BACK )
				{
					DELCONSOLETEXT( 1 );
					ConsoleBuf.PopBackSafe();
					return 0;
				}
				if( wParam == VK_UP )
				{
					if( CommandsHistoryIndex )
					{
						DELCONSOLETEXT( ConsoleBuf.Size() );
						--CommandsHistoryIndex;
						ConsoleBuf = CommandsHistory[ CommandsHistoryIndex ];
						ConsoleBuf.PopBack();
						Console::Write( &ConsoleBuf[ 0 ], ConsoleBuf.Size(), Console::color_t::green, Console::color_t::black );
					}
					return 0;
				}
				if( wParam == VK_DOWN )
				{
					if( CommandsHistoryIndex < CommandsHistory.Size() )
					{
						++CommandsHistoryIndex;
						if( CommandsHistoryIndex < CommandsHistory.Size() )
						{
							DELCONSOLETEXT( ConsoleBuf.Size() );
							ConsoleBuf = CommandsHistory[ CommandsHistoryIndex ];
							ConsoleBuf.PopBack();
							Console::Write( &ConsoleBuf[ 0 ], ConsoleBuf.Size(), Console::color_t::green, Console::color_t::black );
						}
						else
						{
							DELCONSOLETEXT( ConsoleBuf.Size() );
							ConsoleBuf.Clear();
						}
					}
					return 0;
				}
				return 0;
			}

			if( MouseKeyboard::IsKeyDown( VKeys::Control ) || MouseKeyboard::IsKeyDown( VKeys::Shift ) || MouseKeyboard::IsKeyDown( VKeys::Alt ) )
			{
				break;
			}

			switch( wParam )
			{
			case 'B':
				RendererGlobals::MainBloom.IsEnabledSet( !RendererGlobals::MainBloom.IsEnabledGet() );
				break;
			case 'N':
				RendererGlobals::MainBloom.IsTargetEnabledSet( !RendererGlobals::MainBloom.IsTargetEnabledGet() );
				break;
			case 'M':
				RendererGlobals::MainBloom.IsVerticalEnabledSet( !RendererGlobals::MainBloom.IsVerticalEnabledGet() );
				break;
			case '1':
				RendererGlobals::MainBloom.NearQualitySet( RendererGlobals::MainBloom.NearQualityGet() + 1 > CBloom::BloomQuality::high ? CBloom::BloomQuality::disabled : RendererGlobals::MainBloom.NearQualityGet() + 1 );
				break;
			case '2':
				RendererGlobals::MainBloom.MediumQualitySet( RendererGlobals::MainBloom.MediumQualityGet() + 1 > CBloom::BloomQuality::medium ? CBloom::BloomQuality::disabled : RendererGlobals::MainBloom.MediumQualityGet() + 1 );
				break;
			case '3':
				RendererGlobals::MainBloom.WideQualitySet( RendererGlobals::MainBloom.WideQualityGet() + 1 > CBloom::BloomQuality::low ? CBloom::BloomQuality::disabled : RendererGlobals::MainBloom.WideQualityGet() + 1 );
				break;
			case 'P':
				Globals::is_DrawDebugShapes = !Globals::is_DrawDebugShapes;
				break;
			case VK_SPACE:
				TheTest::ReloadParticleTests();
				break;
			case VK_RETURN:
				CommandsManager::ExecuteCommand( "exit", 0 );
				break;
			case VK_UP:
				RendererGlobals::MainBloom.NearAmountSet( RendererGlobals::MainBloom.NearAmountGet() + 0.01f );
				RendererGlobals::MainBloom.MediumAmountSet( RendererGlobals::MainBloom.MediumAmountGet() + 0.01f );
				RendererGlobals::MainBloom.WideAmountSet( RendererGlobals::MainBloom.WideAmountGet() + 0.01f );
				break;
			case VK_DOWN:
				RendererGlobals::MainBloom.NearAmountSet( RendererGlobals::MainBloom.NearAmountGet() - 0.01f );
				RendererGlobals::MainBloom.MediumAmountSet( RendererGlobals::MainBloom.MediumAmountGet() - 0.01f );
				RendererGlobals::MainBloom.WideAmountSet( RendererGlobals::MainBloom.WideAmountGet() - 0.01f );
				break;
			case VK_TAB:
				::MessageBeep( 0xFFFFFFFF );
				is_InConsoleMode = true;
				break;
			}
			break;

		case WM_CHAR:
			if( is_InConsoleMode )
			{
				if( wParam == VK_RETURN || wParam == VK_TAB || wParam == VK_BACK )
				{
					return 0;
				}
				Console::Write( (char *)&wParam, 1, Console::color_t::green, Console::color_t::black );
				ConsoleBuf.Append( wParam );
				return 0;
			}
			break;
		}
	}

	switch( msg )
	{
	// WM_ACTIVATE is sent when the window is activated or deactivated.  
	// We pause the game when the window is deactivated and unpause it 
	// when it becomes active.  
	case WM_ACTIVATE:
        //ToggleRendering();
		is_StopControlsInterception = !is_StopControlsInterception;
		MouseKeyboard::ResetKeyDowns();
		return 0;

	// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
        Globals::Width = ::GetSystemMetrics( SM_CXSCREEN );
		if( is_DrawOnBothMonitors )
		{
			Globals::Width *= 2;
		}
        Globals::Height = ::GetSystemMetrics( SM_CYSCREEN );
        OnResize();
		return 0;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		return 0;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		return 0;
 
	// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	// The WM_MENUCHAR message is sent when a menu is active and the user presses 
	// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

	// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200; 
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		return 0;
	}

	return DefWindowProcA( hwnd, msg, wParam, lParam );
}

void FileLogFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len )
{
    ASSUME( tag < 7 );
    static const char *const scapc_tags[ 7 ] = { "inf", "wrn", "err", "dbg", "usr", "imp", "crl" };
    const char *cp_tag = scapc_tags[ tag ];
    char a_tag[ 7 ];
    Funcs::PrintToStr( a_tag, 7, "[%s] ", cp_tag );
    StatsFile.Write( a_tag, 6 );
    StatsFile.Write( cp_text, len );
}

void VSOutputLogFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len )
{
    ASSUME( tag < 7 );
    static const char *const scapc_tags[ 7 ] = { "inf", "wrn", "err", "dbg", "usr", "imp", "crl" };
    const char *cp_tag = scapc_tags[ tag ];
    CVec < char, void, 2048, Sem_Strict, Allocator::Simple > tempStr( 0, len + 7 );
    Funcs::PrintToStr( tempStr.Data(), 7, "[%s] ", cp_tag );
    _MemCpy( tempStr.Data() + 6, cp_text, len );
    tempStr.Data()[ len + 6 ] = '\0';
    OutputDebugStringA( tempStr.Data() );
}

void ConsoleOutputFunc( CLogger::Tag::messageTag_t tag, const char *cp_text, uiw len )
{
    ASSUME( tag < 7 );
    static const char *const scapc_tags[ 7 ] = { "inf", "wrn", "err", "dbg", "usr", "imp", "crl" };
    const char *cp_tag = scapc_tags[ tag ];
    CVec < char, void, 2048, Sem_Strict, Allocator::Simple > tempStr( 0, len + 7 );
    char *str = tempStr.Data();
    Funcs::PrintToStr( str, 7, "[%s] ", cp_tag );
    _MemCpy( str + 6, cp_text, len );
    static const Console::color_t colors[ 6 ] = { Console::color_t::white, Console::color_t::yellow, Console::color_t::red, Console::color_t::magenta, Console::color_t::green, Console::color_t::cyan };
    Console::Write( str, len + 6, colors[ tag ], Console::color_t::black );
}