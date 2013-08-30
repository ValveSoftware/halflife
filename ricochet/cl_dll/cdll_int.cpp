/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include "netadr.h"
#include "vgui_SchemeManager.h"

#include "interface.h"

cl_enginefunc_t gEngfuncs;
CHud gHUD	;
TeamFortressViewport *gViewPort = NULL;

extern "C"
{
#include "pm_shared.h"
}

#include "hud_servers.h"
#include "vgui_int.h"

CSysModule *g_hTrackerModule = NULL;
#ifdef _WIN32
#endif
void InitInput (void);
void EV_HookEvents( void );
void IN_Commands( void );

/*
========================== 
    Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C" 
{
int EXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int EXPORT HUD_VidInit( void );
int EXPORT HUD_Init( void );
int EXPORT HUD_Redraw( float flTime, int intermission );
int EXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
int EXPORT HUD_Reset ( void );
void EXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void EXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char EXPORT HUD_PlayerMoveTexture( char *name );
int EXPORT HUD_ConnectionlessPacket( struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int EXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void EXPORT HUD_Frame( double time );
void EXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
void	EXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int EXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	EXPORT HUD_ConnectionlessPacket( struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void EXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char EXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void EXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}

int EXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

	//!!! mwh UNDONE We need to think about our versioning strategy. Do we want to try to be compatible
	// with previous versions, especially when we're only 'bonus' functionality? Should it be the engine
	// that decides if the DLL is compliant?

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();

	return 1;
}


/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

int EXPORT HUD_VidInit( void )
{
	gHUD.VidInit();

	VGui_Startup();

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

int EXPORT HUD_Init( void )
{
	InitInput();
	gHUD.Init();
	Scheme_Init();
	return 1;
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int EXPORT HUD_Redraw( float time, int intermission )
{
	gHUD.Redraw( time, intermission );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int EXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();
	
	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

int EXPORT HUD_Reset( void )
{
	gHUD.VidInit();
	return 1;
}


/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void EXPORT HUD_Frame( double time )
{
	ServersThink( time );

	GetClientVoiceMgr()->Frame(time);
}

/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void EXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}