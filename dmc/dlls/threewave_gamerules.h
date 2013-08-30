#ifdef THREEWAVE

#define BLUE 2
#define RED 1


#include "voice_gamemgr.h"



//=========================================================
// Flags
//=========================================================
class CItemFlag : public CBaseEntity
{
public:
    void Spawn( void ); 

    BOOL Dropped;
	float m_flDroppedTime;

	void EXPORT FlagThink( void );

private:
    void Precache ( void );
    void Capture(CBasePlayer *pPlayer, int iTeam );
    void ResetFlag( int iTeam );
    void Materialize( void );
    void EXPORT FlagTouch( CBaseEntity *pOther );
   // BOOL MyTouch( CBasePlayer *pPlayer );
    
}; 

class CCarriedFlag : public CBaseEntity
{
public:
    void Spawn( void ); 

    CBasePlayer *Owner;

	int m_iOwnerOldVel;
   
private:
    void Precache ( void );
    void EXPORT FlagThink( void );
}; 


class CResistRune : public CBaseEntity
{
private:
	
	void EXPORT RuneRespawn ( void );
	
public:
 
	void EXPORT RuneTouch ( CBaseEntity *pOther );
	void Spawn( void );

	void EXPORT MakeTouchable ( void );

	int m_iRuneFlag;
	bool m_bTouchable;
	bool dropped;
}; 

class CStrengthRune : public CBaseEntity
{
	
private:
	void EXPORT RuneRespawn ( void );

public:
 
	void EXPORT RuneTouch ( CBaseEntity *pOther );
	void Spawn( void );

	void EXPORT MakeTouchable ( void );

	int m_iRuneFlag;
	bool m_bTouchable;
	bool dropped;
}; 


class CHasteRune : public CBaseEntity
{
	
private:
	void EXPORT RuneRespawn ( void );
public:
 
	void EXPORT RuneTouch ( CBaseEntity *pOther );

	void EXPORT MakeTouchable ( void );
	void Spawn( void );

	int m_iRuneFlag;
	bool m_bTouchable;
	bool dropped;
}; 


class CRegenRune : public CBaseEntity
{
	
private:
	void EXPORT RuneRespawn ( void );

public:
 
	void EXPORT RuneTouch ( CBaseEntity *pOther );
	void Spawn( void );

	void EXPORT MakeTouchable ( void );

	int m_iRuneFlag;
	bool m_bTouchable;
	bool dropped;
}; 

class CGrapple : public CBaseEntity
{
public:

	//Yes, I have no imagination so I use standard touch, spawn and think function names.
	//Sue me! =P.
	void Spawn ( void );
	void EXPORT OnAirThink ( void );
	void EXPORT GrappleTouch ( CBaseEntity *pOther );
	void Reset_Grapple ( void );
	void EXPORT Grapple_Track ( void );

	float m_flNextIdleTime;
	
};



#define STEAL_SOUND 1
#define CAPTURE_SOUND 2
#define RETURN_SOUND 3

#define RED_FLAG_STOLEN 1
#define BLUE_FLAG_STOLEN 2
#define RED_FLAG_CAPTURED 3
#define BLUE_FLAG_CAPTURED 4
#define RED_FLAG_RETURNED_PLAYER 5
#define BLUE_FLAG_RETURNED_PLAYER 6
#define RED_FLAG_RETURNED 7
#define BLUE_FLAG_RETURNED 8
#define RED_FLAG_LOST 9
#define BLUE_FLAG_LOST 10

#define RED_FLAG_STOLEN 1
#define BLUE_FLAG_STOLEN 2
#define RED_FLAG_DROPPED 3
#define BLUE_FLAG_DROPPED 4
#define RED_FLAG_ATBASE 5
#define BLUE_FLAG_ATBASE 6


#define MAX_TEAMNAME_LENGTH	16
#define MAX_TEAMS			32

#define TEAMPLAY_TEAMLISTLENGTH		MAX_TEAMS*MAX_TEAMNAME_LENGTH

class CThreeWave : public CHalfLifeMultiplay
{
public:
	CThreeWave();

	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd );
	virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer );
	virtual BOOL IsTeamplay( void );
	virtual BOOL FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
	virtual const char *GetTeamID( CBaseEntity *pEntity );
	virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target );
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void InitHUD( CBasePlayer *pl );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor );
	virtual const char *GetGameDescription( void ) { return "3Wave CTF"; }  // this is the game name that gets seen in the server browser
	virtual void UpdateGameMode( CBasePlayer *pPlayer );  // the client needs to be informed of the current game mode
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void Think ( void );
	virtual int GetTeamIndex( const char *pTeamName );
	virtual const char *GetIndexedTeamName( int teamIndex );
	virtual BOOL IsValidTeam( const char *pTeamName );
	virtual void ChangePlayerTeam( CBasePlayer *pPlayer, int iTeam );
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	void JoinTeam ( CBasePlayer *pPlayer, int iTeam );
	int TeamWithFewestPlayers ( void );
	virtual void ClientDisconnected( edict_t *pClient );
	void GetFlagStatus( CBasePlayer *pPlayer );

	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	virtual void PlayerThink( CBasePlayer *pPlayer );

	void PlayerTakeDamage( CBasePlayer *pPlayer , CBaseEntity *pAttacker );

	int iBlueFlagStatus;
	int iRedFlagStatus;

	int iBlueTeamScore;
	int iRedTeamScore; 

	float m_flFlagStatusTime;

private:
	void RecountTeams( void );

	BOOL m_DisableDeathMessages;
	BOOL m_DisableDeathPenalty;
	BOOL m_teamLimit;				// This means the server set only some teams as valid
	char m_szTeamList[TEAMPLAY_TEAMLISTLENGTH];
};

#endif
