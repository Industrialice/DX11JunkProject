#include "PreHeader.hpp"
#include "EngineCommands.hpp"
#include "CommandsManager.hpp"
#include "Camera.hpp"
#include "Globals.hpp"
#include "RendererGlobals.hpp"
#include "SplashScreen.hpp"
#include "PostProcess.hpp"

#pragma optimize( "s", on )

static bln ExitCmd( const CArguments *args )
{
    SENDLOG( CLogger::Tag::info, "posting quit message\n" );
    ::PostQuitMessage( 0 );
    return true;
}

static bln EchoCmd( const CArguments *args )
{
    if( args )
    {
        CStr str;
        for( ui32 index = 0; index < args->Size(); ++index )
        {
            str += args->AgS( index );
            if( index + 1 < args->Size() )
            {
                str += " ";
            }
        }
        SENDLOG( CLogger::Tag::info, "%s\n", str.CStr() );
    }
    return true;
}

static bln PrintArgsCmd( const CArguments *args )
{
    if( !args )
    {
        SENDLOG( CLogger::Tag::info, "no arguments\n" );
        return true;
    }
    CStr str;
    for( ui32 index = 0; index < args->Size(); ++index )
    {
        char a_buf[ 2048 ];
        Funcs::PrintToStr( a_buf, 2047, "%s %i\n", args->AgS( index ), args->FmtGet( index ) );
        str += a_buf;
    }
    SENDLOG( CLogger::Tag::info, "arguments:\n%s\n", str.CStr() );
    return true;
}

static bln AgsParserTestCmd( const CArguments *args )
{
    if( !args )
    {
        ::MessageBoxA( 0, "no arguments", 0, 0 );
        return true;
    }
    char a_buf[ 2048 ];
    ui32 off = 0;
    for( ui32 index = 0; index < args->Size(); index += 2 )
    {
        Funcs::PrintToStr( a_buf + off, 2047, "%s %i %s\n", args->AgS( index ), args->FmtGet( index ), args->AgS( index + 1 ) );
        off += _StrLen( a_buf + off );
    }
    ::MessageBoxA( 0, a_buf, 0, 0 );
    return true;
}

static bln ShowMessageCmd( const CArguments *args )
{
    if( !args )
    {
        ::MessageBoxA( 0, 0, 0, 0 );
        return true;
    }
    ui32 len = 0;
    for( ui32 index = 0; index < args->Size(); ++index )
    {
        if( args->MaskS( index, 's' ) )
        {
            len += args->AgsLength( index ) + 1;
        }
    }
    char *p_buf = (char *)ALLOCA( len );
    len = 0;
    for( ui32 index = 0; index < args->Size(); ++index )
    {
        if( args->MaskS( index, 's' ) )
        {
            _MemCpy( p_buf + len, args->AgS( index ), args->AgsLength( index ) );
			len += args->AgsLength( index );
            p_buf[ len++ ] = ' ';
        }
    }
    p_buf[ len - 1 ] = '\0';
    ::MessageBoxA( 0, p_buf, "Message With Ags", 0 );
    return true;
}

static bln CamPosCmd( const CArguments *args )
{
    if( !args || !args->Size() )
    {
        SENDLOG( CLogger::Tag::info, "current camera position is %f %f %f\n", RendererGlobals::MainCamera.PositionGet().x, RendererGlobals::MainCamera.PositionGet().y, RendererGlobals::MainCamera.PositionGet().z );
        return true;
    }
    vec3 pos = RendererGlobals::MainCamera.PositionGet();
    for( ui32 index = 0; index < 3; ++index )
    {
        if( args->Size() <= index )
        {
            break;
        }
        if( !args->MaskS( index, 'n' ) )
        {
            SENDLOG( CLogger::Tag::error, "CamPosCmd you've passed not a number as arg\n" );
            return false;
        }
        pos.arr[ index ] = args->AgF( index );
    }
    RendererGlobals::MainCamera.PositionSet( pos );
    SENDLOG( CLogger::Tag::info, "new camera position is %f %f %f\n", pos.x, pos.y, pos.z );
    return true;
}

static bln CamSpeedCmd( const CArguments *ags )
{
    if( !ags || !ags->Size() )
    {
        SENDLOG( CLogger::Tag::info, "current camera speed is %f\n", RendererGlobals::MainCamera.SpeedGet() );
        return true;
    }
    if( !ags->MaskS( 0, 'n' ) )
    {
        SENDLOG( CLogger::Tag::error, "CamSpeedCmd you've passed not a number as arg\n" );
        return false;
    }
    if( ags->Size() > 1 )
    {
        SENDLOG( CLogger::Tag::error, "CamSpeedCmd ignoring arguments other then first\n" );
    }
    RendererGlobals::MainCamera.SpeedSet( ags->AgF( 0 ) );
    SENDLOG( CLogger::Tag::info, "new camera speed is %f\n", RendererGlobals::MainCamera.SpeedGet() );
    return true;
}

static bln CmdListCmd( const CArguments *ags )
{
    SENDLOG( CLogger::Tag::info, "commands count %u\n", CommandsManager::CommandsCount() );
    for( ui32 index = 0; index < CommandsManager::CommandsCount(); ++index )
    {
        const char *name = CommandsManager::CommandNameGet( index );
        const char *desc = CommandsManager::DescGet( name );
        if( !desc )
        {
            desc = "no desc";
        }
        SENDLOG( CLogger::Tag::info, "%u. %s [%u; %s]\n", index, name, CommandsManager::CategoryGet( name ), desc );
    }
    return true;
}

static bln CmdSplashScreen( const CArguments *ags )
{
	if( !ags || !ags->Size() )
	{
		SENDLOG( CLogger::Tag::info, "currenty splash screen is %s\n", SplashScreen::IsActiveGet() ? "active" : "inactive" );
		return true;
	}
	SplashScreen::IsActiveSet( ags->AgA( 0 ) );
	SENDLOG( CLogger::Tag::info, "currenty splash screen is now %s\n", SplashScreen::IsActiveGet() ? "active" : "inactive" );
	return true;
}

static bln CmdFilmGrain( const CArguments *ags )
{
	PostProcess::Effects::FilmGrain *fg = (PostProcess::Effects::FilmGrain *)PostProcess::LockEffectByID( Globals::FilmGrainId );

	if( !ags || !ags->Size() )
	{
		SENDLOG( CLogger::Tag::info, "film grain is %s with scale %f\n", fg->_is_enabled ? "active" : "inactive", fg->_noiseScale );
		PostProcess::Unlock();
		return true;
	}

	f32 scale = ags->AgF( 0 );
	fg->_noiseScale = scale;
	fg->_is_enabled = scale != 0.f;
	
	SENDLOG( CLogger::Tag::info, "film grain set to %s with scale %f\n", fg->_is_enabled ? "active" : "inactive", fg->_noiseScale );
	PostProcess::Unlock();
	return true;
}

static bln ArgsTestCmd( const CArguments *ags )
{
    CArguments args;
    args.AddAgsAL( "124.45 fp" );
    args.AddAgsAL( "124 dec" );
    args.AddAgsAL( "254e str" );
    args.AddAgsAL( "245.e str" );
    args.AddAgsAL( "2134234e5 fp" );
    args.AddAgsAL( "32523.e235 fp" );
    args.AddAgsAL( "23525e+ str" );
    args.AddAgsAL( "2342e+5 fp" );
    args.AddAgsAL( "213525e+15.5 str" );
    args.AddAgsAL( "3434.e-235 fp" );
    args.AddAgsAL( "23523.2135e-445 fp" );
    args.AddAgsAL( ".e+18 str" );
    args.AddAgsAL( "..e+18 str" );
    args.AddAgsAL( ".ee+18 str" );
    args.AddAgsAL( ".e++18 str" );
    args.AddAgsAL( "+.e+18 str" );
    args.AddAgsAL( "-.e+18 str" );
    args.AddAgsAL( ".e+-18 str" );
    args.AddAgsAL( ".4e+18 fp" );
    args.AddAgsAL( "+.4e+18 fp" );
    args.AddAgsAL( "-.4e+18 fp" );
    args.AddAgsAL( ".4ee+18 str" );
    args.AddAgsAL( "..4e+18 str" );
    args.AddAgsAL( ".4e+-18 str" );
    args.AddAgsAL( ".4e++18 str" );
    args.AddAgsAL( "0x45 hex" );
    args.AddAgsAL( "0x4fe hex" );
    args.AddAgsAL( "0x4aq str" );
    args.AddAgsAL( "0x str" );
    args.AddAgsAL( " str" );
    args.AddAgsAL( "0b00101 bin" );
    args.AddAgsAL( "0b02w str" );
    args.AddAgsAL( "0b str" );
    return CommandsManager::ExecuteCommand( "agsparsertest", &args );
}

void EngineCommands::Register()
{
    CommandsManager::AddCommand( "exit", ExitCmd, 0, "pussy out" );
    CommandsManager::AddCommand( "echo", EchoCmd, 0, "echo args" );
    CommandsManager::AddCommand( "printargs", PrintArgsCmd, 0, "print arguments line" );
    CommandsManager::AddCommand( "showmessage", ShowMessageCmd, 0, "show message" );
    CommandsManager::AddCommand( "campos", CamPosCmd, 0, "show or set camera position in meters" );
    CommandsManager::AddCommand( "camspeed", CamSpeedCmd, 0, "show or set camera speed in m/s" );
    CommandsManager::AddCommand( "cmdlist", CmdListCmd, 0, "shows commands list: index, name, category, descryption" );
    CommandsManager::AddCommand( "agsparsertest", AgsParserTestCmd, 0, "test command" );
    CommandsManager::AddCommand( "argstest", ArgsTestCmd, 0, "test command" );
    CommandsManager::AddCommand( "splash", CmdSplashScreen, 0, "activate or deactivate splash screen" );
    CommandsManager::AddCommand( "filmgrain", CmdFilmGrain, 0, "film grain scale" );
}