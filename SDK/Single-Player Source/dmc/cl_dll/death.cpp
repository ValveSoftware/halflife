/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
// death notice
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

#include "vgui_viewport.h"

DECLARE_MESSAGE( m_DeathNotice, DeathMsg );

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	int iSuicide;
	int iTeamKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1]; 
extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS+1];

float g_ColorBlue[3]	= { 0.6, 0.8, 1.0 };
float g_ColorRed[3]		= { 1.0, 0.25, 0.25 };
float g_ColorGreen[3]	= { 0.6, 1.0, 0.6 };
float g_ColorYellow[3]	= { 1.0, 0.7, 0.0 };
float g_ColorYellowish[3]	= { 1.0, 0.625, 0.0 };

float *GetClientColor( int clientIndex )
{
	const char *teamName = g_PlayerExtraInfo[ clientIndex].teamname;

	if ( !teamName || *teamName == 0 ) 
		return g_ColorYellowish;

	if ( !stricmp( "blue", teamName ) )
		return g_ColorBlue;
	else if ( !stricmp( "red", teamName ) )
		return g_ColorRed;
	else if ( !stricmp( "green", teamName ) )
		return g_ColorGreen;
	else if ( !stricmp( "yellow", teamName ) )
		return g_ColorYellow;

	return g_ColorYellowish;
}

int GetTeamIndex( int clientIndex )
{
	const char *teamName = g_PlayerExtraInfo[ clientIndex].teamname;

	if ( !teamName || *teamName == 0 ) 
		return NULL;

	if ( !stricmp( "red", teamName ) )
		return 1;
	else if ( !stricmp( "blue", teamName ) )
		return 2;

	return 0;
}

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( DeathMsg );

	CVAR_CREATE( "hud_deathnotice_time", "6", 0 );

	return 1;
}


void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}


int CHudDeathNotice :: VidInit( void )
{
	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );

	return 1;
}

int CHudDeathNotice :: Draw( float flTime )
{
	int x, y, r, g, b;

	for ( int i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if ( rgDeathNoticeList[i].flDisplayTime < flTime )
		{ // display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i+1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i) );
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min( rgDeathNoticeList[i].flDisplayTime, gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME );

			// Draw the death notice
			y = YRES(DEATHNOTICE_TOP) + 2 + (20 * i);  //!!!

		int id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
		x = ScreenWidth - ConsoleStringLen(rgDeathNoticeList[i].szVictim) - (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

		if ( !rgDeathNoticeList[i].iSuicide )
		{
			x -= (5 + ConsoleStringLen( rgDeathNoticeList[i].szKiller ) );

			// Draw killers name
			if ( rgDeathNoticeList[i].KillerColor )
				gEngfuncs.pfnDrawSetTextColor( rgDeathNoticeList[i].KillerColor[0], rgDeathNoticeList[i].KillerColor[1], rgDeathNoticeList[i].KillerColor[2] );
			x = 5 + DrawConsoleString( x, y, rgDeathNoticeList[i].szKiller );
		}

		r = 255;  g = 80;	b = 0;
		if ( rgDeathNoticeList[i].iTeamKill )
		{
			r = 10;	g = 240; b = 10;  // display it in sickly green
		}

		// Draw death weapon
		SPR_Set( gHUD.GetSprite(id), r, g, b );
		SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(id) );

		x += (gHUD.GetSpriteRect(id).right - gHUD.GetSpriteRect(id).left);

		// Draw victims name
		if ( rgDeathNoticeList[i].VictimColor )
			gEngfuncs.pfnDrawSetTextColor( rgDeathNoticeList[i].VictimColor[0], rgDeathNoticeList[i].VictimColor[1], rgDeathNoticeList[i].VictimColor[2] );
		x = DrawConsoleString( x, y, rgDeathNoticeList[i].szVictim );
	}

	return 1;
}


// This message handler may be better off elsewhere
int CHudDeathNotice :: MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );

	int killer = READ_BYTE();
	int victim = READ_BYTE();

	char killedwith[32];
	strcpy( killedwith, "d_" );
	strncat( killedwith, READ_STRING(), 32 );

	if (gViewPort)
		gViewPort->DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage(victim);
	for ( int i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if ( i == MAX_DEATHNOTICES )
	{ // move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList+1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	//gHUD.m_Scoreboard.GetAllPlayersInfo();

	if ( gViewPort )
		gViewPort->GetAllPlayersInfo();

	char *killer_name = g_PlayerInfoList[ killer ].name;
	char *victim_name = g_PlayerInfoList[ victim ].name;
	if ( !killer_name )
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strncpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	if ( !victim_name )
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strncpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	if ( killer == victim || killer == 0 )
		rgDeathNoticeList[i].iSuicide = TRUE;

	if ( !strcmp( killedwith, "d_teammate" ) )
		rgDeathNoticeList[i].iTeamKill = TRUE;

	// Find the sprite in the list
	int spr = gHUD.GetSpriteIndex( killedwith );

	rgDeathNoticeList[i].iId = spr;

	DEATHNOTICE_DISPLAY_TIME = CVAR_GET_FLOAT( "hud_deathnotice_time" );
	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + DEATHNOTICE_DISPLAY_TIME;

	// record the death notice in the console
	if ( rgDeathNoticeList[i].iSuicide )
	{
		ConsolePrint( rgDeathNoticeList[i].szVictim );

		if ( !strcmp( killedwith, "d_world" ) )
		{
			ConsolePrint( " died" );
		}
		else
		{
			ConsolePrint( " killed self" );
		}
	}
	else if ( rgDeathNoticeList[i].iTeamKill )
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed his teammate " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
	}
	else
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
	}

	if ( killedwith && *killedwith && (*killedwith > 13 ) && strcmp( killedwith, "d_world" ) && !rgDeathNoticeList[i].iTeamKill )
	{
		ConsolePrint( " with " );

		// replace the code names with the 'real' names
		if ( !strcmp( killedwith+2, "egon" ) )
			strcpy( killedwith, "d_gluon gun" );
		if ( !strcmp( killedwith+2, "gauss" ) )
			strcpy( killedwith, "d_tau cannon" );

		ConsolePrint( killedwith+2 ); // skip over the "d_" part
	}

	ConsolePrint( "\n" );

	return 1;
}



