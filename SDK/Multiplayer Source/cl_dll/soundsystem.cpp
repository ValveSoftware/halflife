//======== (C) Copyright 1996-2002 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include <windows.h>
#include <dsound.h>
#include <mmsystem.h>
#include "r_studioint.h"

extern engine_studio_api_t IEngineStudio;

#define RENDERTYPE_UNDEFINED	0
#define RENDERTYPE_SOFTWARE		1
#define RENDERTYPE_HARDWARE		2

#define ENGINE_LAUNCHER_API_VERSION 1

LPDIRECTSOUND       lpDS		= NULL;
LPDIRECTSOUNDBUFFER lpDSBuf		= NULL;
LPHWAVEOUT			lpHW		= NULL;

static HMODULE hEngine = 0;

typedef struct engine_api_s
{
	int		version;
	int		rendertype;
	int		size;

	// Functions
	void	( *unused1 )					( void );
	void	( *unused2 )					( void );
	void	( *unused3 )					( void );
	void	( *unused4 )					( void );
	void	( *unused5 )					( void );
	void	( *unused6 )					( void );
	void	( *unused7 )					( void );
	void	( *unused8 )					( void );
	void	( *unused9 )					( void );
	void	( *unused10 )					( void );
	void	( *unused11 )					( void );
	void	( *unused12 )					( void );
	void	( *unused13 )					( void );
	void	( *unused14 )					( void );
	void	( *unused15 )					( void );
	void	( *unused16 )					( void );
	void	( *unused17 )					( void );
	void	( *unused18 )					( void );
	void	( *unused19 )					( void );
	void	( *unused20 )					( void );
	void	( *unused21 )					( void );
	void	( *unused22 )					( void );
	void	( *unused23 )					( void );
	void	( *unused24 )					( void );
	void	( *unused25 )					( void );
	void	( *unused26 )					( void );
	void	( *unused27 )					( void );
	void	( *unused28 )					( void );
	void	( *unused29 )					( void );
	void	( *unused30 )					( void );
	void	( *unused31 )					( void );
	void	( *unused32 )					( void );
	void	( *unused33 )					( void );
	void	( *unused34 )					( void );

	void	( *S_GetDSPointer )				( struct IDirectSound **lpDS, struct IDirectSoundBuffer **lpDSBuf );
	void 	*( *S_GetWAVPointer )			( void );

	void	( *unused35 )					( void );
	void	( *unused36 )					( void );
	void	( *unused37 )					( void );
	void	( *unused38 )					( void );
	void	( *unused39 )					( void );
	void	( *unused40 )					( void );
	void	( *unused41 )					( void );
	void	( *unused42 )					( void );
	void	( *unused43 )					( void );
	void	( *unused44 )					( void );
	void	( *unused45 )					( void );
	void	( *unused46 )					( void );
	void	( *unused47 )					( void );
	void	( *unused48 )					( void );
	void	( *unused49 )					( void );
	void	( *unused50 )					( void );
	void	( *unused51 )					( void );
	void	( *unused52 )					( void );
	void	( *unused53 )					( void );
	void	( *unused54 )					( void );
	void	( *unused55 )					( void );
} engine_api_t;

static engine_api_t engineapi;

typedef int (*engine_api_func)( int version, int size, struct engine_api_s *api );

//-----------------------------------------------------------------------------
// Purpose: Get launcher/engine interface from engine module
// Input  : hMod - 
// Output : int
//-----------------------------------------------------------------------------
int Eng_LoadFunctions( HMODULE hMod )
{
	engine_api_func pfnEngineAPI;
	
	pfnEngineAPI = ( engine_api_func )GetProcAddress( hMod, "Sys_EngineAPI"  );
	if ( !pfnEngineAPI )
		return 0;

	if ( !(*pfnEngineAPI)( ENGINE_LAUNCHER_API_VERSION, sizeof( engine_api_t ), &engineapi ) )
		return 0;

	// All is okay
	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Load proper engine .dll and get pointer to either DSound and primary buffer or HWAVEOUT ( NT 4.0, e.g. )
//-----------------------------------------------------------------------------
void LoadSoundAPIs( void )
{
	hEngine = ::LoadLibrary( IEngineStudio.IsHardware() ? "hw.dll" : "sw.dll" );
	if ( hEngine )
	{
		if ( Eng_LoadFunctions( hEngine ) )
		{
			if ( engineapi.S_GetDSPointer && engineapi.S_GetWAVPointer )
			{
				engineapi.S_GetDSPointer(&lpDS, &lpDSBuf);
				lpHW = (HWAVEOUT FAR *)engineapi.S_GetWAVPointer();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Close engine library, release sound pointers
//-----------------------------------------------------------------------------
void ShutdownSoundAPIs( void )
{
	if( hEngine )
	{
		FreeLibrary( hEngine );
		hEngine = 0;
	}
	
	lpDS = 0;
	lpDSBuf = 0;
	lpHW = 0;
}
