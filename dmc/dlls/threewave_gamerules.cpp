/***
*
*	Copyright (c) 2001, Valve LLC. All rights reserved.
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
/*

===== threewave_gamerules.cpp ========================================================

  This contains all the gamerules for the ThreeWave CTF Gamemode.
  It also contains the Flag entity information.

*/

#ifdef THREEWAVE

#define NUM_TEAMS 2

char *sTeamNames[] =
{
	"SPECTATOR",
	"RED",
	"BLUE",
};

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
#include	"skill.h"
#include	"game.h"
#include	"items.h"
#include	"threewave_gamerules.h"

extern int gmsgCTFMsgs;
extern int gmsgShowMenu;
extern int gmsgFlagStatus;
extern int gmsgRuneStatus;
extern int gmsgFlagCarrier;
extern int gmsgScoreInfo;

extern unsigned short g_usHook;	
extern unsigned short g_usCable;
extern unsigned short g_usCarried;
extern unsigned short g_usFlagSpawn;


static char team_names[MAX_TEAMS][MAX_TEAMNAME_LENGTH];
static int team_scores[MAX_TEAMS];
static int num_teams = 0;

bool g_bSpawnedRunes;
void SpawnRunes( void );

extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer, bool bCheckDM );
extern edict_t *RuneSelectSpawnPoint( void );

// Standard Scoring
#define TEAM_CAPTURE_CAPTURE_BONUS 5 // what you get for capture
#define TEAM_CAPTURE_TEAM_BONUS  10 // what your team gets for capture
#define TEAM_CAPTURE_RECOVERY_BONUS  1 // what you get for recovery
#define TEAM_CAPTURE_FLAG_BONUS  0 // what you get for picking up enemy flag
#define TEAM_CAPTURE_FRAG_CARRIER_BONUS  2 // what you get for fragging a enemy flag carrier
#define TEAM_CAPTURE_FLAG_RETURN_TIME 40 // seconds until auto return

// bonuses
#define TEAM_CAPTURE_CARRIER_DANGER_PROTECT_BONUS  2 // bonus for fraggin someone
// who has recently hurt your flag carrier
#define TEAM_CAPTURE_CARRIER_PROTECT_BONUS 1 // bonus for fraggin someone while
// either you or your target are near your flag carrier
#define TEAM_CAPTURE_FLAG_DEFENSE_BONUS 1 // bonus for fraggin someone while
// either you or your target are near your flag
#define TEAM_CAPTURE_RETURN_FLAG_ASSIST_BONUS 1 // awarded for returning a flag that causes a
// capture to happen almost immediately
#define TEAM_CAPTURE_FRAG_CARRIER_ASSIST_BONUS 2 // award for fragging a flag carrier if a
// capture happens almost immediately

// Radius
#define TEAM_CAPTURE_TARGET_PROTECT_RADIUS 550 // the radius around an object being
// defended where a target will be worth extra frags
#define TEAM_CAPTURE_ATTACKER_PROTECT_RADIUS 550 // the radius around an object being
// defended where an attacker will get extra frags when making kills

// timeouts
#define TEAM_CAPTURE_CARRIER_DANGER_PROTECT_TIMEOUT 4
#define TEAM_CAPTURE_CARRIER_FLAG_SINCE_TIMEOUT 2
#define TEAM_CAPTURE_FRAG_CARRIER_ASSIST_TIMEOUT 6
#define TEAM_CAPTURE_RETURN_FLAG_ASSIST_TIMEOUT 4



class CThreeWaveGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pPlayer1, CBasePlayer *pPlayer2)
	{
		return stricmp(pPlayer1->TeamID(), pPlayer2->TeamID()) == 0;
	}
};
static CThreeWaveGameMgrHelper g_GameMgrHelper;



extern DLL_GLOBAL BOOL		g_fGameOver;

char* GetTeamName( int team )
{
	if ( team < 0 || team > NUM_TEAMS )
		team = 0;

	return sTeamNames[ team ];
}

CThreeWave :: CThreeWave()
{
	// CHalfLifeMultiplay already initialized it - just override its helper callback.
	m_VoiceGameMgr.SetHelper(&g_GameMgrHelper);

	m_DisableDeathMessages = FALSE;
	m_DisableDeathPenalty = FALSE;

	memset( team_names, 0, sizeof(team_names) );
	memset( team_scores, 0, sizeof(team_scores) );
	num_teams = 0;

	iBlueTeamScore = iRedTeamScore = 0;
	g_bSpawnedRunes = FALSE;

	// Copy over the team from the server config
	m_szTeamList[0] = 0;

	// Cache this because the team code doesn't want to deal with changing this in the middle of a game
	strncpy( m_szTeamList, teamlist.string, TEAMPLAY_TEAMLISTLENGTH );

	edict_t *pWorld = INDEXENT(0);
	if ( pWorld && pWorld->v.team )
	{
		if ( teamoverride.value )
		{
			const char *pTeamList = STRING(pWorld->v.team);
			if ( pTeamList && strlen(pTeamList) )
			{
				strncpy( m_szTeamList, pTeamList, TEAMPLAY_TEAMLISTLENGTH );
			}
		}
	}
	// Has the server set teams
	if ( strlen( m_szTeamList ) )
		m_teamLimit = TRUE;
	else
		m_teamLimit = FALSE;

	RecountTeams();
}


BOOL CThreeWave::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] )
{
	return CHalfLifeMultiplay::ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}


extern cvar_t timeleft, fragsleft;

void CThreeWave :: Think ( void )
{
	m_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if ( g_fGameOver )   // someone else quit the game already
	{
		CHalfLifeMultiplay::Think();
		return;
	}

	float flTimeLimit = CVAR_GET_FLOAT("mp_timelimit") * 60;
	
	time_remaining = (int)(flTimeLimit ? ( flTimeLimit - gpGlobals->time ) : 0);

	if ( flTimeLimit != 0 && gpGlobals->time >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	float flFragLimit = fraglimit.value;
	if ( flFragLimit )
	{
		int bestfrags = 9999;
		int remain;

		// check if any team is over the frag limit
		for ( int i = 0; i < num_teams; i++ )
		{
			if ( team_scores[i] >= flFragLimit )
			{
				GoToIntermission();
				return;
			}

			remain = flFragLimit - team_scores[i];
			if ( remain < bestfrags )
			{
				bestfrags = remain;
			}
		}
		frags_remaining = bestfrags;
	}

	if ( !g_bSpawnedRunes )
		SpawnRunes();

	if ( m_flFlagStatusTime && m_flFlagStatusTime <= gpGlobals->time )
         GetFlagStatus( NULL );
	
	// Updates when frags change
	if ( frags_remaining != last_frags )
	{
		g_engfuncs.pfnCvar_DirectSet( &fragsleft, UTIL_VarArgs( "%i", frags_remaining ) );
	}

	// Updates once per second
	if ( timeleft.value != last_time )
	{
		g_engfuncs.pfnCvar_DirectSet( &timeleft, UTIL_VarArgs( "%i", time_remaining ) );
	}

	last_frags = frags_remaining;
	last_time  = time_remaining;
}

void CThreeWave :: JoinTeam ( CBasePlayer *pPlayer, int iTeam )
{
	if ( pPlayer->pev->team == iTeam )
		return;

	if ( pPlayer->m_flNextTeamChange > gpGlobals->time )
		return;

	pPlayer->m_flNextTeamChange = gpGlobals->time + 5;

	if ( pPlayer->pev->team == 0 )
	{
		ChangePlayerTeam( pPlayer, iTeam );
		RecountTeams();

		pPlayer->Spawn();
	}
	else 
	{
		ChangePlayerTeam( pPlayer, iTeam );
        RecountTeams();
	}
}

int CThreeWave::TeamWithFewestPlayers( void )
{

	CBaseEntity *pPlayer = NULL;
	CBasePlayer *player = NULL;

	int iNumRed, iNumBlue;
	
	int iTeam;

	// Initialize the player counts..
	iNumRed = iNumBlue = 0;
	
	pPlayer = UTIL_FindEntityByClassname ( pPlayer, "player" );

	while (	(pPlayer != NULL) && (!FNullEnt(pPlayer->edict()))	)
	{
		if (pPlayer->pev->flags != FL_DORMANT)
		{
			player = GetClassPtr((CBasePlayer *)pPlayer->pev);

			
				if ( player->pev->team == RED )
					iNumRed += 1;
	
				else if ( player->pev->team == BLUE )
					iNumBlue += 1;
		
		}
		pPlayer = UTIL_FindEntityByClassname ( pPlayer, "player" );
	}

	if ( iNumRed == iNumBlue )
	{
		switch ( RANDOM_LONG( 0, 1 ) )
		{
			case 0: iTeam = RED; break;
			case 1: iTeam = BLUE; break;
		}
	}
	else if ( iNumRed == 0 && iNumBlue == 0)
	{
		switch ( RANDOM_LONG( 0, 1 ) )
		{
			case 0: iTeam = RED; break;
			case 1: iTeam = BLUE; break;
		}
	}

	else if ( iNumRed > iNumBlue )
		iTeam = BLUE;

	else if ( iNumRed < iNumBlue )
		iTeam = RED;

	return iTeam;

}

void DropRune ( CBasePlayer *pPlayer );
//=========================================================
// ClientCommand
// the user has typed a command which is unrecognized by everything else;
// this check to see if the gamerules knows anything about the command
//=========================================================
BOOL CThreeWave :: ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if( m_VoiceGameMgr.ClientCommand( pPlayer, pcmd ) )
		return TRUE;

	if ( FStrEq( pcmd, "menuselect" ) )
	{
		if ( CMD_ARGC() < 2 )
			return TRUE;

		int slot = atoi( CMD_ARGV(1) );

		// select the item from the current menu
		switch( pPlayer->m_iMenu )
		{
			case Team_Menu:
				
				switch ( slot )
				{
				case 1: 
					JoinTeam( pPlayer, RED );
					break;
				case 2:
					JoinTeam( pPlayer, BLUE );
					break;
				case 5:
					JoinTeam( pPlayer, TeamWithFewestPlayers() );
					break;
				}

				break;

			case Team_Menu_IG:

				switch ( slot )
				{
				case 1: 
					JoinTeam( pPlayer, RED );
					break;
				case 2:
					JoinTeam( pPlayer, BLUE );
					break;
				case 5:
					JoinTeam( pPlayer, TeamWithFewestPlayers() );
					break;
				default:
					return TRUE;
				}

				break;
		}
 
		return TRUE;
	}
	else if ( FStrEq( pcmd, "droprune" ) )
	{
		DropRune( pPlayer );

		return TRUE;
	}
	else if ( FStrEq( pcmd, "changeteam" ) )
	{
		if ( pPlayer->pev->team != 0 )
		{
			 pPlayer->ShowMenu( 1 + 2 + 16 + 512, -1, FALSE, "#Team_Menu_Join_IG" );
			 pPlayer->m_iMenu = Team_Menu_IG;
		}

		return TRUE;
	}

	return FALSE;
}

extern int gmsgGameMode;
extern int gmsgSayText;
extern int gmsgTeamInfo;

void CThreeWave :: UpdateGameMode( CBasePlayer *pPlayer )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgGameMode, NULL, pPlayer->edict() );
		WRITE_BYTE( 1 );  // game mode teamplay
	MESSAGE_END();
}

edict_t *CThreeWave::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	edict_t *pentSpawnSpot;

	if ( FBitSet( pPlayer->m_afPhysicsFlags, PFLAG_OBSERVER ) || pPlayer->pev->team == 0 )
	     pentSpawnSpot = EntSelectSpawnPoint( pPlayer, FALSE );
	else
	{
		if ( RANDOM_LONG ( 1, 7 ) < 3 )
			pentSpawnSpot= EntSelectSpawnPoint( pPlayer, TRUE );	
		else
			pentSpawnSpot= EntSelectSpawnPoint( pPlayer, FALSE );	
	}

	if ( IsMultiplayer() && pentSpawnSpot->v.target )
	{
		FireTargets( STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0 );
	}

	pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
	pPlayer->pev->v_angle  = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = TRUE;

	return pentSpawnSpot;
}

void CThreeWave :: PlayerTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if ( !pAttacker->IsPlayer() )
		return;

	if ( pPlayer->pev->team == pAttacker->pev->team )
		return;

	if ( pPlayer->m_bHasFlag )
	{
		pPlayer->pCarrierHurter = (CBasePlayer *)pAttacker;
		pPlayer->m_flCarrierHurtTime = gpGlobals->time + TEAM_CAPTURE_CARRIER_DANGER_PROTECT_TIMEOUT;
	}

}

void CThreeWave :: PlayerSpawn( CBasePlayer *pPlayer )
{
	BOOL		addDefault;
	CBaseEntity	*pWeaponEntity = NULL;

	if ( pPlayer->pev->team == 0 )
	{
		pPlayer->pev->takedamage		= DAMAGE_NO;
		pPlayer->pev->solid			= SOLID_NOT;
		pPlayer->pev->movetype		= MOVETYPE_NOCLIP;
		pPlayer->pev->effects		|= EF_NODRAW;
		pPlayer->pev->flags |= FL_NOTARGET;
        pPlayer->m_afPhysicsFlags |= PFLAG_OBSERVER;
        pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS | HIDEHUD_FLASHLIGHT | HIDEHUD_HEALTH; 

		pPlayer->m_flFlagStatusTime = gpGlobals->time + 0.1;
	}
	else
	{
		pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
		
		addDefault = TRUE;

		while ( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" ))
		{
			pWeaponEntity->Touch( pPlayer );
			addDefault = FALSE;
		}

		if ( addDefault )
		{
			pPlayer->m_bHasFlag = FALSE;

			pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
			pPlayer->m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
			pPlayer->m_iHideHUD &= ~HIDEHUD_HEALTH;
			pPlayer->m_afPhysicsFlags &= ~PFLAG_OBSERVER;

			// Start with init ammoload
			pPlayer->m_iAmmoShells = 25;

			// Start with shotgun and axe
			pPlayer->GiveNamedItem( "weapon_quakegun" );
			pPlayer->m_iQuakeItems |= ( IT_SHOTGUN | IT_AXE | IT_EXTRA_WEAPON );
			pPlayer->m_iQuakeWeapon = pPlayer->W_BestWeapon();
			pPlayer->W_SetCurrentAmmo();

			pPlayer->m_flFlagStatusTime = gpGlobals->time + 0.1;
		}
	}

/*	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pPlayer->pev);
		WRITE_BYTE( pPlayer->m_iRuneStatus );
	MESSAGE_END();*/
}

void CBasePlayer::ShowMenu ( int bitsValidSlots, int nDisplayTime, BOOL fNeedMore, char *pszText )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgShowMenu, NULL, pev);
        WRITE_SHORT( bitsValidSlots);
        WRITE_CHAR( nDisplayTime );
        WRITE_BYTE( fNeedMore );
        WRITE_STRING (pszText);
    MESSAGE_END();
}

//=========================================================
// InitHUD
//=========================================================
void CThreeWave::InitHUD( CBasePlayer *pPlayer )
{
	CHalfLifeMultiplay::InitHUD( pPlayer );

	int clientIndex = pPlayer->entindex();
	// update this player with all the other players team info
	// loop through all active players and send their team info to the new client
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *plr = UTIL_PlayerByIndex( i );
		if ( plr )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgTeamInfo, NULL, pPlayer->edict() );
				WRITE_BYTE( plr->entindex() );
				WRITE_STRING( plr->TeamID() );
			MESSAGE_END();

			if ( ((CBasePlayer *)plr)->m_bHasFlag )
			{
				MESSAGE_BEGIN( MSG_ONE, gmsgFlagCarrier, NULL, pPlayer->edict() );
					WRITE_BYTE( plr->entindex() );
					WRITE_BYTE( 1 );
				MESSAGE_END();
			}
		}
	}

	//Remove Rune icon if we have one.
	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pPlayer->pev);
		WRITE_BYTE( 0 );
	MESSAGE_END();

	if ( pPlayer->pev->team == 0)
	{
		 pPlayer->ShowMenu( 1 + 2 + 16, -1, FALSE, "#Team_Menu_Join" );
		 pPlayer->m_iMenu = Team_Menu;
	}
}


void CThreeWave::ChangePlayerTeam( CBasePlayer *pPlayer, int iTeam )
{
	int damageFlags = DMG_GENERIC;
	int clientIndex = pPlayer->entindex();

	if ( pPlayer->pev->team != 0 )
	{
		damageFlags |= DMG_ALWAYSGIB;

		// kill the player,  remove a death,  and let them start on the new team
		m_DisableDeathMessages = TRUE;
		m_DisableDeathPenalty = TRUE;

		entvars_t *pevWorld = VARS( INDEXENT(0) );
		pPlayer->TakeDamage( pevWorld, pevWorld, 900, damageFlags );

		m_DisableDeathMessages = FALSE;
		m_DisableDeathPenalty = FALSE;
	}

	int oldTeam = pPlayer->pev->team;
	pPlayer->pev->team = iTeam;

	if ( pPlayer->pev->team == RED )
	{
		strncpy( pPlayer->m_szTeamName, "RED", TEAM_NAME_LENGTH );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model", "red" );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "topcolor", UTIL_VarArgs( "%d", 255 ) );
	}
	else if ( pPlayer->pev->team == BLUE )
	{
		strncpy( pPlayer->m_szTeamName, "BLUE", TEAM_NAME_LENGTH );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model", "blue" );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "topcolor", UTIL_VarArgs( "%d", 153 ) );
	}

	// notify everyone's HUD of the team change
	MESSAGE_BEGIN( MSG_ALL, gmsgTeamInfo );
		WRITE_BYTE( clientIndex );
		WRITE_STRING( pPlayer->m_szTeamName );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(pPlayer->edict()) );
		WRITE_SHORT( pPlayer->pev->frags );
		WRITE_SHORT( pPlayer->m_iDeaths );
		WRITE_SHORT( pPlayer->pev->team );
	MESSAGE_END();

	// log the change
	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" joined team \"%s\"\n", 
		STRING(pPlayer->pev->netname),
		GETPLAYERUSERID( pPlayer->edict() ),
		GETPLAYERAUTHID( pPlayer->edict() ),
		GetTeamName( oldTeam ),
		pPlayer->m_szTeamName );
}


//=========================================================
// ClientUserInfoChanged
//=========================================================
void CThreeWave::ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer )
{

	int clientIndex = pPlayer->entindex();

	if ( pPlayer->pev->team == RED )
	{
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "topcolor", UTIL_VarArgs( "%d", 255 ) );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model", "red" );
	}
	else if ( pPlayer->pev->team == BLUE )
	{
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "topcolor", UTIL_VarArgs( "%d", 153 ) );
		g_engfuncs.pfnSetClientKeyValue( clientIndex, g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model", "blue" );
	}

}

extern int gmsgDeathMsg;

//=========================================================
// Deathnotice. 
//=========================================================
void CThreeWave::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor )
{
	if ( m_DisableDeathMessages )
		return;
	
	if ( pVictim && pKiller && pKiller->flags & FL_CLIENT )
	{
		CBasePlayer *pk = (CBasePlayer*) CBaseEntity::Instance( pKiller );

		if ( pk )
		{
			if ( (pk != pVictim) && (PlayerRelationship( pVictim, pk ) == GR_TEAMMATE) )
			{
				MESSAGE_BEGIN( MSG_ALL, gmsgDeathMsg );
					WRITE_BYTE( ENTINDEX(ENT(pKiller)) );		// the killer
					WRITE_BYTE( ENTINDEX(pVictim->edict()) );	// the victim
					WRITE_STRING( "teammate" );		// flag this as a teammate kill
				MESSAGE_END();
				return;
			}
		}
	}

	CHalfLifeMultiplay::DeathNotice( pVictim, pKiller, pevInflictor );
}

//=========================================================
//=========================================================
void CThreeWave :: ClientDisconnected( edict_t *pClient )
{
	if ( pClient )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );

		if ( pPlayer )
		{
			//We have the flag, spawn it
			if ( pPlayer->m_bHasFlag )
			{
				CBaseEntity *pEnt; 

				//We have the BLUE flag, Spawn it
				if ( pPlayer->pev->team == RED )
				{
					pEnt = CBaseEntity::Create( "item_flag_team2", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );

					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_Blue_Flag\"\n",  
						STRING( pPlayer->pev->netname ), 
						GETPLAYERUSERID( pPlayer->edict() ),
						GETPLAYERAUTHID( pPlayer->edict() ),
						GetTeamName( pPlayer->pev->team ) );
				}
				//We have the RED flag, Spawn it
				else if ( pPlayer->pev->team == BLUE )
				{
					pEnt = CBaseEntity::Create( "item_flag_team1", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );

					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_Red_Flag\"\n",  
							STRING( pPlayer->pev->netname ), 
							GETPLAYERUSERID( pPlayer->edict() ),
							GETPLAYERAUTHID( pPlayer->edict() ),
							GetTeamName( pPlayer->pev->team ) );
				}
   
				pEnt->pev->velocity = pPlayer->pev->velocity * 1.2; 
				pEnt->pev->angles.x = 0;

				CItemFlag *pFlag = (CItemFlag *)pEnt; 
				pFlag->Dropped = TRUE; 
				pFlag->m_flDroppedTime = gpGlobals->time + TEAM_CAPTURE_FLAG_RETURN_TIME;

				PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
				pPlayer->edict(), g_usCarried, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
				0.0, 0.0, pPlayer->entindex(), pPlayer->pev->team, 1, 0 );

				MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
					if ( pPlayer->pev->team == RED )
						WRITE_BYTE( BLUE_FLAG_LOST );
					else if ( pPlayer->pev->team == BLUE )
						WRITE_BYTE( RED_FLAG_LOST );
			
					WRITE_STRING( STRING(pPlayer->pev->netname) );
				MESSAGE_END();

				m_flFlagStatusTime = gpGlobals->time + 0.1;

				pPlayer->m_bHasFlag = FALSE;
			}

			// drop any runes the player has
			CBaseEntity *pRune;
			char * runeName;

			switch ( pPlayer->m_iRuneStatus )
			{
				case ITEM_RUNE1_FLAG:

					pRune = CBaseEntity::Create( "item_rune1", pPlayer->pev->origin, pPlayer->pev->angles, NULL );
					
					pRune->pev->velocity = pPlayer->pev->velocity * 1.5; 
					pRune->pev->angles.x = 0;
					((CResistRune*)pRune)->dropped = true;

					runeName = "ResistRune";
						
					break;

				case ITEM_RUNE2_FLAG:

					pRune = CBaseEntity::Create( "item_rune2", pPlayer->pev->origin, pPlayer->pev->angles, NULL );

					pRune->pev->velocity = pPlayer->pev->velocity * 1.5; 
					pRune->pev->angles.x = 0;
					((CStrengthRune*)pRune)->dropped = true;

					runeName = "StrengthRune";
			
					break;

				case ITEM_RUNE3_FLAG:
			
					pRune = CBaseEntity::Create( "item_rune3", pPlayer->pev->origin, pPlayer->pev->angles, NULL );

					pRune->pev->velocity = pPlayer->pev->velocity * 1.5; 
					pRune->pev->angles.x = 0;
					((CHasteRune*)pRune)->dropped = true;

					runeName = "HasteRune";
			
					break;

				case ITEM_RUNE4_FLAG:
			
					pRune = CBaseEntity::Create( "item_rune4", pPlayer->pev->origin, pPlayer->pev->angles, NULL );

					pRune->pev->velocity = pPlayer->pev->velocity * 1.5; 
					pRune->pev->angles.x = 0;
					((CRegenRune*)pRune)->dropped = true;

					runeName = "RegenRune";
				
					break;

				default:

					runeName = "Unknown";

					break;
			}

			if ( pPlayer->m_iRuneStatus )
			{
				pPlayer->m_iRuneStatus = 0;

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_%s\"\n", 
					STRING(pPlayer->pev->netname),
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					pPlayer->m_szTeamName,
					runeName );
			}

			FireTargets( "game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0 );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" disconnected\n",  
				STRING( pPlayer->pev->netname ), 
				GETPLAYERUSERID( pPlayer->edict() ),
				GETPLAYERAUTHID( pPlayer->edict() ),
				GetTeamName( pPlayer->pev->team ) );

			pPlayer->RemoveAllItems( TRUE );// destroy all of the players weapons and items
		}
	}
}

void CThreeWave :: PlayerThink( CBasePlayer *pPlayer )
{
	if ( g_fGameOver )
	{
		// check for button presses
		if ( pPlayer->m_afButtonPressed & ( IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP ) )
			m_iEndIntermissionButtonHit = TRUE;

		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}

	if ( pPlayer->pFlagCarrierKiller )
	{
		if ( pPlayer->m_flFlagCarrierKillTime <= gpGlobals->time )
			pPlayer->pFlagCarrierKiller = NULL;
	}

	if ( pPlayer->pFlagReturner )
	{
		if ( pPlayer->m_flFlagReturnTime <= gpGlobals->time )
			pPlayer->pFlagReturner = NULL;
	}

	if ( pPlayer->pCarrierHurter )
	{
		if ( pPlayer->m_flCarrierHurtTime <= gpGlobals->time )
			 pPlayer->pCarrierHurter = NULL;
	}

	if ( pPlayer->m_iRuneStatus == ITEM_RUNE4_FLAG) 
	{
		if ( pPlayer->m_flRegenTime <= gpGlobals->time) 
		{
			

			if ( pPlayer->pev->health < 150 ) 
			{
				pPlayer->pev->health += 5;

				if ( pPlayer->pev->health > 150)
					pPlayer->pev->health = 150;

				pPlayer->m_flRegenTime = gpGlobals->time + 1;
				
				EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "rune/rune4.wav", 1, ATTN_NORM);
			}
			if ( pPlayer->pev->armorvalue < 150 && pPlayer->pev->armorvalue )
			{
				pPlayer->pev->armorvalue += 5;

				if ( pPlayer->pev->armorvalue > 150)
					pPlayer->pev->armorvalue = 150;

				pPlayer->m_flRegenTime = gpGlobals->time + 1;

				EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "rune/rune4.wav", 1, ATTN_NORM);
			}
		}
	}

	if ( pPlayer->m_bOn_Hook ) 
		pPlayer->Service_Grapple();

	if ( pPlayer->m_flFlagStatusTime && pPlayer->m_flFlagStatusTime <= gpGlobals->time )
		GetFlagStatus( pPlayer );
}

//=========================================================
//=========================================================
void CThreeWave :: PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
	CBasePlayer *pk = NULL; 
	
	if ( pKiller )
	{
		CBaseEntity *pTemp =  CBaseEntity::Instance( pKiller );

		if ( pTemp->IsPlayer() )
			pk = (CBasePlayer*)pTemp;
	}
	
	//Only award a bonus if the Flag carrier had the flag for more than 2 secs
	//Prevents from people waiting for the flag carrier to grab the flag and then killing him
	//Instead of actually defending the flag.
	if ( pVictim->m_bHasFlag  )
    {
		if ( pk )
		{
			if ( pVictim->pev->team != pk->pev->team )
			{
				if ( pVictim->m_flCarrierPickupTime <= gpGlobals->time )
					pk->AddPoints( TEAM_CAPTURE_FRAG_CARRIER_BONUS, TRUE );

				if ( pk->pev->team == RED )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pk->pev->netname ) );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " fragged " );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "BLUE" );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier!\n" );

					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Killed_Enemy_Flag_Carrier\"\n",  
						STRING( pk->pev->netname ), 
						GETPLAYERUSERID( pk->edict() ),
						GETPLAYERAUTHID( pk->edict() ),
						GetTeamName( pk->pev->team ) );

					if ( iBlueFlagStatus == BLUE_FLAG_STOLEN )
					{
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
														
							if ( pTeamMate )
								{
									if ( pTeamMate->m_bHasFlag )
									{
										pTeamMate->pFlagCarrierKiller = pk;
										pTeamMate->m_flFlagCarrierKillTime = gpGlobals->time + TEAM_CAPTURE_FRAG_CARRIER_ASSIST_TIMEOUT;
									}
								}
						}
					}
				}

				if ( pk->pev->team == BLUE )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pk->pev->netname ) );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " fragged " );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED" );
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier!\n" );

					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Killed_Enemy_Flag_Carrier\"\n",  
						STRING( pk->pev->netname ), 
						GETPLAYERUSERID( pk->edict() ),
						GETPLAYERAUTHID( pk->edict() ),
						GetTeamName( pk->pev->team ) );

					if ( iRedFlagStatus == RED_FLAG_STOLEN )
					{
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
														
							if ( pTeamMate )
								{
									if ( pTeamMate->m_bHasFlag )
									{
										pTeamMate->pFlagCarrierKiller = pk;
										pTeamMate->m_flFlagCarrierKillTime = gpGlobals->time + TEAM_CAPTURE_FRAG_CARRIER_ASSIST_TIMEOUT;
									}
								}
						}
					}
				}
			}
		}


        CBaseEntity *pEnt; 

		//We have the BLUE flag, Spawn it
        if ( pVictim->pev->team == RED )
		{
            pEnt = CBaseEntity::Create( "item_flag_team2", pVictim->pev->origin, pVictim->pev->angles, pVictim->edict() );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_Blue_Flag\"\n",  
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				GetTeamName( pVictim->pev->team ) );
		}
		else if ( pVictim->pev->team == BLUE )
		{
            pEnt = CBaseEntity::Create( "item_flag_team1", pVictim->pev->origin, pVictim->pev->angles, pVictim->edict() );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_Red_Flag\"\n",  
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				GetTeamName( pVictim->pev->team ) );
		}
       
        pEnt->pev->velocity = pVictim->pev->velocity * 1.2; 
		pEnt->pev->angles.x = 0;

        CItemFlag *pFlag = (CItemFlag *)pEnt; 
        pFlag->Dropped = TRUE; 

		PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
		pVictim->edict(), g_usCarried, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
		0.0, 0.0, pVictim->entindex(), pVictim->pev->team, 1, 0 );

		pFlag->m_flDroppedTime = gpGlobals->time + TEAM_CAPTURE_FLAG_RETURN_TIME;

        MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
			if ( pVictim->pev->team == RED )
				WRITE_BYTE( BLUE_FLAG_LOST );
			else if ( pVictim->pev->team == BLUE )
				WRITE_BYTE( RED_FLAG_LOST );
	
			WRITE_STRING( STRING(pVictim->pev->netname) );
		MESSAGE_END();

		pVictim->m_bHasFlag = FALSE;

		m_flFlagStatusTime = gpGlobals->time + 0.1;
	}
	else
	{
		if ( pk )
		{
				if ( pk->pev->team == RED )
				{
					if ( iBlueFlagStatus == BLUE_FLAG_STOLEN )
					{
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
														
							if ( pTeamMate && pTeamMate != pk )
								{
									if ( pTeamMate->pev->team == pk->pev->team )
									{
										if ( pTeamMate->m_bHasFlag )
										{
											if ( pTeamMate->pCarrierHurter )
											{
												if ( pTeamMate->pCarrierHurter == pVictim )
												{
													if ( pTeamMate->m_flCarrierHurtTime > gpGlobals->time )
													{
														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pk->pev->netname ) );
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends ");
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED" );
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier against an agressive enemy\n");
												
													    pk->AddPoints( TEAM_CAPTURE_CARRIER_DANGER_PROTECT_BONUS, TRUE );
													}
												}
											}
										}
									}
								}
						}
					}
				}
		
				if ( pk->pev->team == BLUE )
				{
					if ( iRedFlagStatus == RED_FLAG_STOLEN )
					{
						for ( int i = 1; i <= gpGlobals->maxClients; i++ )
						{
							CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
														
							if ( pTeamMate && pTeamMate != pk )
								{
									if ( pTeamMate->pev->team == pk->pev->team )
									{
										if ( pTeamMate->m_bHasFlag )
										{
											if ( pTeamMate->pCarrierHurter )
											{
												if ( pTeamMate->pCarrierHurter == pVictim )
												{
													if ( pTeamMate->m_flCarrierHurtTime > gpGlobals->time )
													{
														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pk->pev->netname ) );
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends ");
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "BLUE" );
 														UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier against an agressive enemy\n");

													    pk->AddPoints( TEAM_CAPTURE_CARRIER_DANGER_PROTECT_BONUS, TRUE );
													}
												}
											}
										}
									}
								}
						}
					}
				}
		}
		
	}

	// Find if this guy is near our flag or our flag carrier
	CBaseEntity *ent = NULL;
	float Dist;

	if ( pk )
	{
		if ( pk->pev->team == RED )
		{
			while((ent = UTIL_FindEntityByClassname( ent, "item_flag_team1")) != NULL)
			{
				//Do not defend a invisible flag
				if ( ent->pev->effects & EF_NODRAW )
					break;

				Dist = (pk->pev->origin - ent->pev->origin).Length();

				if ( Dist <= TEAM_CAPTURE_TARGET_PROTECT_RADIUS )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING ( pk->pev->netname ));
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends the ");
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED");
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " flag\n");

					pk->AddPoints( TEAM_CAPTURE_FLAG_DEFENSE_BONUS, TRUE );
					break;
				}
			}

			if ( iBlueFlagStatus == BLUE_FLAG_STOLEN )
				{
					for ( int i = 1; i <= gpGlobals->maxClients; i++ )
					{
						CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
														
						if ( pTeamMate && pTeamMate != pk )
							{
								if ( pTeamMate->pev->team == pk->pev->team )
								{
									if ( pTeamMate->m_bHasFlag )
									{
										Dist = (pk->pev->origin - pTeamMate->pev->origin).Length();

										if ( Dist <= TEAM_CAPTURE_TARGET_PROTECT_RADIUS )
										{
											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING ( pk->pev->netname ));
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends ");
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED");
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier\n");

											pk->AddPoints( TEAM_CAPTURE_CARRIER_PROTECT_BONUS, TRUE );
										}
									}
								}
							}
					}
				}

		}
		else if ( pk->pev->team == BLUE )
		{
			while((ent = UTIL_FindEntityByClassname( ent, "item_flag_team2")) != NULL)
			{
				//Do not defend a invisible flag
				if ( ent->pev->effects & EF_NODRAW )
					break;

				Dist = (pk->pev->origin - ent->pev->origin).Length();

				if ( Dist <= TEAM_CAPTURE_TARGET_PROTECT_RADIUS )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING ( pk->pev->netname ));
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends the ");
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED");
 					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " flag\n");

					pk->AddPoints( TEAM_CAPTURE_FLAG_DEFENSE_BONUS, TRUE );
					break;
				}
			}

			if ( iRedFlagStatus == RED_FLAG_STOLEN )
				{
					for ( int i = 1; i <= gpGlobals->maxClients; i++ )
					{
						CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
													
						if ( pTeamMate && pTeamMate != pk )
							{
								if ( pTeamMate->pev->team == pk->pev->team )
								{
									if ( pTeamMate->m_bHasFlag )
									{
										Dist = (pk->pev->origin - pTeamMate->pev->origin).Length();

										if ( Dist <= TEAM_CAPTURE_TARGET_PROTECT_RADIUS )
										{
											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING ( pk->pev->netname ));
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " defends ");
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "RED");
 											UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "'s flag carrier\n");

											pk->AddPoints( TEAM_CAPTURE_CARRIER_PROTECT_BONUS, TRUE );
										}
									}
								}
							}
					}
				}

		}
	}

	CBaseEntity *pRune;
	char * runeName;

	switch ( pVictim->m_iRuneStatus )
	{
		case ITEM_RUNE1_FLAG:

			pRune = CBaseEntity::Create( "item_rune1", pVictim->pev->origin, pVictim->pev->angles, NULL );
			
			pRune->pev->velocity = pVictim->pev->velocity * 1.5; 
			pRune->pev->angles.x = 0;
			((CResistRune*)pRune)->dropped = true;

			runeName = "ResistRune";
				
			break;

		case ITEM_RUNE2_FLAG:

			pRune = CBaseEntity::Create( "item_rune2", pVictim->pev->origin, pVictim->pev->angles, NULL );

			pRune->pev->velocity = pVictim->pev->velocity * 1.5; 
			pRune->pev->angles.x = 0;
			((CStrengthRune*)pRune)->dropped = true;

			runeName = "StrengthRune";
	
			break;

		case ITEM_RUNE3_FLAG:
	
			pRune = CBaseEntity::Create( "item_rune3", pVictim->pev->origin, pVictim->pev->angles, NULL );

			pRune->pev->velocity = pVictim->pev->velocity * 1.5; 
			pRune->pev->angles.x = 0;
			((CHasteRune*)pRune)->dropped = true;

			runeName = "HasteRune";
	
			break;

		case ITEM_RUNE4_FLAG:
	
			pRune = CBaseEntity::Create( "item_rune4", pVictim->pev->origin, pVictim->pev->angles, NULL );

			pRune->pev->velocity = pVictim->pev->velocity * 1.5; 
			pRune->pev->angles.x = 0;
			((CRegenRune*)pRune)->dropped = true;

			runeName = "RegenRune";
		
			break;

		default:

			runeName = "Unknown";

			break;
	}

	if ( pVictim->m_iRuneStatus )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_%s\"\n", 
			STRING(pVictim->pev->netname),
			GETPLAYERUSERID( pVictim->edict() ),
			GETPLAYERAUTHID( pVictim->edict() ),
			pVictim->m_szTeamName,
			runeName );
	}

	if ( pVictim->m_ppHook )
	   (( CGrapple *)pVictim->m_ppHook)->Reset_Grapple();

	pVictim->m_iRuneStatus = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pVictim->pev);
		WRITE_BYTE( pVictim->m_iRuneStatus );
	MESSAGE_END();

	if ( !m_DisableDeathPenalty )
	{
		CHalfLifeMultiplay::PlayerKilled( pVictim, pKiller, pInflictor );
		RecountTeams();
	}
}


//=========================================================
// IsTeamplay
//=========================================================
BOOL CThreeWave::IsTeamplay( void )
{
	return TRUE;
}

BOOL CThreeWave::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if ( pAttacker && PlayerRelationship( pPlayer, pAttacker ) == GR_TEAMMATE )
	{
		// my teammate hit me.
		if ( (CVAR_GET_FLOAT("mp_friendlyfire") == 0) && (pAttacker != pPlayer) )
		{
			// friendly fire is off, and this hit came from someone other than myself,  then don't get hurt
			return FALSE;
		}
	}

	return CHalfLifeMultiplay::FPlayerCanTakeDamage( pPlayer, pAttacker );
}

//=========================================================
//=========================================================
int CThreeWave::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() )
		return GR_NOTTEAMMATE;

	//As simple as this
	if ( pPlayer->pev->team == pTarget->pev->team )
	{
		return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}

//=========================================================
//=========================================================
BOOL CThreeWave::ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target )
{
	// always autoaim, unless target is a teammate
	CBaseEntity *pTgt = CBaseEntity::Instance( target );
	if ( pTgt && pTgt->IsPlayer() )
	{
		if ( PlayerRelationship( pPlayer, pTgt ) == GR_TEAMMATE )
			return FALSE; // don't autoaim at teammates
	}

	return CHalfLifeMultiplay::ShouldAutoAim( pPlayer, target );
}

//=========================================================
//=========================================================
int CThreeWave::IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	if ( !pKilled )
		return 0;

	if ( !pAttacker )
		return 1;

	if ( pAttacker != pKilled && PlayerRelationship( pAttacker, pKilled ) == GR_TEAMMATE )
		return -1;

	return 1;
}

//=========================================================
//=========================================================
const char *CThreeWave::GetTeamID( CBaseEntity *pEntity )
{
	if ( pEntity == NULL || pEntity->pev == NULL )
		return "";

	// return their team name
	return pEntity->TeamID();
}


int CThreeWave::GetTeamIndex( const char *pTeamName )
{
	if ( pTeamName && *pTeamName != 0 )
	{
		// try to find existing team
		for ( int tm = 0; tm < num_teams; tm++ )
		{
			if ( !stricmp( team_names[tm], pTeamName ) )
				return tm;
		}
	}
	
	return -1;	// No match
}


const char *CThreeWave::GetIndexedTeamName( int teamIndex )
{
	if ( teamIndex < 0 || teamIndex >= num_teams )
		return "";

	return team_names[ teamIndex ];
}


BOOL CThreeWave::IsValidTeam( const char *pTeamName ) 
{
	if ( !m_teamLimit )	// Any team is valid if the teamlist isn't set
		return TRUE;

	return ( GetTeamIndex( pTeamName ) != -1 ) ? TRUE : FALSE;
}


void CThreeWave::GetFlagStatus( CBasePlayer *pPlayer )
{
	
	CBaseEntity *pFlag = NULL;
    int iFoundCount = 0;
	int iDropped = 0;

	while((pFlag = UTIL_FindEntityByClassname( pFlag, "carried_flag_team1")) != NULL)
	{
		if ( pFlag && !FBitSet( pFlag->pev->flags, FL_KILLME) )
			iFoundCount++;
	}

	if ( iFoundCount >= 1 )
		iRedFlagStatus = RED_FLAG_STOLEN;

	if ( !iFoundCount )
	{
		while((pFlag = UTIL_FindEntityByClassname( pFlag, "item_flag_team1")) != NULL)
		{
			if ( pFlag )
			{
				if ( ((CItemFlag *)pFlag)->Dropped )
					iDropped++;

				iFoundCount++;
			}
		}
			
		if ( iFoundCount > 1 && iDropped == 1 )
			iRedFlagStatus = RED_FLAG_DROPPED;
		else if ( iFoundCount >= 1 && iDropped == 0 )
			iRedFlagStatus = RED_FLAG_ATBASE;
	}

	iDropped = iFoundCount = 0;

	while((pFlag = UTIL_FindEntityByClassname( pFlag, "carried_flag_team2")) != NULL)
	{
		if ( pFlag && !FBitSet( pFlag->pev->flags, FL_KILLME) )
			iFoundCount++;
	}

	if ( iFoundCount >= 1 )
		iBlueFlagStatus = BLUE_FLAG_STOLEN;

	if ( !iFoundCount )
	{

		while((pFlag = UTIL_FindEntityByClassname( pFlag, "item_flag_team2")) != NULL)
		{
			if ( pFlag )
			{
				if ( ((CItemFlag *)pFlag)->Dropped )
					iDropped++;

				iFoundCount++;
			}
		}
			
		if ( iFoundCount > 1 && iDropped == 1 )
			iBlueFlagStatus = BLUE_FLAG_DROPPED;
		else if ( iFoundCount >= 1 && iDropped == 0 )
			iBlueFlagStatus = BLUE_FLAG_ATBASE;
	}

	if ( pPlayer )
	{
		if ( pPlayer->pev->team == 0 )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgFlagStatus, NULL, pPlayer->edict() );
				WRITE_BYTE( 0 );  
				WRITE_BYTE( iRedFlagStatus );
				WRITE_BYTE( iBlueFlagStatus );
				WRITE_BYTE( iRedTeamScore );
				WRITE_BYTE( iBlueTeamScore );
			MESSAGE_END();
		}
		else
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgFlagStatus, NULL, pPlayer->edict() );
				WRITE_BYTE( 1 );  
				WRITE_BYTE( iRedFlagStatus );
				WRITE_BYTE( iBlueFlagStatus );
				WRITE_BYTE( iRedTeamScore );
				WRITE_BYTE( iBlueTeamScore );
			MESSAGE_END();
		}

		pPlayer->m_flFlagStatusTime = 0.0;
	}
	else
	{
		MESSAGE_BEGIN( MSG_ALL, gmsgFlagStatus, NULL );
				WRITE_BYTE( 1 );  
				WRITE_BYTE( iRedFlagStatus );
				WRITE_BYTE( iBlueFlagStatus );
				WRITE_BYTE( iRedTeamScore );
				WRITE_BYTE( iBlueTeamScore );
		MESSAGE_END();

		m_flFlagStatusTime = 0.0;
	}

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *plr = UTIL_PlayerByIndex( i );
		if ( plr )
		{
			if ( ((CBasePlayer *)plr)->m_bHasFlag )
			{
				MESSAGE_BEGIN( MSG_ALL, gmsgFlagCarrier, NULL );
					WRITE_BYTE( plr->entindex() );
					WRITE_BYTE( 1 );
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN( MSG_ALL, gmsgFlagCarrier, NULL );
					WRITE_BYTE( plr->entindex() );
					WRITE_BYTE( 0 );
				MESSAGE_END();
			}
		}
	}
}


//=========================================================
//=========================================================
void CThreeWave::RecountTeams( void )
{
	char	*pName;
	char	teamlist[TEAMPLAY_TEAMLISTLENGTH];

	// loop through all teams, recounting everything
	num_teams = 0;

	// Copy all of the teams from the teamlist
	// make a copy because strtok is destructive
	strcpy( teamlist, m_szTeamList );
	pName = teamlist;
	pName = strtok( pName, ";" );
	while ( pName != NULL && *pName )
	{
		if ( GetTeamIndex( pName ) < 0 )
		{
			strcpy( team_names[num_teams], pName );
			num_teams++;
		}
		pName = strtok( NULL, ";" );
	}

	if ( num_teams < 2 )
	{
		num_teams = 0;
		m_teamLimit = FALSE;
	}

	// Sanity check
	memset( team_scores, 0, sizeof(team_scores) );

	// loop through all clients
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *plr = UTIL_PlayerByIndex( i );

		if ( plr )
		{
			const char *pTeamName = plr->TeamID();
			// try add to existing team
			int tm = GetTeamIndex( pTeamName );
			
			if ( tm < 0 ) // no team match found
			{ 
				if ( !m_teamLimit )
				{
					// add to new team
					tm = num_teams;
					num_teams++;
					team_scores[tm] = 0;
					strncpy( team_names[tm], pTeamName, MAX_TEAMNAME_LENGTH );
				}
			}

			if ( tm >= 0 )
			{
				team_scores[tm] += plr->pev->frags;
			}
		}
	}
}

/*****************************************************
******************************************************
                THREEWAVE CTF FLAG CODE
******************************************************
*****************************************************/

enum Flag_Anims
{
	ON_GROUND = 0,
	NOT_CARRIED,
	CARRIED,
	WAVE_IDLE,
	FLAG_POSITION
}; 


void CItemFlag::Spawn ( void )
{
    Precache( );
    SET_MODEL(ENT(pev), "models/flag.mdl"); 

    pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;
    UTIL_SetOrigin( pev, pev->origin );
    UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16)); 

    SetThink( &CItemFlag::FlagThink );
    SetTouch( &CItemFlag::FlagTouch ); 

    pev->nextthink = gpGlobals->time + 0.3; 

	//Set the Skin based on the team.
    pev->skin = pev->team;
   
    Dropped = FALSE; 
	m_flDroppedTime = 0.0;

    pev->sequence = NOT_CARRIED;
    pev->framerate = 1.0; 
   
   // if ( !DROP_TO_FLOOR(ENT(pev)) )
   //       ResetFlag( pev->team );
} 

void CItemFlag::FlagTouch ( CBaseEntity *pToucher  )
{
	if ( !pToucher )
		return;

	if ( !pToucher->IsPlayer() )
		return;

	if ( FBitSet( pev->effects, EF_NODRAW ) )
		return;
	
	if ( pToucher->pev->health <= 0 )
		 return;

	if ( pToucher->pev->team == 0 )
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pToucher;

	//Same team as the flag
	if ( pev->team == pToucher->pev->team )
	{
		//Flag is dropped, let's return it
		if ( Dropped )
		{
			Dropped = FALSE;
	
			pPlayer->AddPoints( TEAM_CAPTURE_RECOVERY_BONUS, TRUE );

			if ( pPlayer->pev->team == RED )
			{

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Returned_Red_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );

				if ( ((CThreeWave *) g_pGameRules)->iBlueFlagStatus == BLUE_FLAG_STOLEN )
				{
					for ( int i = 1; i <= gpGlobals->maxClients; i++ )
					{
						CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
													
						if ( pTeamMate )
						{
							if ( pTeamMate->m_bHasFlag )
							{
								pTeamMate->pFlagReturner = pPlayer;
								pTeamMate->m_flFlagReturnTime = gpGlobals->time + TEAM_CAPTURE_RETURN_FLAG_ASSIST_TIMEOUT;
							}
						}
					}
				}
			}

			if ( pPlayer->pev->team == BLUE )
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Returned_Blue_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );

				if ( ((CThreeWave *) g_pGameRules)->iRedFlagStatus == RED_FLAG_STOLEN )
				{
					for ( int i = 1; i <= gpGlobals->maxClients; i++ )
					{
						CBasePlayer *pTeamMate = (CBasePlayer *)UTIL_PlayerByIndex( i );
													
						if ( pTeamMate )
						{
							if ( pTeamMate->m_bHasFlag )
							{
								pTeamMate->pFlagReturner = pPlayer;
								pTeamMate->m_flFlagReturnTime = gpGlobals->time + TEAM_CAPTURE_RETURN_FLAG_ASSIST_TIMEOUT;
							}
						}
					}
				}
			}

			//Back at home!
			ResetFlag( pev->team );
            
			MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
		
				if ( pev->team == RED )
					WRITE_BYTE( RED_FLAG_RETURNED_PLAYER );
				else if ( pev->team == BLUE )
					WRITE_BYTE( BLUE_FLAG_RETURNED_PLAYER );
	
				WRITE_STRING( STRING(pToucher->pev->netname) );
			MESSAGE_END();

			//Remove this one
            UTIL_Remove( this );
		
			return;
		}
		//Not Dropped, means it's the one in our base
		else if ( !Dropped )
		{
			//We have the enemy flag!
			//Capture it!
			if ( pPlayer->m_bHasFlag )
			{
				if ( pev->team == RED )
					Capture( pPlayer, BLUE );
				else if ( pev->team == BLUE )
					Capture( pPlayer, RED );

				PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
				pPlayer->edict(), g_usCarried, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
				0.0, 0.0, pPlayer->entindex(), pPlayer->pev->team, 1, 0 );


				return;
			}
		}
	}
	else 
	{
		if ( Dropped )
		{
			MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
			
				if ( pev->team == RED )
					WRITE_BYTE( RED_FLAG_STOLEN );
				else if ( pev->team == BLUE )
					WRITE_BYTE( BLUE_FLAG_STOLEN );

				WRITE_STRING( STRING(pToucher->pev->netname) );

			MESSAGE_END();

			pPlayer->m_bHasFlag = TRUE;

			CBaseEntity *pEnt = NULL;

			if ( pev->team == RED )
			{
				pEnt = CBaseEntity::Create( "carried_flag_team1", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Picked_Up_Red_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
			}
			else if ( pev->team == BLUE )
			{
				pEnt = CBaseEntity::Create( "carried_flag_team2", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Picked_Up_Blue_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
			}

			CCarriedFlag *pCarriedFlag = (CCarriedFlag *)pEnt;
			pCarriedFlag->Owner = pPlayer;
			
			PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
			pPlayer->edict(), g_usCarried, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
			0.0, 0.0, pPlayer->entindex(), pPlayer->pev->team, 0, 0 );



			UTIL_Remove( this );
		}
		else
		{
			pev->effects |= EF_NODRAW;
			
			MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
			
				if ( pev->team == RED )
					WRITE_BYTE( RED_FLAG_STOLEN );
				else if ( pev->team == BLUE )
					WRITE_BYTE( BLUE_FLAG_STOLEN );

				WRITE_STRING( STRING(pToucher->pev->netname) );

			MESSAGE_END();

			pPlayer->m_bHasFlag = TRUE;
			pPlayer->m_flCarrierPickupTime = gpGlobals->time + TEAM_CAPTURE_CARRIER_FLAG_SINCE_TIMEOUT;

			CBaseEntity *pEnt = NULL;

			if ( pev->team == RED )
			{
				pEnt = CBaseEntity::Create( "carried_flag_team1", pev->origin, pev->angles, pPlayer->edict() );

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Stole_Red_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
			}
			else if ( pev->team == BLUE )
			{
				pEnt = CBaseEntity::Create( "carried_flag_team2", pev->origin, pev->angles, pPlayer->edict() );

				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Stole_Blue_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
			}

			CCarriedFlag *pCarriedFlag = (CCarriedFlag *)pEnt;
			pCarriedFlag->Owner = pPlayer;

			PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
			pPlayer->edict(), g_usCarried, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
			0.0, 0.0, pPlayer->entindex(), pPlayer->pev->team, 0, 0 );
		}
		
		((CThreeWave *) g_pGameRules)->m_flFlagStatusTime = gpGlobals->time + 0.1;
	}
}

void CItemFlag::Capture(CBasePlayer *pPlayer, int iTeam )
{
	CBaseEntity *pFlag1 = NULL; 

	MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
	
		if ( iTeam == RED )
			WRITE_BYTE( RED_FLAG_CAPTURED );
		else if ( iTeam == BLUE )
			WRITE_BYTE( BLUE_FLAG_CAPTURED );

		WRITE_STRING( STRING( pPlayer->pev->netname) );

	MESSAGE_END();

	if ( pPlayer->pFlagCarrierKiller )
	{
		if ( pPlayer->m_flFlagCarrierKillTime > gpGlobals->time )
		{
			UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pPlayer->pFlagCarrierKiller->pev->netname ) );
			UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " gets an assist for fragging the flag carrier!\n");

			pPlayer->pFlagCarrierKiller->AddPoints( TEAM_CAPTURE_FRAG_CARRIER_ASSIST_BONUS, TRUE );
			pPlayer->pFlagCarrierKiller = NULL;
			pPlayer->m_flFlagCarrierKillTime = 0.0;
		}
	}

	if ( pPlayer->pFlagReturner )
	{
		if ( pPlayer->m_flFlagReturnTime > gpGlobals->time )
		{
			UTIL_ClientPrintAll( HUD_PRINTNOTIFY, STRING( pPlayer->pFlagReturner->pev->netname ) );
			UTIL_ClientPrintAll( HUD_PRINTNOTIFY, " gets an assist for returning his flag!\n");

			pPlayer->pFlagReturner->AddPoints( TEAM_CAPTURE_RETURN_FLAG_ASSIST_BONUS, TRUE );
			pPlayer->pFlagReturner = NULL;
			pPlayer->m_flFlagReturnTime = 0.0;
		}
	}

	if ( iTeam != pPlayer->pev->team )
	{
		if ( iTeam == RED )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Captured_Red_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Captured_Blue_Flag\"\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GetTeamName( pPlayer->pev->team ) );
		}
	}

	if ( iTeam == RED )
	{
		((CThreeWave *) g_pGameRules)->iBlueTeamScore++;

		while((pFlag1 = UTIL_FindEntityByClassname( pFlag1, "carried_flag_team1")) != NULL)
		{
			if ( pFlag1 )
				UTIL_Remove( pFlag1 );
		}
	}
	else if ( iTeam == BLUE )
	{
		((CThreeWave *) g_pGameRules)->iRedTeamScore++;

		while((pFlag1 = UTIL_FindEntityByClassname( pFlag1, "carried_flag_team2")) != NULL)
		{
			if ( pFlag1 )
				UTIL_Remove( pFlag1 );
		}
	}

	pPlayer->m_bHasFlag = FALSE;

	pPlayer->AddPoints( TEAM_CAPTURE_CAPTURE_BONUS, TRUE );

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pTeamMate = UTIL_PlayerByIndex( i );
										
			if ( pTeamMate )
				{
					if ( pTeamMate->pev->team == pPlayer->pev->team )
					     pTeamMate->AddPoints( TEAM_CAPTURE_TEAM_BONUS, TRUE );
				}
		}

    ResetFlag( iTeam );
} 

void CItemFlag::Materialize( void )
{
    if ( pev->effects & EF_NODRAW )
    {        
        pev->effects &= ~EF_NODRAW;
        pev->effects |= EF_MUZZLEFLASH;
    } 

	PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
	edict(), g_usFlagSpawn, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
	0.0, 0.0, pev->team, 0, 0, 0 );

	Dropped = FALSE;

    SetTouch( &CItemFlag::FlagTouch );
    SetThink( &CItemFlag::FlagThink );
} 


void CItemFlag::ResetFlag( int iTeam )
{
	CBaseEntity *pFlag1 = NULL; 

	if ( iTeam == BLUE )
	{
		while((pFlag1 = UTIL_FindEntityByClassname( pFlag1, "item_flag_team2")) != NULL)
		{
			CItemFlag *pFlag2 = (CItemFlag *)pFlag1;

			if ( pFlag2->Dropped )
				continue;

			if ( pFlag2->pev->effects & EF_NODRAW)
				 pFlag2->Materialize(); 
		}

	}
	else if ( iTeam == RED )
	{
		while((pFlag1 = UTIL_FindEntityByClassname( pFlag1, "item_flag_team1")) != NULL)
		{
			CItemFlag *pFlag2 = (CItemFlag *)pFlag1;
			
			if ( pFlag2->Dropped )
				continue;

			if ( pFlag2->pev->effects & EF_NODRAW)
				 pFlag2->Materialize(); 
		}
	}

	((CThreeWave *) g_pGameRules)->m_flFlagStatusTime = gpGlobals->time + 0.1;

} 

void CItemFlag::FlagThink( void )
{
   	if ( Dropped )
	{
		if ( m_flDroppedTime <= gpGlobals->time )
		{
			
			ResetFlag( pev->team );
			
			MESSAGE_BEGIN ( MSG_ALL, gmsgCTFMsgs, NULL );
		
				if ( pev->team == RED )
					WRITE_BYTE( RED_FLAG_RETURNED );
				else if ( pev->team == BLUE )
					WRITE_BYTE( BLUE_FLAG_RETURNED );
	
				WRITE_STRING( "" );
			MESSAGE_END();

			UTIL_Remove( this );
			return;
		}
	}

	//Using 0.2 just in case we might lag the server.
	pev->nextthink = gpGlobals->time + 0.2;
} 

void CItemFlag::Precache( void )
{
    PRECACHE_MODEL ("models/flag.mdl");
    PRECACHE_SOUND ("ctf/flagcap.wav");
    PRECACHE_SOUND ("ctf/flagtk.wav");
	PRECACHE_SOUND ("ctf/flagret.wav");
} 

class CItemFlagTeam1 : public CItemFlag
{
    void Spawn( void )
    {
        pev->team = RED;
        CItemFlag::Spawn( );
    }
}; 

class CItemFlagTeam2 : public CItemFlag
{
    void Spawn( void )
    {
        pev->team = BLUE;
        CItemFlag::Spawn( );
    }
}; 

LINK_ENTITY_TO_CLASS( item_flag_team1, CItemFlagTeam1 ); 
LINK_ENTITY_TO_CLASS( item_flag_team2, CItemFlagTeam2 ); 


void CCarriedFlag ::Spawn( )
{
    Precache( ); 

    SET_MODEL(ENT(pev), "models/flag.mdl");
    UTIL_SetOrigin( pev, pev->origin ); 

    pev->movetype = MOVETYPE_NONE;
    pev->solid = SOLID_NOT; 
  
    pev->effects |= EF_NODRAW; 

    pev->sequence = WAVE_IDLE;
    pev->framerate = 1.0; 

    if ( pev->team == RED )
        pev->skin = 1;
    else if ( pev->team == BLUE )
        pev->skin = 2; 

	m_iOwnerOldVel = 0;

    SetThink( &CCarriedFlag::FlagThink );
    pev->nextthink = gpGlobals->time + 0.1;
} 

void CCarriedFlag::Precache( )
{
    PRECACHE_MODEL ("models/flag.mdl");
} 

void CCarriedFlag::FlagThink( )
{
    //Make it visible
    pev->effects &= ~EF_NODRAW; 

    //And let if follow
    pev->aiment = ENT(Owner->pev);
    pev->movetype = MOVETYPE_FOLLOW; 

    //Remove if owner is death
    if (!Owner->IsAlive())
        UTIL_Remove( this ); 

    //If owner lost flag, remove
    if ( !Owner->m_bHasFlag )
         UTIL_Remove( this );
    else
    {
	    //If owners speed is low, go in idle mode
        if (Owner->pev->velocity.Length() <= 75 && pev->sequence != WAVE_IDLE)
        {
            pev->sequence = WAVE_IDLE;
        }
        //Else let the flag go wild
        else if (Owner->pev->velocity.Length() >= 75 && pev->sequence != CARRIED)
        {
            pev->sequence = CARRIED;
        }
        pev->frame += pev->framerate;
        if (pev->frame < 0.0 || pev->frame >= 256.0)
        {
            pev->frame -= (int)(pev->frame / 256.0) * 256.0;
        }
        pev->nextthink = gpGlobals->time + 0.1;
    }
} 

class CCarriedFlagTeam1 : public CCarriedFlag
{
    void Spawn( void )
    {
        pev->team = RED; 

        CCarriedFlag::Spawn( );
    }
}; 

class CCarriedFlagTeam2 : public CCarriedFlag
{
    void Spawn( void )
    {
        pev->team = BLUE;
 
        CCarriedFlag::Spawn( );
    }
}; 

LINK_ENTITY_TO_CLASS( carried_flag_team1, CCarriedFlagTeam1 ); 
LINK_ENTITY_TO_CLASS( carried_flag_team2, CCarriedFlagTeam2 ); 


/***************************************
****************************************
				RUNES
****************************************
***************************************/

/*----------------------------------------------------------------------
  The Rune Game modes

  Rune 1 - Earth Magic
	  resistance
  Rune 2 - Black Magic
	  strength
  Rune 3 - Hell Magic
	  haste
  Rune 4 - Elder Magic
	  regeneration

 ----------------------------------------------------------------------*/

BOOL IsRuneSpawnPointValid( CBaseEntity *pSpot )
{
	CBaseEntity *ent = NULL;
	
	while ( (ent = UTIL_FindEntityInSphere( ent, pSpot->pev->origin, 128 )) != NULL )
	{
		//Try not to spawn it near other runes.
		if ( !strcmp( STRING( ent->pev->classname ), "item_rune1")  || 
			 !strcmp( STRING( ent->pev->classname ), "item_rune2")  ||
			 !strcmp( STRING( ent->pev->classname ), "item_rune3")  ||
			 !strcmp( STRING( ent->pev->classname ), "item_rune4")  )
			return FALSE;
	}

	return TRUE;
}

edict_t *RuneSelectSpawnPoint( void )
{
	CBaseEntity *pSpot;
	
	pSpot = NULL;

	// Randomize the start spot
	for ( int i = RANDOM_LONG(1,5); i > 0; i-- )
		pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
	if ( !pSpot )  // skip over the null point
		pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );

	CBaseEntity *pFirstSpot = pSpot;

	do 
	{
		if ( pSpot )
		{
			if ( IsRuneSpawnPointValid( pSpot ) )
			{
				if ( pSpot->pev->origin == Vector( 0, 0, 0 ) )
				{
					pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
					continue;
				}
				// if so, go to pSpot
				goto ReturnSpot;
			}
		
		}
		// increment pSpot
		pSpot = UTIL_FindEntityByClassname( pSpot, "info_player_deathmatch" );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	if ( pSpot )
		goto ReturnSpot;
	
	// If startspot is set, (re)spawn there.
	if ( FStringNull( gpGlobals->startspot ) || !strlen(STRING(gpGlobals->startspot)))
	{
		pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");
		if ( pSpot )
			goto ReturnSpot;
	}
	else
	{
		pSpot = UTIL_FindEntityByTargetname( NULL, STRING(gpGlobals->startspot) );
		if ( pSpot )
			goto ReturnSpot;
	}

ReturnSpot:
	if ( !pSpot )
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}
	return pSpot->edict();
}

void VectorScale (const float *in, float scale, float *out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}

#define VectorSet(v, x, y, z)	(v[0]=(x), v[1]=(y), v[2]=(z))

void DropRune ( CBasePlayer *pPlayer )
{
	TraceResult tr;

	// do they even have a rune?
	if ( pPlayer->m_iRuneStatus == 0 )
		return;

	// Make Sure there's enough room to drop the rune here
	// This is so hacky ( the reason why we are doing this), and I hate it to death.
	UTIL_MakeVectors ( pPlayer->pev->v_angle );
	Vector vecSrc	= pPlayer->GetGunPosition( );
	Vector vecEnd	= vecSrc + gpGlobals->v_forward * 32;
	UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, human_hull, ENT( pPlayer->pev ), &tr );

	if (tr.flFraction != 1)
	{
		ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "Not enough room to drop the rune here." );
		return;
	}

	CBaseEntity *pRune = NULL;
	char * runeName;

	if ( pPlayer->m_iRuneStatus == ITEM_RUNE1_FLAG )
	{
		pRune = CBaseEntity::Create( "item_rune1", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );
		runeName = "ResistRune";

		if ( pRune )
			((CResistRune*)pRune)->dropped = true;
	}
	else if ( pPlayer->m_iRuneStatus == ITEM_RUNE2_FLAG )
	{
		pRune = CBaseEntity::Create( "item_rune2", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );
		runeName = "StrengthRune";

		if ( pRune )
			((CStrengthRune*)pRune)->dropped = true;
	}
	else if ( pPlayer->m_iRuneStatus == ITEM_RUNE3_FLAG )
	{
		pRune = CBaseEntity::Create( "item_rune3", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );
		runeName = "HasteRune";

		if ( pRune )
			((CHasteRune*)pRune)->dropped = true;
	}
	else if ( pPlayer->m_iRuneStatus == ITEM_RUNE4_FLAG )
	{
		pRune = CBaseEntity::Create( "item_rune4", pPlayer->pev->origin, pPlayer->pev->angles, pPlayer->edict() );
		runeName = "RegenRune";

		if ( pRune )
			((CRegenRune*)pRune)->dropped = true;
	}
	else
	{
		runeName = "Unknown";
	}

	if ( pPlayer->m_iRuneStatus == ITEM_RUNE3_FLAG )
		g_engfuncs.pfnSetClientMaxspeed( ENT( pPlayer->pev ), PLAYER_MAX_SPEED ); //Reset Haste player speed to normal

	pPlayer->m_iRuneStatus = 0;

	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Dropped_%s\"\n", 
		STRING(pPlayer->pev->netname),
		GETPLAYERUSERID( pPlayer->edict() ),
		GETPLAYERAUTHID( pPlayer->edict() ),
		pPlayer->m_szTeamName,
		runeName );

	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pPlayer->pev);
		WRITE_BYTE( pPlayer->m_iRuneStatus );
	MESSAGE_END();
}


void CResistRune::RuneTouch ( CBaseEntity *pOther )
{
	//No toucher?
	if ( !pOther )
		return;

	//Not a player?
	if ( !pOther->IsPlayer() )
		return;

	//DEAD?!
	if ( pOther->pev->health <= 0 )
		 return;

	//Spectating?
	if ( pOther->pev->movetype == MOVETYPE_NOCLIP )
		 return;
	
	//Only one per customer
	if ( ((CBasePlayer *)pOther)->m_iRuneStatus )
	{
		ClientPrint( pOther->pev, HUD_PRINTCENTER, "You already have a rune!\n" );
		return;
	}

	if ( !m_bTouchable )
		return;

	((CBasePlayer *)pOther)->m_iRuneStatus = m_iRuneFlag; //Add me the rune flag

	ClientPrint( pOther->pev, HUD_PRINTCENTER, "You got the rune of Resistance!\n" );

	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Found_ResistRune\"\n", 
		STRING(pOther->pev->netname),
		GETPLAYERUSERID( pOther->edict() ),
		GETPLAYERAUTHID( pOther->edict() ),
		((CBasePlayer *)pOther)->m_szTeamName );

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	//Update my client side rune hud thingy.
	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pOther->pev);
		WRITE_BYTE( ((CBasePlayer *)pOther)->m_iRuneStatus );
	MESSAGE_END();

	//And Remove this entity
	UTIL_Remove( this );
}


void CResistRune::RuneRespawn ( void )
{
	edict_t *pentSpawnSpot;
	vec3_t vOrigin;

	pentSpawnSpot = RuneSelectSpawnPoint();
	vOrigin = VARS(pentSpawnSpot)->origin;
	
	UTIL_SetOrigin( pev, vOrigin );

	if ( dropped )
		UTIL_LogPrintf( "\"<-1><><>\" triggered triggered \"Respawn_ResistRune\"\n" );
	    
	Spawn();
}

void CResistRune::MakeTouchable ( void )
{
	m_bTouchable = TRUE;
	pev->nextthink = gpGlobals->time + 120; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CResistRune::RuneRespawn );
}

void CResistRune::Spawn ( void )
{
	SET_MODEL( ENT(pev), "models/rune_resist.mdl");
   
	m_bTouchable = FALSE;

	m_iRuneFlag = ITEM_RUNE1_FLAG;

	dropped = false;

	pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

	vec3_t forward, right, up;

	UTIL_SetSize( pev, Vector(-15, -15, -15), Vector(15, 15, 15) ); 

	pev->angles.z = pev->angles.x = 0;
	pev->angles.y = RANDOM_LONG ( 0, 360 );

	//If we got an owner, it means we are either dropping the flag or diying and letting it go.
	if ( pev->owner )
	    g_engfuncs.pfnAngleVectors ( pev->owner->v.angles, forward, right, up );
	else
		g_engfuncs.pfnAngleVectors ( pev->angles, forward, right, up);

	UTIL_SetOrigin( pev, pev->origin );
	
	pev->velocity = ( forward * 400 ) + ( up * 200 );
	
	if ( pev->owner == NULL )
	{
		pev->origin.z += 16;
		pev->velocity.z = 300;
	}
	
	pev->owner = NULL;
	
	SetTouch( &CResistRune::RuneTouch );
	
	pev->nextthink = gpGlobals->time + 1; 
	SetThink ( &CResistRune::MakeTouchable );
}


LINK_ENTITY_TO_CLASS( item_rune1, CResistRune ); 


void CStrengthRune::MakeTouchable ( void )
{
	m_bTouchable = TRUE;
	pev->nextthink = gpGlobals->time + 120; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CStrengthRune::RuneRespawn );
}

void CStrengthRune::RuneTouch ( CBaseEntity *pOther )
{
	//No toucher?
	if ( !pOther )
		return;

	//Not a player?
	if ( !pOther->IsPlayer() )
		return;

	//DEAD?!
	if ( pOther->pev->health <= 0 )
		 return;

	//Spectating?
	if ( pOther->pev->movetype == MOVETYPE_NOCLIP )
		 return;
	
	//Only one per customer
	if ( ((CBasePlayer *)pOther)->m_iRuneStatus )
	{
		ClientPrint( pOther->pev, HUD_PRINTCENTER, "You already have a rune!\n" );
		return;
	}

	if ( !m_bTouchable )
		return;

	((CBasePlayer *)pOther)->m_iRuneStatus = m_iRuneFlag; //Add me the rune flag

	ClientPrint( pOther->pev, HUD_PRINTCENTER, "You got the rune of Strength!\n" );

	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Found_StrengthRune\"\n", 
		STRING(pOther->pev->netname),
		GETPLAYERUSERID( pOther->edict() ),
		GETPLAYERAUTHID( pOther->edict() ),
		((CBasePlayer *)pOther)->m_szTeamName );

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	//Update my client side rune hud thingy.
	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pOther->pev);
		WRITE_BYTE( ((CBasePlayer *)pOther)->m_iRuneStatus );
	MESSAGE_END();

	//And Remove this entity
	UTIL_Remove( this );
}

void CStrengthRune::RuneRespawn ( void )
{
	edict_t *pentSpawnSpot;
	vec3_t vOrigin;

	pentSpawnSpot = RuneSelectSpawnPoint();
	vOrigin = VARS(pentSpawnSpot)->origin;
	
	UTIL_SetOrigin( pev, vOrigin );

	if ( dropped )
		UTIL_LogPrintf( "\"<-1><><>\" triggered triggered \"Respawn_StrengthRune\"\n" );

	Spawn();
}


void CStrengthRune::Spawn ( void )
{
	SET_MODEL( ENT(pev), "models/rune_strength.mdl");
    
	m_bTouchable = FALSE;

	m_iRuneFlag = ITEM_RUNE2_FLAG;

	dropped = false;

	pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

	vec3_t forward, right, up;

	UTIL_SetSize( pev, Vector(-15, -15, -15), Vector(15, 15, 15) ); 

	pev->angles.z = pev->angles.x = 0;
	pev->angles.y = RANDOM_LONG ( 0, 360 );

	//If we got an owner, it means we are either dropping the flag or diying and letting it go.
	if ( pev->owner )
	    g_engfuncs.pfnAngleVectors ( pev->owner->v.angles, forward, right, up);
	else
		g_engfuncs.pfnAngleVectors ( pev->angles, forward, right, up);

	UTIL_SetOrigin( pev, pev->origin );
	
	pev->velocity = ( forward * 400 ) + ( up * 200 );
	
	if ( pev->owner == NULL )
	{
		pev->origin.z += 16;
		pev->velocity.z = 300;
	}
	
	pev->owner = NULL;
	
	SetTouch( &CStrengthRune::RuneTouch );

	pev->nextthink = gpGlobals->time + 1; 
	SetThink ( &CStrengthRune::MakeTouchable );
}


LINK_ENTITY_TO_CLASS( item_rune2, CStrengthRune ); 

void CHasteRune::MakeTouchable ( void )
{
	m_bTouchable = TRUE;
	pev->nextthink = gpGlobals->time + 120; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CHasteRune::RuneRespawn );
}


void CHasteRune::RuneTouch ( CBaseEntity *pOther )
{
	//No toucher?
	if ( !pOther )
		return;

	//Not a player?
	if ( !pOther->IsPlayer() )
		return;

	//DEAD?!
	if ( pOther->pev->health <= 0 )
		 return;

	//Spectating?
	if ( pOther->pev->movetype == MOVETYPE_NOCLIP )
		 return;
	
	//Only one per customer
	if ( ((CBasePlayer *)pOther)->m_iRuneStatus )
	{
		ClientPrint( pOther->pev, HUD_PRINTCENTER, "You already have a rune!\n" );
		return;
	}

	if ( !m_bTouchable )
		return;

	((CBasePlayer *)pOther)->m_iRuneStatus = m_iRuneFlag; //Add me the rune flag

	ClientPrint( pOther->pev, HUD_PRINTCENTER, "You got the rune of Haste!\n" );

	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Found_HasteRune\"\n", 
		STRING(pOther->pev->netname),
		GETPLAYERUSERID( pOther->edict() ),
		GETPLAYERAUTHID( pOther->edict() ),
		((CBasePlayer *)pOther)->m_szTeamName );

	g_engfuncs.pfnSetClientMaxspeed( ENT( pOther->pev ), ( PLAYER_MAX_SPEED * 1.25 ) ); //25% more speed

	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	//Update my client side rune hud thingy.
	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pOther->pev);
		WRITE_BYTE( ((CBasePlayer *)pOther)->m_iRuneStatus );
	MESSAGE_END();

	//And Remove this entity
	UTIL_Remove( this );
}

void CHasteRune::RuneRespawn ( void )
{
	edict_t *pentSpawnSpot;
	vec3_t vOrigin;

	pentSpawnSpot = RuneSelectSpawnPoint();
	vOrigin = VARS(pentSpawnSpot)->origin;
	
	UTIL_SetOrigin( pev, vOrigin );

	if ( dropped )
		UTIL_LogPrintf( "\"<-1><><>\" triggered triggered \"Respawn_HasteRune\"\n" );
    
	Spawn();
}


void CHasteRune::Spawn ( void )
{
	SET_MODEL( ENT(pev), "models/rune_haste.mdl");
 
	m_bTouchable = FALSE;

	m_iRuneFlag = ITEM_RUNE3_FLAG;

	dropped = false;

	pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

	vec3_t forward, right, up;

	UTIL_SetSize( pev, Vector(-15, -15, -15), Vector(15, 15, 15) ); 

	pev->angles.z = pev->angles.x = 0;
	pev->angles.y = RANDOM_LONG ( 0, 360 );

	//If we got an owner, it means we are either dropping the flag or diying and letting it go.
	if ( pev->owner )
	    g_engfuncs.pfnAngleVectors ( pev->owner->v.angles, forward, right, up);
	else
		g_engfuncs.pfnAngleVectors ( pev->angles, forward, right, up);

	UTIL_SetOrigin( pev, pev->origin );
	
	pev->velocity = ( forward * 400 ) + ( up * 200 );
	
	if ( pev->owner == NULL )
	{
		pev->origin.z += 16;
		pev->velocity.z = 300;
	}
	
	pev->owner = NULL;
	
	SetTouch( &CHasteRune::RuneTouch );

	pev->nextthink = gpGlobals->time + 1; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CHasteRune::MakeTouchable );
}


LINK_ENTITY_TO_CLASS( item_rune3, CHasteRune ); 


void CRegenRune::MakeTouchable ( void )
{
	m_bTouchable = TRUE;
	pev->nextthink = gpGlobals->time + 120; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CRegenRune::RuneRespawn );
}

void CRegenRune::RuneTouch ( CBaseEntity *pOther )
{
	//No toucher?
	if ( !pOther )
		return;

	//Not a player?
	if ( !pOther->IsPlayer() )
		return;

	//DEAD?!
	if ( pOther->pev->health <= 0 )
		 return;

	//Spectating?
	if ( pOther->pev->movetype == MOVETYPE_NOCLIP )
		 return;
	
	//Only one per customer
	if ( ((CBasePlayer *)pOther)->m_iRuneStatus )
	{
		ClientPrint( pOther->pev, HUD_PRINTCENTER, "You already have a rune!\n" );
		return;
	}

	if ( !m_bTouchable )
		return;

	((CBasePlayer *)pOther)->m_iRuneStatus = m_iRuneFlag; //Add me the rune flag

	ClientPrint( pOther->pev, HUD_PRINTCENTER, "You got the rune of Regeneration!\n" );

	UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"Found_RegenRune\"\n", 
		STRING(pOther->pev->netname),
		GETPLAYERUSERID( pOther->edict() ),
		GETPLAYERAUTHID( pOther->edict() ),
		((CBasePlayer *)pOther)->m_szTeamName );
	
	EMIT_SOUND( ENT(pev), CHAN_ITEM, "weapons/lock4.wav", 1, ATTN_NORM );

	//Update my client side rune hud thingy.
	MESSAGE_BEGIN( MSG_ONE, gmsgRuneStatus, NULL, pOther->pev);
		WRITE_BYTE( ((CBasePlayer *)pOther)->m_iRuneStatus );
	MESSAGE_END();

	//And Remove this entity
	UTIL_Remove( this );
}

void CRegenRune::RuneRespawn ( void )
{
	edict_t *pentSpawnSpot;
	vec3_t vOrigin;

	pentSpawnSpot = RuneSelectSpawnPoint();
	vOrigin = VARS(pentSpawnSpot)->origin;
	
	UTIL_SetOrigin( pev, vOrigin );

	if ( dropped )
		UTIL_LogPrintf( "\"<-1><><>\" triggered triggered \"Respawn_RegenRune\"\n" );
    
	Spawn();
}


void CRegenRune::Spawn ( void )
{

	SET_MODEL( ENT(pev), "models/rune_regen.mdl" );
    
	m_bTouchable = FALSE;

	m_iRuneFlag = ITEM_RUNE4_FLAG;

	dropped = false;

	pev->movetype = MOVETYPE_TOSS;
    pev->solid = SOLID_TRIGGER;

	vec3_t forward, right, up;

	UTIL_SetSize( pev, Vector(-15, -15, -15), Vector(15, 15, 15) ); 

	pev->angles.z = pev->angles.x = 0;
	pev->angles.y = RANDOM_LONG ( 0, 360 );

	//If we got an owner, it means we are either dropping the flag or diying and letting it go.
	if ( pev->owner )
	    g_engfuncs.pfnAngleVectors ( pev->owner->v.angles, forward, right, up);
	else
		g_engfuncs.pfnAngleVectors ( pev->angles, forward, right, up);

	UTIL_SetOrigin( pev, pev->origin );
	
	pev->velocity = ( forward * 400 ) + ( up * 200 );
	
	if ( pev->owner == NULL )
	{
		pev->origin.z += 16;
		pev->velocity.z = 300;
	}
	
	pev->owner = NULL;

	SetTouch( &CRegenRune::RuneTouch );

	pev->nextthink = gpGlobals->time + 1; // if no one touches it in two minutes,
											// respawn it somewhere else, so inaccessible 
											// ones will come 'back'
	SetThink ( &CRegenRune::MakeTouchable );
}


LINK_ENTITY_TO_CLASS( item_rune4, CRegenRune ); 

/*
================
SpawnRunes
spawn all the runes
self is the entity that was created for us, we remove it
================
*/
void SpawnRunes( void )
{
	if ( g_bSpawnedRunes )
		return;

	edict_t *pentSpawnSpot;

	pentSpawnSpot = RuneSelectSpawnPoint();
	CBaseEntity::Create( "item_rune1", VARS(pentSpawnSpot)->origin, VARS(pentSpawnSpot)->angles, NULL );
	
	pentSpawnSpot = RuneSelectSpawnPoint();
	CBaseEntity::Create( "item_rune2", VARS(pentSpawnSpot)->origin, VARS(pentSpawnSpot)->angles, NULL );

	pentSpawnSpot = RuneSelectSpawnPoint();
	CBaseEntity::Create( "item_rune3", VARS(pentSpawnSpot)->origin, VARS(pentSpawnSpot)->angles, NULL );
	
	pentSpawnSpot = RuneSelectSpawnPoint();
	CBaseEntity::Create( "item_rune4", VARS(pentSpawnSpot)->origin, VARS(pentSpawnSpot)->angles, NULL );

	g_bSpawnedRunes = TRUE;
}


/***********************************************
************************************************
				    GRAPPLE
************************************************
***********************************************/

void CGrapple::Reset_Grapple ( void )
{
		CBaseEntity *pOwner =  CBaseEntity::Instance( pev->owner );
		
	    ((CBasePlayer *)pOwner)->m_bOn_Hook = FALSE;
        ((CBasePlayer *)pOwner)->m_bHook_Out = FALSE;

		PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
		((CBasePlayer *)pOwner)->edict(), g_usCable, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
		0.0, 0.0, entindex(), pev->team, 1, 0 );

		STOP_SOUND( edict(), CHAN_WEAPON, "weapons/grhang.wav" );
		STOP_SOUND( ((CBasePlayer *)pOwner)->edict(), CHAN_WEAPON, "weapons/grfire.wav" );
		STOP_SOUND( ((CBasePlayer *)pOwner)->edict(), CHAN_WEAPON, "weapons/grpull.wav" );

		((CBasePlayer *)pOwner)->m_ppHook = NULL;
		pev->enemy = NULL;

        UTIL_Remove ( this );
}

void CGrapple::GrappleTouch ( CBaseEntity *pOther )
{
	CBaseEntity *pOwner =  CBaseEntity::Instance( pev->owner );

	if ( pOther == pOwner )
             return;
		

        // DO NOT allow the grapple to hook to any projectiles, no matter WHAT!
        // if you create new types of projectiles, make sure you use one of the
        // classnames below or write code to exclude your new classname so
        // grapples will not stick to them.
        if ( FClassnameIs( pOther->pev, "grenade" )||   
			 FClassnameIs( pOther->pev, "spike" ) || 
			 FClassnameIs( pOther->pev, "hook" ) )
			return;
      
        if ( FClassnameIs( pOther->pev, "player" ) )
        {
                // glance off of teammates
                if ( pOther->pev->team == pOwner->pev->team )
                        return;

               // sound (self, CHAN_WEAPON, "player/axhit1.wav", 1, ATTN_NORM);
                //TakeDamage( pOther->pev, pOwner->pev, 10, DMG_GENERIC );

                // make hook invisible since we will be pulling directly
                // towards the player the hook hit. Quakeworld makes it
                // too quirky to try to match hook's velocity with that of
                // the client that it hit. 
               // setmodel (self, "");

				pev->velocity = Vector(0,0,0);
				UTIL_SetOrigin( pev, pOther->pev->origin);
        }
        else if ( !FClassnameIs( pOther->pev, "player" ) )
        {
               // sound (self, CHAN_WEAPON, "player/axhit2.wav", 1, ATTN_NORM);

                // One point of damage inflicted upon impact. Subsequent
                // damage will only be done to PLAYERS... this way secret
                // doors and triggers will only be damaged once.
                if ( pOther->pev->takedamage )
                        TakeDamage( pOther->pev, pOwner->pev, 1, DMG_GENERIC );

                pev->velocity = Vector(0,0,0);

				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/grhit.wav", 1, ATTN_NORM);

				//No sparks underwater
				if ( pev->waterlevel == 0 )
					UTIL_Sparks( pev->origin );
        }

        // conveniently clears the sound channel of the CHAIN1 sound,
        // which is a looping sample and would continue to play. Tink1 is
        // the least offensive choice, ass NULL.WAV loops and clogs the
        // channel with silence
      //  sound (self.owner, CHAN_NO_PHS_ADD+CHAN_WEAPON, "weapons/tink1.wav", 1, ATTN_NORM);

        if ( !(pOwner->pev->button & IN_ATTACK) )
        {
			if ( ((CBasePlayer*)pOwner)->m_bOn_Hook ) 
			{
                Reset_Grapple();
				return;
			}
        }


        if ( pOwner->pev->flags & FL_ONGROUND)
        {
                pOwner->pev->flags &= ~FL_ONGROUND;
//                setorigin(self.owner,self.owner.origin + '0 0 1');
        }

        ((CBasePlayer*)pOwner)->m_bOn_Hook = TRUE;

       // sound (self.owner, CHAN_WEAPON, "weapons/chain2.wav", 1, ATTN_NORM);

        // CHAIN2 is a looping sample. Use LEFTY as a flag so that client.qc
        // will know to only play the tink sound ONCE to clear the weapons
        // sound channel. (Lefty is a leftover from AI.QC, so I reused it to
        // avoid adding a field)
        //self.owner.lefty = TRUE;
	
		STOP_SOUND( ((CBasePlayer *)pOwner)->edict(), CHAN_WEAPON, "weapons/grfire.wav" );

        pev->enemy = pOther->edict();// remember this guy!
        SetThink ( &CGrapple::Grapple_Track );
        pev->nextthink = gpGlobals->time;
		m_flNextIdleTime = gpGlobals->time + 0.1;
		pev->solid = SOLID_NOT;
        SetTouch ( NULL );
};

bool CanSee ( CBaseEntity *pEnemy, CBaseEntity *pOwner )
{
	TraceResult tr;

	UTIL_TraceLine ( pOwner->pev->origin, pEnemy->pev->origin,  ignore_monsters, ENT( pOwner->pev ), &tr);
	if ( tr.flFraction == 1 )
		return TRUE;

	UTIL_TraceLine ( pOwner->pev->origin, pEnemy->pev->origin + Vector( 15, 15, 0 ),  ignore_monsters, ENT( pOwner->pev ), &tr);
	if ( tr.flFraction == 1 )
		return TRUE;

	UTIL_TraceLine ( pOwner->pev->origin, pEnemy->pev->origin + Vector( -15, -15, 0 ),  ignore_monsters, ENT( pOwner->pev ), &tr);
	if ( tr.flFraction == 1 )
		return TRUE;

	UTIL_TraceLine ( pOwner->pev->origin, pEnemy->pev->origin + Vector( -15, 15, 0 ),  ignore_monsters, ENT( pOwner->pev ), &tr);
	if ( tr.flFraction == 1 )
		return TRUE;

	UTIL_TraceLine ( pOwner->pev->origin, pEnemy->pev->origin + Vector( 15, -15, 0 ),  ignore_monsters, ENT( pOwner->pev ), &tr);
	if ( tr.flFraction == 1 )
		return TRUE;

	return FALSE;
}

void CGrapple::Grapple_Track ( void )
{
	CBaseEntity *pOwner =  CBaseEntity::Instance( pev->owner );
	CBaseEntity *pEnemy =  CBaseEntity::Instance( pev->enemy );

        // Release dead targets
        if ( FClassnameIs( pEnemy->pev, "player" ) && pEnemy->pev->health <= 0)
                Reset_Grapple();
                
        
		// drop the hook if owner is dead or has released the button
        if ( !((CBasePlayer*)pOwner)->m_bOn_Hook|| ((CBasePlayer*)pOwner)->pev->health <= 0)
        {
                Reset_Grapple();
                return;
        }

		if ( !(pOwner->pev->button & IN_ATTACK) )
        {
			if ( ((CBasePlayer*)pOwner)->m_iQuakeWeapon == IT_EXTRA_WEAPON ) 
			{
                Reset_Grapple();
				return;
			}
        }

        // bring the pAiN!
        if ( FClassnameIs( pEnemy->pev, "player" ) )
        {
			if ( !CanSee( pEnemy, pOwner ) ) 
			{
				Reset_Grapple();
				return;
			}


			// move the hook along with the player.  It's invisible, but
			// we need this to make the sound come from the right spot
			UTIL_SetOrigin( pev, pEnemy->pev->origin);
			
			//sound (self, CHAN_WEAPON, "blob/land1.wav", 1, ATTN_NORM);

			SpawnBlood( pEnemy->pev->origin, BLOOD_COLOR_RED, 1 );
			((CBasePlayer *)pEnemy)->TakeDamage( pev, pOwner->pev, 1, DMG_GENERIC );
        }

        // If the hook is not attached to the player, constantly copy
        // copy the target's velocity. Velocity copying DOES NOT work properly
        // for a hooked client. 
        if ( !FClassnameIs( pEnemy->pev, "player" ) )
              pev->velocity = pEnemy->pev->velocity;

        pev->nextthink = gpGlobals->time + 0.1;
};

void CBasePlayer::Service_Grapple ( void )
{
        Vector  hook_dir;
		CBaseEntity *pEnemy =  CBaseEntity::Instance( pev->enemy );

        // drop the hook if player lets go of button
        if ( !(pev->button & IN_ATTACK) )
        {
			if ( m_iQuakeWeapon == IT_EXTRA_WEAPON ) 
			{
                ((CGrapple *)m_ppHook)->Reset_Grapple();
				return;
			}
        }

		if ( m_ppHook->pev->enemy != NULL )
		{
			// If hooked to a player, track them directly!
			if ( FClassnameIs( pEnemy->pev, "player" ) )
			{
				pEnemy =  CBaseEntity::Instance( pev->enemy );
				hook_dir = ( pEnemy->pev->origin - pev->origin );
			}
			// else, track to hook
			else if ( !FClassnameIs( pEnemy->pev, "player" ) )
				hook_dir = ( m_ppHook->pev->origin - pev->origin );
			
			pev->velocity =  ( (hook_dir).Normalize() * 750 );
			pev->speed = 750;

			if ( ((CGrapple *)m_ppHook)->m_flNextIdleTime <= gpGlobals->time && (hook_dir).Length() <= 50 )
			{
				//No sparks underwater
				if ( m_ppHook->pev->waterlevel == 0 )
					UTIL_Sparks( m_ppHook->pev->origin );
			
				STOP_SOUND( edict(), CHAN_WEAPON, "weapons/grpull.wav" );
				EMIT_SOUND( ENT( m_ppHook->pev ), CHAN_WEAPON, "weapons/grhang.wav", 1, ATTN_NORM);

				((CGrapple *)m_ppHook)->m_flNextIdleTime = gpGlobals->time + RANDOM_LONG( 1, 3 );

				PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
				edict(), g_usCable, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
				0.0, 0.0, m_ppHook->entindex(), pev->team, 1, 0 );

			}
			else if ( ((CGrapple *)m_ppHook)->m_flNextIdleTime <= gpGlobals->time )
			{
				//No sparks underwater
				if ( m_ppHook->pev->waterlevel == 0 )
					UTIL_Sparks( m_ppHook->pev->origin );

				STOP_SOUND( edict(), CHAN_WEAPON, "weapons/grfire.wav" );
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/grpull.wav", 1, ATTN_NORM);
				((CGrapple *)m_ppHook)->m_flNextIdleTime = gpGlobals->time + RANDOM_LONG( 1, 3 );
			}

		}
};

void CGrapple::OnAirThink ( void )
{
	TraceResult tr;

	CBaseEntity *pOwner =  CBaseEntity::Instance( pev->owner );

	if ( !(pOwner->pev->button & IN_ATTACK) )
	{
         Reset_Grapple();
		 return;
	}

	UTIL_TraceLine ( pev->origin, pOwner->pev->origin,  ignore_monsters, ENT(pev), &tr);

	if ( tr.flFraction < 1.0 )
	{
		Reset_Grapple();
		return;
	}

	pev->nextthink = gpGlobals->time + 0.5;
}

			
     
void CGrapple::Spawn ( void )
{
	pev->movetype = MOVETYPE_FLYMISSILE;
	pev->solid = SOLID_BBOX;

	SET_MODEL ( ENT(pev),"models/hook.mdl");

	SetTouch ( &CGrapple::GrappleTouch );
	SetThink ( &CGrapple::OnAirThink );

	pev->nextthink = gpGlobals->time + 0.1;
}

LINK_ENTITY_TO_CLASS( hook, CGrapple ); 

void CBasePlayer::Throw_Grapple ( void )
{
        if ( m_bHook_Out )
             return;

		CBaseEntity *pHookCBEnt = NULL;
	
		pHookCBEnt = CBaseEntity::Create( "hook", pev->origin, pev->angles, NULL );

		if ( pHookCBEnt )
		{
			m_ppHook = pHookCBEnt;
		
			m_ppHook->pev->owner = edict();
		
			UTIL_MakeVectors ( pev->v_angle);

			UTIL_SetOrigin ( m_ppHook->pev , pev->origin + gpGlobals->v_forward * 16 + Vector( 0, 0, 16 ) );
			UTIL_SetSize( m_ppHook->pev, Vector(0,0,0) , Vector(0,0,0) );

			EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/grfire.wav", 1, ATTN_NORM);

			//Make if fly forward
			m_ppHook->pev->velocity = gpGlobals->v_forward * 1000;
			//And make the hook face forward too!
			m_ppHook->pev->angles = UTIL_VecToAngles ( gpGlobals->v_forward );	
			m_ppHook->pev->fixangle = TRUE;

			PLAYBACK_EVENT_FULL( FEV_GLOBAL | FEV_RELIABLE, 
			edict(), g_usCable, 0, (float *)&g_vecZero, (float *)&g_vecZero, 
			0.0, 0.0, m_ppHook->entindex(), pev->team, 0, 0 );

		
			m_bHook_Out = TRUE;
		}
};



#endif
