#include "PreHeader.hpp"
#include "CommandsManager.hpp"
#include <CVector.hpp>
#include "Globals.hpp"

namespace
{
	struct SCommand
	{
    	CStr name;
		void *executeFunc;
		ui32 category;
		CStr desc;
		CVec < byte, void > param;
	};

	CVec < SCommand, void > o_Commands;
}

static ui32 CmdIndexGet( const char *cp_name );  //  ui32_max if not exist

NOINLINE void CommandsManager::AddCommand( const char *cp_name, cmdfunc func, ui32 category /* = 0 */, const char *cp_desc /* = 0 */ )
{
	ASSUME( cp_name && func && category != WrongCategory );
	if( IsCommandExist( cp_name ) )
	{
		SENDLOG( CLogger::Tag::error, "CommandsManager::AddCommand command %s already exists\n", cp_name );
		return;
	}
	ui32 nameLen = _StrLen( cp_name ) + 1;
	SCommand &o_cmd = *o_Commands.AppendNum();
	o_cmd.executeFunc = func;
	o_cmd.category = category;
    o_cmd.name = cp_name;
    o_cmd.desc = cp_desc;
}

NOINLINE void CommandsManager::AddComplexCommand( const char *cp_name, ui32 category, const char *cp_desc, const void *cp_param, uiw paramSize, executorfunc executor )
{
	ASSUME( cp_name && category != WrongCategory && cp_param && paramSize && executor );
	if( IsCommandExist( cp_name ) )
	{
		SENDLOG( CLogger::Tag::error, "CommandsManager::AddCommand command %s already exists\n", cp_name );
		return;
	}
	ui32 nameLen = _StrLen( cp_name ) + 1;
	o_Commands.Resize( o_Commands.Size() + 1 );
	SCommand &o_cmd = o_Commands.Back();
	o_cmd.executeFunc = executor;
	o_cmd.category = category;
    o_cmd.name = cp_name;
    o_cmd.desc = cp_desc;
    o_cmd.param.Resize( paramSize );
    _MemCpy( o_cmd.param.Data(), cp_param, paramSize );
}

NOINLINE void CommandsManager::RemoveCommand( const char *cp_name )
{
	ASSUME( cp_name );
	ui32 index = CmdIndexGet( cp_name );
	if( index == ui32_max )
	{
		SENDLOG( CLogger::Tag::error, "CommandsManager::RemoveCommand there is no command %s\n", cp_name );
		return;
	}
	o_Commands.Erase( index, 1 );
}

bln CommandsManager::IsCommandExist( const char *cp_name )
{
	ASSUME( cp_name );
	ui32 index = CmdIndexGet( cp_name );
	return index != ui32_max;
}

NOINLINE bln CommandsManager::ExecuteCommand( const char *cp_name, const CArguments *cpo_args )
{
	ASSUME( cp_name );
	ui32 index = CmdIndexGet( cp_name );
	if( index != ui32_max )
	{
		if( o_Commands[ index ].param.Size() )
		{
			return ((executorfunc)o_Commands[ index ].executeFunc)( o_Commands[ index ].param.Data(), o_Commands[ index ].param.Size(), cpo_args );
		}
		return ((cmdfunc)o_Commands[ index ].executeFunc)( cpo_args );
	}
	SENDLOG( CLogger::Tag::error, "CommandsManager::ExecuteCommand there is no command %s\n", cp_name );
	return false;
}

const char *CommandsManager::DescGet( const char *cp_name )
{
	ASSUME( cp_name );
	ui32 index = CmdIndexGet( cp_name );
	if( index != ui32_max )
	{
		return o_Commands[ index ].desc.CStr();
	}
	SENDLOG( CLogger::Tag::error, "CommandsManager::ExecuteCommand there is no command %s\n", cp_name );
	return 0;
}

void CommandsManager::DescSet( const char *cp_name, const char *cp_desc )
{
	ASSUME( cp_name );
    ui32 index = CmdIndexGet( cp_name );
    if( index == ui32_max )
    {
	    SENDLOG( CLogger::Tag::error, "CommandsManager::DescSet there is no command %s\n", cp_name );
        return;
    }
    o_Commands[ index ].desc = cp_desc;
}

ui32 CommandsManager::CategoryGet( const char *cp_name )
{
	ASSUME( cp_name );
	ui32 index = CmdIndexGet( cp_name );
	if( index != ui32_max )
	{
		return o_Commands[ index ].category;
	}
	SENDLOG( CLogger::Tag::error, "CommandsManager::CategoryGet there is no command %s\n", cp_name );
	return ui32_max;
}

void CommandsManager::CategorySet( const char *cp_name, ui32 category )
{
	ASSUME( cp_name && category != WrongCategory );
	ui32 index = CmdIndexGet( cp_name );
	if( index != ui32_max )
	{
		o_Commands[ index ].category = category;
	}
    else
    {
	    SENDLOG( CLogger::Tag::error, "CommandsManager::CategorySet there is no command %s\n", cp_name );
    }
}

ui32 CommandsManager::CommandsCount()
{
    return o_Commands.Size();
}

const char *CommandsManager::CommandNameGet( ui32 index )
{
    ASSUME( index < o_Commands.Size() );
    return o_Commands[ index ].name.CStr();
}

ui32 CmdIndexGet( const char *cp_name )
{
	for( ui32 index = 0; index < o_Commands.Size(); ++index )
	{
		if( _StrEqual( o_Commands[ index ].name.CStr(), cp_name ) )
		{
			return index;
		}
	}
	return ui32_max;
}
