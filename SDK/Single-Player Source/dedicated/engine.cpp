//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#define snprintf _snprintf
#else
#include <memory.h>
#include <string.h>
#include <stdlib.h> // exit()
#endif

#include "dedicated.h"
#include "dll_state.h"
#include "enginecallback.h"
#include "sys_ded.h"

int			iWait = 0;
int		fDeferedPause = 0;

int			gDLLState;		
int			gDLLStateInfo;
long		ghMod = 0;

static engine_api_t nullapi;
engine_api_t engineapi = nullapi;

extern SleepType Sys_Sleep; // setup by sys_ded.cpp

typedef int (*engine_api_func)( int version, int size, struct engine_api_s *api );

#ifdef _WIN32
// This is a big chunk of uninitialized memory that we reserve for loading blobs into.
char g_rgchBlob[0x03800000];
#endif	// _WIN32

/*
==============
Eng_LoadFunctions

Load engine->front end interface, if possible
==============
*/
int Eng_LoadFunctions( long hMod )
{
	engine_api_func pfnEngineAPI;
	
	pfnEngineAPI = ( engine_api_func )Sys_GetProcAddress( hMod, "Sys_EngineAPI"  );
	if ( !pfnEngineAPI )
		return 0;

	if ( !(*pfnEngineAPI)( ENGINE_LAUNCHER_API_VERSION, sizeof( engine_api_t ), &engineapi ) )
		return 0;

	// All is okay
	return 1;
}

/*
==============
Eng_LoadStubs

Force NULL interface
==============
*/
void Eng_LoadStubs( void )
{
	// No callbacks in dedicated server since engine should always be loaded.
	memset( &engineapi, 0, sizeof( engineapi ) );

	engineapi.version		= ENGINE_LAUNCHER_API_VERSION;
	engineapi.rendertype	= RENDERTYPE_UNDEFINED;
	engineapi.size			= sizeof( engine_api_t );
}

/*
==============
Eng_Unload

Free engine .dll and reset interfaces
==============
*/
void Eng_Unload(void)
{
	if ( ghMod )
	{
		Sys_FreeLibrary(ghMod);
		ghMod = 0;
	}

	Eng_LoadStubs();

	gDLLState		= 0;
	gDLLStateInfo	= 0;
}

/*
==============
Eng_KillEngine

Load failure on engine
==============
*/
void Eng_KillEngine( long *phMod )
{
	Sys_FreeLibrary( ghMod );
	ghMod = *phMod = 0;

	Eng_LoadStubs();
}

/*
==============
Eng_Load

Try to load the engine with specified command line, etc. etc. and the specified .dll
==============
*/
int Eng_Load( const char *cmdline, struct exefuncs_s *pef, int memory, void *pmembase, const char *psz, int iSubMode )
{
	char	szLastDLL[ 100 ];
	long hMod = (long)NULL;

#if defined( _DEBUG )
	char *p;

	if ( psz && !stricmp( psz, "swds.dll" ) && CheckParm( "-force", &p ) && p )
	{
		psz = p;
	}
#endif

	// Are we loading a different engine?
	if ( psz && ghMod && !strcmp( psz, szLastDLL ) )
	{
		return 1;
	}

	if ( ghMod )
	{
		Eng_KillEngine( &hMod );
	}
	
	if ( !psz )
	{
		hMod = 0;
		Eng_LoadStubs();
	}
	else if ( !ghMod )
	{
		hMod = Sys_LoadLibrary( (char *)psz );
		if ( !hMod )
		{
			return 0;
		}

		// Load function table from engine
		if ( !Eng_LoadFunctions( hMod ) )
		{
			Sys_FreeLibrary( hMod );
			Eng_LoadStubs();
			return 0;
		}

		// Activate engine
		Eng_SetState( DLL_ACTIVE );
	}

	Eng_SetSubState( iSubMode );

	snprintf( szLastDLL, sizeof( szLastDLL ), "%s", psz );

	ghMod		= hMod;

	if ( ghMod )
	{
		static char *szEmpty = "";

		char *p = (char *)cmdline;
		if ( !p )
		{
			p = szEmpty;
		}

		if ( !engineapi.Game_Init( p, (unsigned char *)pmembase, memory, pef, NULL, 1) )
		{
			Sys_FreeLibrary(ghMod);
			ghMod = hMod = 0;
			return 0;
		}
		
		if ( engineapi.SetStartupMode )
		{
			engineapi.SetStartupMode( 1 );
		}
		
		if ( engineapi.Host_Frame )
		{
			Eng_Frame( 1, 0.05 );
		}

		if ( engineapi.SetStartupMode )
		{
			engineapi.SetStartupMode( 0 );
		}
	}
	return 1;
}

/*
==============
Eng_Frame

Run a frame in the engine, if it's loaded.
==============
*/
int Eng_Frame( int fForce, double time )
{
	if ( ( gDLLState != DLL_ACTIVE ) && !fForce )
		return 0;

	if ( gDLLState )
	{
		gDLLStateInfo = DLL_NORMAL;

		int iState = engineapi.Host_Frame ( (float)time, gDLLState, &gDLLStateInfo ); 

		// Special Signal
		if ( gDLLStateInfo != DLL_NORMAL )
		{
			switch (gDLLStateInfo)
			{
			case DLL_QUIT:
				Eng_Unload();
#ifdef _WIN32
				PostQuitMessage(0);
#else			
				exit( 0 );
#endif
				break;
			case DLL_RESTART:
				Eng_Unload();
#ifdef _WIN32
				PostQuitMessage(1);
#else			
				exit( 1 );
#endif
				break;
			default:
				break;
			}
		}

		// Are we in our transistion counter?
		if (iWait)
		{
			iWait--;

			// Defer all pauses until we're ready to bring up the launcher
			if (iState == DLL_PAUSED)
			{
				fDeferedPause = 1;
				Eng_SetState(DLL_ACTIVE);
				iState = DLL_ACTIVE;
			}

			// Are we done waiting, if so, did someone request a pause?
			if (!iWait && fDeferedPause)
			{
				//force a pause
				iState = DLL_PAUSED;
				gDLLState = DLL_ACTIVE; 
				fDeferedPause = 0;
			}
		}

		// Are we now in a transistion?
		if (iState == DLL_TRANS)
		{
			iState = DLL_ACTIVE;
			iWait = 5; // Let's wait N frames before we'll allow a pause
			Eng_SetState(DLL_ACTIVE);
		}

		// Has the state changed?
		if (iState != gDLLState)
		{
			Eng_SetState(iState);
		}
	}

	if ( gDLLState == DLL_CLOSE || gDLLState == DLL_RESTART)
	{
		static int bQuitting = 0;
		
		if ( !bQuitting )
		{
			bQuitting = 1;
			engineapi.Cbuf_AddText( "killserver\n" );
			Eng_Frame( 1, 0.05 );
			Sys_Sleep( 100 );
			Eng_Frame( 1, 0.05 );
			Sys_Sleep( 100 );
			return gDLLState;
		}
		
		Eng_Unload();

#ifdef _WIN32
		PostQuitMessage(0);
#else			
		if ( gDLLState == DLL_RESTART ) 
		{
			exit( 1 );
		}
		else
		{
			exit( 0 );
		}
#endif
	}

	return gDLLState;
}

/*
==============
Eng_SetSubState

==============
*/
void Eng_SetSubState(int iSubState)
{
	if ( !engineapi.GameSetSubState )
		return;

	if ( iSubState != ENG_NORMAL )
	{
		engineapi.GameSetSubState( iSubState );
	}
}

/*
==============
Eng_SetState

==============
*/
void Eng_SetState(int iState)
{
	gDLLState = iState;

	if (engineapi.GameSetState)
	{
		engineapi.GameSetState( iState );
	}
}
