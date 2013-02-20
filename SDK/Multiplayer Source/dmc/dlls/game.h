/***
*
*	Copyright (c) 1996-2002 Valve LLC. All rights reserved.
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

#ifndef GAME_H
#define GAME_H

extern void GameDLLInit( void );


extern cvar_t	displaysoundlist;
extern cvar_t	mapcyclefile;
extern cvar_t	servercfgfile;
extern cvar_t	lservercfgfile;

// multiplayer server rules
extern cvar_t	fraglimit;
extern cvar_t	timelimit;
extern cvar_t	friendlyfir;
extern cvar_t	falldamage;
extern cvar_t	weaponstay;
extern cvar_t	forcerespaw;
extern cvar_t	flashlight;
extern cvar_t	aimcrosshair;
extern cvar_t	decalfrequency;
extern cvar_t	teamlist;
extern cvar_t	teamoverride;
extern cvar_t	defaultteam;

// Engine Cvars
extern cvar_t	*g_psv_gravity;
extern cvar_t	*g_psv_aim;
extern cvar_t	*g_footsteps;

#endif		// GAME_H
