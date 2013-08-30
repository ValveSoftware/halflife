//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
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

#ifndef DISC_ARENA_H
#define DISC_ARENA_H
#pragma once

#define MAX_ARENAS					16

// Arena States
#define ARENA_WAITING_FOR_PLAYERS	0
#define ARENA_COUNTDOWN				1
#define ARENA_BATTLE_IN_PROGRESS	2
#define ARENA_SHOWING_SCORES		3

// Arena Times
#define ARENA_TIME_PREBATTLE		5
#define ARENA_TIME_VIEWSCORES		3
#define ARENA_TIME_ROUNDLIMIT		120		// Timelimit on rounds

enum
{
	GAME_LOST = 0,
	GAME_WON,
	GAME_DIDNTPLAY,
};

//-----------------------------------------------------------------------------
// Arena object
class CDiscArena : public CBaseEntity
{
public:
	void Spawn( void );
	void Reset( void );

	// Battle initialisation
	void StartBattle( void );
	void StartRound( void );
	void SpawnCombatant( CBasePlayer *pPlayer );
	void MoveToSpectator( CBasePlayer *pPlayer );
	void EXPORT StartBattleThink( void );

	// Battle running
	void EXPORT CountDownThink( void );
	void PlayerKilled( CBasePlayer *pPlayer );
	void PlayerRespawned( CBasePlayer *pPlayer );
	void BattleOver( void );
	void EXPORT CheckOverThink( void );
	void EXPORT FinishedThink( void );
	void RestoreWorldObjects( void );
	int  ValidateCombatants( void );
	void EXPORT TimeOver( void );
	void EXPORT BattleThink( void );
	bool CheckBattleOver( void );

	// Client handling
	void AddClient( CBasePlayer *pPlayer, BOOL bCheckStart );
	void RemoveClient( CBasePlayer *pPlayer );
	void AddPlayerToQueue( CBasePlayer *pPlayer );
	void RemovePlayerFromQueue( CBasePlayer *pPlayer );
	CBasePlayer * GetNextPlayer( void );

	// Multiple Arena handling
	int  IsFull( void );
	CBasePlayer *GetFirstSparePlayer( void );
	void PostBattle( void );

	// Game handling
	bool AllowedToFire( void );

	// Variables
	int		m_iArenaState;
	int		m_iPlayers;
	int		m_iMaxRounds;
	int		m_iCurrRound;
	int		m_iPlayersPerTeam;			// Current players per team
	int		m_iSecondsTillStart;
	int		m_iWinningTeam;
	int		m_iTeamOneScore;
	int		m_iTeamTwoScore;
	float	m_flTimeLimitOver;
	BOOL	m_bShownTimeWarning;

	// Queue
	EHANDLE m_pPlayerQueue;

	// Players in the current battle
	EHANDLE m_hCombatants[ 32 ];
};

extern CDiscArena *g_pArenaList[ MAX_ARENAS ];
extern float g_iaDiscColors[33][3];

int InArenaMode();

#endif // DISC_ARENA_H
