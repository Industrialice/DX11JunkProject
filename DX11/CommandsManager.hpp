#ifndef __COMMANDS_MANAGER_HPP__
#define __COMMANDS_MANAGER_HPP__

#include "CArguments.hpp"

namespace CommandsManager
{
    const ui32 WrongCategory = ui32_max;

	typedef bln (*cmdfunc)( const CArguments *cpo_args );
	typedef bln (*executorfunc)( const void *cp_param, uiw paramSize, const CArguments *cpo_args );

	void AddCommand( const char *cp_name, cmdfunc func, ui32 category = 0, const char *cp_desc = 0 );  //  WrongCategory is unacceptable category
	void AddComplexCommand( const char *cp_name, ui32 category, const char *cp_desc, const void *cp_param, uiw paramSize, executorfunc executor );  //  WrongCategory is unacceptable category
	void RemoveCommand( const char *cp_name );
	bln IsCommandExist( const char *cp_name );
	bln ExecuteCommand( const char *cp_name, const CArguments *cpo_args );
	const char *DescGet( const char *cp_name );  //  will return null if commands does not exist
	void DescSet( const char *cp_name, const char *cp_desc );
	ui32 CategoryGet( const char *cp_name );  //  will return WrongCategory if commands does not exist
	void CategorySet( const char *cp_name, ui32 category );  //  WrongCategory is unacceptable category
    ui32 CommandsCount();
    const char *CommandNameGet( ui32 index );
}

#endif __COMMANDS_MANAGER_HPP__
