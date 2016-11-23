//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <string.h>
#include <stdlib.h>
#include "interface.h"

#ifndef _WIN32  // LINUX
#include <dlfcn.h>
#include <unistd.h> // getcwd
#include <stdio.h> // sprintf
#endif


// ------------------------------------------------------------------------------------ //
// InterfaceReg.
// ------------------------------------------------------------------------------------ //
InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;


InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) :
	m_pName(pName)
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}



// ------------------------------------------------------------------------------------ //
// CreateInterface.
// ------------------------------------------------------------------------------------ //
EXPORT_FUNCTION IBaseInterface *CreateInterface( const char *pName, int *pReturnCode )
{
	InterfaceReg *pCur;
	
	for(pCur=InterfaceReg::s_pInterfaceRegs; pCur; pCur=pCur->m_pNext)
	{
		if(strcmp(pCur->m_pName, pName) == 0)
		{
			if ( pReturnCode )
			{
				*pReturnCode = IFACE_OK;
			}
			return pCur->m_CreateFn();
		}
	}
	
	if ( pReturnCode )
	{
		*pReturnCode = IFACE_FAILED;
	}
	return NULL;	
}


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif


#ifdef _WIN32
HINTERFACEMODULE Sys_LoadModule(const char *pModuleName)
{
	return (HINTERFACEMODULE)LoadLibrary(pModuleName);
}

#else  // LINUX
HINTERFACEMODULE Sys_LoadModule(const char *pModuleName)
{
	// Linux dlopen() doesn't look in the current directory for libraries.
	// We tell it to, so people don't have to 'install' libraries as root.

	char szCwd[1024];
	char szAbsoluteLibFilename[1024];

	getcwd( szCwd, sizeof( szCwd ) );
	if ( szCwd[ strlen( szCwd ) - 1 ] == '/' )
		szCwd[ strlen( szCwd ) - 1 ] = 0;

	sprintf( szAbsoluteLibFilename, "%s/%s", szCwd, pModuleName );

	return (HINTERFACEMODULE)dlopen( szAbsoluteLibFilename, RTLD_NOW );
}

#endif


#ifdef _WIN32
void Sys_FreeModule(HINTERFACEMODULE hModule)
{
	if(!hModule)
		return;

	FreeLibrary((HMODULE)hModule);
}

#else  // LINUX
void Sys_FreeModule(HINTERFACEMODULE hModule)
{
	if(!hModule)
		return;

	dlclose( (void *)hModule );
}

#endif


//-----------------------------------------------------------------------------
// Purpose: returns the instance of this module
// Output : interface_instance_t
//-----------------------------------------------------------------------------
CreateInterfaceFn Sys_GetFactoryThis( void )
{
	return CreateInterface;
}


//-----------------------------------------------------------------------------
// Purpose: returns the instance of the named module
// Input  : *pModuleName - name of the module
// Output : interface_instance_t - instance of that module
//-----------------------------------------------------------------------------

#ifdef _WIN32
CreateInterfaceFn Sys_GetFactory( HINTERFACEMODULE hModule )
{
	if(!hModule)
		return NULL;

	return (CreateInterfaceFn)GetProcAddress((HMODULE)hModule, CREATEINTERFACE_PROCNAME);
}

#else  // LINUX
CreateInterfaceFn Sys_GetFactory( HINTERFACEMODULE hModule )
{
	if(!hModule)
		return NULL;

	return (CreateInterfaceFn)dlsym( (void *)hModule, CREATEINTERFACE_PROCNAME );
}

#endif
