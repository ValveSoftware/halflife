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
// Scoreboard.cpp
//
// implementation of CHudScoreboard class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_COMMAND( m_Scoreboard, ShowScores );
DECLARE_COMMAND( m_Scoreboard, HideScores );

DECLARE_MESSAGE( m_Scoreboard, ScoreInfo );
DECLARE_MESSAGE( m_Scoreboard, TeamInfo );
DECLARE_MESSAGE( m_Scoreboard, TeamScore );


int CHudScoreboard :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	InitHUDData();

	return 1;
}


int CHudScoreboard :: VidInit( void )
{

	return 1;
}

void CHudScoreboard :: InitHUDData( void )
{
	memset( g_PlayerExtraInfo, 0, sizeof g_PlayerExtraInfo );
	m_iLastKilledBy = 0;
	m_fLastKillTime = 0;
	m_iPlayerNum = 0;
	m_iNumTeams = 0;

	memset( m_TeamInfo, 0, sizeof m_TeamInfo );

	m_iFlags &= ~HUD_ACTIVE;  // starts out inactive

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission
}

/* The scoreboard
We have a minimum width of 1-320 - we could have the field widths scale with it?
*/

// X positions
// relative to the side of the scoreboard
#define NAME_RANGE_MIN  20
#define NAME_RANGE_MAX  145
#define KILLS_RANGE_MIN 130
#define KILLS_RANGE_MAX 170
#define DIVIDER_POS		180
#define DEATHS_RANGE_MIN  185
#define DEATHS_RANGE_MAX  210
#define PING_RANGE_MIN	245
#define PING_RANGE_MAX	295

#define SCOREBOARD_WIDTH 320
		

// Y positions
#define ROW_GAP  13
#define ROW_RANGE_MIN 15
#define ROW_RANGE_MAX ( ScreenHeight - 50 )

int CHudScoreboard :: Draw( float fTime )
{
	if ( !m_iShowscoresHeld && gHUD.m_Health.m_iHealth > 0 && !gHUD.m_iIntermission )
		return 1;


	GetAllPlayersInfo();

	// just sort the list on the fly
	// list is sorted first by frags, then by deaths
	float list_slot = 0;
	int xpos_rel = (ScreenWidth - SCOREBOARD_WIDTH) / 2;

	// print the heading line
	int ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);
	int	xpos = NAME_RANGE_MIN + xpos_rel;

	if ( !gHUD.m_Teamplay ) 
		gHUD.DrawHudString( xpos, ypos, NAME_RANGE_MAX + xpos_rel, "Player", 255, 140, 0 );
	else
		gHUD.DrawHudString( xpos, ypos, NAME_RANGE_MAX + xpos_rel, "Teams", 255, 140, 0 );

	gHUD.DrawHudStringReverse( KILLS_RANGE_MAX + xpos_rel, ypos, 0, "kills", 255, 140, 0 );
	gHUD.DrawHudString( DIVIDER_POS + xpos_rel, ypos, ScreenWidth, "/", 255, 140, 0 );
	gHUD.DrawHudString( DEATHS_RANGE_MIN + xpos_rel + 5, ypos, ScreenWidth, "deaths", 255, 140, 0 );
	gHUD.DrawHudString( PING_RANGE_MAX + xpos_rel - 35, ypos, ScreenWidth, "latency", 255, 140, 0 );

	list_slot += 1.2;
	ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);
	xpos = NAME_RANGE_MIN + xpos_rel;
	FillRGBA( xpos - 5, ypos, PING_RANGE_MAX - 5, 1, 255, 140, 0, 255);  // draw the seperator line
	
	list_slot += 0.8;

	if ( !gHUD.m_Teamplay )
	{
		// it's not teamplay,  so just draw a simple player list
		DrawPlayers( xpos_rel, list_slot );
		return 1;
	}

	// clear out team scores
	for ( int i = 1; i <= m_iNumTeams; i++ )
	{
		if ( !m_TeamInfo[i].scores_overriden )
			m_TeamInfo[i].frags = m_TeamInfo[i].deaths = 0;
		m_TeamInfo[i].ping = m_TeamInfo[i].packetloss = 0;
	}

	// recalc the team scores, then draw them
	for ( i = 1; i < MAX_PLAYERS; i++ )
	{
		if ( m_PlayerInfoList[i].name == NULL )
			continue; // empty player slot, skip

		if ( m_PlayerExtraInfo[i].teamname[0] == 0 )
			continue; // skip over players who are not in a team

		// find what team this player is in
		for ( int j = 1; j <= m_iNumTeams; j++ )
		{
			if ( !stricmp( m_PlayerExtraInfo[i].teamname, m_TeamInfo[j].name ) )
				break;
		}
		if ( j > m_iNumTeams )  // player is not in a team, skip to the next guy
			continue;

		if ( !m_TeamInfo[j].scores_overriden )
		{
			m_TeamInfo[j].frags += m_PlayerExtraInfo[i].frags;
			m_TeamInfo[j].deaths += m_PlayerExtraInfo[i].deaths;
		}

		m_TeamInfo[j].ping += m_PlayerInfoList[i].ping;
		m_TeamInfo[j].packetloss += m_PlayerInfoList[i].packetloss;

		if ( m_PlayerInfoList[i].thisplayer )
			m_TeamInfo[j].ownteam = TRUE;
		else
			m_TeamInfo[j].ownteam = FALSE;
	}

	// find team ping/packetloss averages
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		m_TeamInfo[i].already_drawn = FALSE;

		if ( m_TeamInfo[i].players > 0 )
		{
			m_TeamInfo[i].ping /= m_TeamInfo[i].players;  // use the average ping of all the players in the team as the teams ping
			m_TeamInfo[i].packetloss /= m_TeamInfo[i].players;  // use the average ping of all the players in the team as the teams ping
		}
	}

	// Draw the teams
	while ( 1 )
	{
		int highest_frags = -99999; int lowest_deaths = 99999;
		int best_team = 0;

		for ( i = 1; i <= m_iNumTeams; i++ )
		{
			if ( m_TeamInfo[i].players < 0 )
				continue;

			if ( !m_TeamInfo[i].already_drawn && m_TeamInfo[i].frags >= highest_frags )
			{
				if ( m_TeamInfo[i].frags > highest_frags || m_TeamInfo[i].deaths < lowest_deaths )
				{
					best_team = i;
					lowest_deaths = m_TeamInfo[i].deaths;
					highest_frags = m_TeamInfo[i].frags;
				}
			}
		}

		// draw the best team on the scoreboard
		if ( !best_team )
			break;

		// draw out the best team
		team_info_t *team_info = &m_TeamInfo[best_team];

		ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);

		// check we haven't drawn too far down
		if ( ypos > ROW_RANGE_MAX )  // don't draw to close to the lower border
			break;

		xpos = NAME_RANGE_MIN + xpos_rel;
		int r = 255, g = 225, b = 55; // draw the stuff kinda yellowish
		
		if ( team_info->ownteam ) // if it is their team, draw the background different color
		{
			// overlay the background in blue,  then draw the score text over it
			FillRGBA( NAME_RANGE_MIN + xpos_rel - 5, ypos, PING_RANGE_MAX - 5, ROW_GAP, 0, 0, 255, 70 );
		}

		// draw their name (left to right)
		gHUD.DrawHudString( xpos, ypos, NAME_RANGE_MAX + xpos_rel, team_info->name, r, g, b );

		// draw kills (right to left)
		xpos = KILLS_RANGE_MAX + xpos_rel;
		gHUD.DrawHudNumberString( xpos, ypos, KILLS_RANGE_MIN + xpos_rel, team_info->frags, r, g, b );

		// draw divider
		xpos = DIVIDER_POS + xpos_rel;
		gHUD.DrawHudString( xpos, ypos, xpos + 20, "/", r, g, b );

		// draw deaths
		xpos = DEATHS_RANGE_MAX + xpos_rel;
		gHUD.DrawHudNumberString( xpos, ypos, DEATHS_RANGE_MIN + xpos_rel, team_info->deaths, r, g, b );

		// draw ping
		// draw ping & packetloss
		static char buf[64];
		sprintf( buf, "%d", team_info->ping );
		xpos = ((PING_RANGE_MAX - PING_RANGE_MIN) / 2) + PING_RANGE_MIN + xpos_rel + 25;
		UnpackRGB( r, g, b, RGB_YELLOWISH );
		gHUD.DrawHudStringReverse( xpos, ypos, xpos - 50, buf, r, g, b );

	/*  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
		sprintf( buf, " %d", team_info->packetloss );
		gHUD.DrawHudString( xpos, ypos, xpos+50, buf, r, g, b );
	*/

		team_info->already_drawn = TRUE;  // set the already_drawn to be TRUE, so this team won't get drawn again
		list_slot++;

		// draw all the players that belong to this team, indented slightly
		list_slot = DrawPlayers( xpos_rel, list_slot, 10, team_info->name );
	}

	// draw all the players who are not in a team
	list_slot += 0.5;
	DrawPlayers( xpos_rel, list_slot, 0, "" );

	return 1;
}

// returns the ypos where it finishes drawing
int CHudScoreboard :: DrawPlayers( int xpos_rel, float list_slot, int nameoffset, char *team )
{
	// draw the players, in order,  and restricted to team if set
	while ( 1 )
	{
		// Find the top ranking player
		int highest_frags = -99999;	int lowest_deaths = 99999;
		int best_player = 0;
		
		for ( int i = 1; i < MAX_PLAYERS; i++ )
		{
			if ( m_PlayerInfoList[i].name && m_PlayerExtraInfo[i].frags >= highest_frags )
			{
				if ( !(team && stricmp(m_PlayerExtraInfo[i].teamname, team)) )  // make sure it is the specified team
				{
					extra_player_info_t *pl_info = &m_PlayerExtraInfo[i];
					if ( pl_info->frags > highest_frags || pl_info->deaths < lowest_deaths )
					{
						best_player = i;
						lowest_deaths = pl_info->deaths;
						highest_frags = pl_info->frags;
	
					}
				}
			}
		}

		if ( !best_player )
			break;

		// draw out the best player
		hud_player_info_t *pl_info = &m_PlayerInfoList[best_player];

		int ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);

		// check we haven't drawn too far down
		if ( ypos > ROW_RANGE_MAX )  // don't draw to close to the lower border
			break;

		int xpos = NAME_RANGE_MIN + xpos_rel;
		int r = 255, g = 255, b = 255;
		if ( best_player == m_iLastKilledBy && m_fLastKillTime && m_fLastKillTime > gHUD.m_flTime )
		{
			if ( pl_info->thisplayer )
			{  // green is the suicide color? i wish this could do grey...
				FillRGBA( NAME_RANGE_MIN + xpos_rel - 5, ypos, PING_RANGE_MAX - 5, ROW_GAP, 80, 155, 0, 70 );
			}
			else
			{  // Highlight the killers name - overlay the background in red,  then draw the score text over it
				FillRGBA( NAME_RANGE_MIN + xpos_rel - 5, ypos, PING_RANGE_MAX - 5, ROW_GAP, 255, 0, 0, ((float)15 * (float)(m_fLastKillTime - gHUD.m_flTime)) );
			}
		}
		else if ( pl_info->thisplayer ) // if it is their name, draw it a different color
		{
			// overlay the background in blue,  then draw the score text over it
			FillRGBA( NAME_RANGE_MIN + xpos_rel - 5, ypos, PING_RANGE_MAX - 5, ROW_GAP, 0, 0, 255, 70 );
		}

		// draw their name (left to right)
		gHUD.DrawHudString( xpos + nameoffset, ypos, NAME_RANGE_MAX + xpos_rel, pl_info->name, r, g, b );

		// draw kills (right to left)
		xpos = KILLS_RANGE_MAX + xpos_rel;
		gHUD.DrawHudNumberString( xpos, ypos, KILLS_RANGE_MIN + xpos_rel, m_PlayerExtraInfo[best_player].frags, r, g, b );

		// draw divider
		xpos = DIVIDER_POS + xpos_rel;
		gHUD.DrawHudString( xpos, ypos, xpos + 20, "/", r, g, b );

		// draw deaths
		xpos = DEATHS_RANGE_MAX + xpos_rel;
		gHUD.DrawHudNumberString( xpos, ypos, DEATHS_RANGE_MIN + xpos_rel, m_PlayerExtraInfo[best_player].deaths, r, g, b );

		// draw ping & packetloss
		static char buf[64];
		sprintf( buf, "%d", m_PlayerInfoList[best_player].ping );
		xpos = ((PING_RANGE_MAX - PING_RANGE_MIN) / 2) + PING_RANGE_MIN + xpos_rel + 25;
		gHUD.DrawHudStringReverse( xpos, ypos, xpos - 50, buf, r, g, b );

	/*  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
		if ( m_PlayerInfoList[best_player].packetloss >= 63 )
		{
			UnpackRGB( r, g, b, RGB_REDISH );
			sprintf( buf, " !!!!" );
		}
		else
		{
			sprintf( buf, " %d", m_PlayerInfoList[best_player].packetloss );
		}

		gHUD.DrawHudString( xpos, ypos, xpos+50, buf, r, g, b );
	*/

		pl_info->name = NULL;  // set the name to be NULL, so this client won't get drawn again
		list_slot++;
	}

	return list_slot;
}


void CHudScoreboard :: GetAllPlayersInfo( void )
{
	for ( int i = 1; i < MAX_PLAYERS; i++ )
	{
		GetPlayerInfo( i, &m_PlayerInfoList[i] );

		if ( m_PlayerInfoList[i].thisplayer )
			m_iPlayerNum = i;  // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
	}
}

int CHudScoreboard :: MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_ACTIVE;

	BEGIN_READ( pbuf, iSize );
	short cl = READ_BYTE();
	short frags = READ_SHORT();
	short deaths = READ_SHORT();

	if ( cl > 0 && cl <= MAX_PLAYERS )
	{
		m_PlayerExtraInfo[cl].frags = frags;
		m_PlayerExtraInfo[cl].deaths = deaths;
	}

	return 1;
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
int CHudScoreboard :: MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	short cl = READ_BYTE();
	
	if ( cl > 0 && cl <= MAX_PLAYERS )
	{  // set the players team
		strncpy( m_PlayerExtraInfo[cl].teamname, READ_STRING(), MAX_TEAM_NAME );
	}

	// rebuild the list of teams

	// clear out player counts from teams
	for ( int i = 1; i <= m_iNumTeams; i++ )
	{
		m_TeamInfo[i].players = 0;
	}

	// rebuild the team list
	GetAllPlayersInfo();
	m_iNumTeams = 0;
	for ( i = 1; i < MAX_PLAYERS; i++ )
	{
		if ( m_PlayerInfoList[i].name == NULL )
			continue;

		if ( m_PlayerExtraInfo[i].teamname[0] == 0 )
			continue; // skip over players who are not in a team

		// is this player in an existing team?
		for ( int j = 1; j <= m_iNumTeams; j++ )
		{
			if ( m_TeamInfo[j].name[0] == '\0' )
				break;

			if ( !stricmp( m_PlayerExtraInfo[i].teamname, m_TeamInfo[j].name ) )
				break;
		}

		if ( j > m_iNumTeams )
		{ // they aren't in a listed team, so make a new one
			// search through for an empty team slot
			for ( int j = 1; j <= m_iNumTeams; j++ )
			{
				if ( m_TeamInfo[j].name[0] == '\0' )
					break;
			}
			m_iNumTeams = max( j, m_iNumTeams );

			strncpy( m_TeamInfo[j].name, m_PlayerExtraInfo[i].teamname, MAX_TEAM_NAME );
			m_TeamInfo[j].players = 0;
		}

		m_TeamInfo[j].players++;
	}

	// clear out any empty teams
	for ( i = 1; i <= m_iNumTeams; i++ )
	{
		if ( m_TeamInfo[i].players < 1 )
			memset( &m_TeamInfo[i], 0, sizeof(team_info_t) );
	}

	return 1;
}

// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths 
// if this message is never received, then scores will simply be the combined totals of the players.
int CHudScoreboard :: MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *TeamName = READ_STRING();

	// find the team matching the name
	for ( int i = 1; i <= m_iNumTeams; i++ )
	{
		if ( !stricmp( TeamName, m_TeamInfo[i].name ) )
			break;
	}
	if ( i > m_iNumTeams )
		return 1;

	// use this new score data instead of combined player scores
	m_TeamInfo[i].scores_overriden = TRUE;
	m_TeamInfo[i].frags = READ_SHORT();
	m_TeamInfo[i].deaths = READ_SHORT();
	
	return 1;
}

void CHudScoreboard :: DeathMsg( int killer, int victim )
{
	// if we were the one killed,  or the world killed us, set the scoreboard to indicate suicide
	if ( victim == m_iPlayerNum || killer == 0 )
	{
		m_iLastKilledBy = killer ? killer : m_iPlayerNum;
		m_fLastKillTime = gHUD.m_flTime + 10;	// display who we were killed by for 10 seconds

		if ( killer == m_iPlayerNum )
			m_iLastKilledBy = m_iPlayerNum;
	}
}



void CHudScoreboard :: UserCmd_ShowScores( void )
{
	m_iShowscoresHeld = TRUE;
}

void CHudScoreboard :: UserCmd_HideScores( void )
{
	m_iShowscoresHeld = FALSE;
}
