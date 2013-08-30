//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "archtypes.h"     // DAL
#include "voice_gamemgr.h"
#include <string.h>
#include <assert.h>
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"



#define UPDATE_INTERVAL	0.3


// These are stored off as CVoiceGameMgr is created and deleted.
CPlayerBitVec	g_PlayerModEnable;		// Set to 1 for each player if the player wants to use voice in this mod.
										// (If it's zero, then the server reports that the game rules are saying the
										// player can't hear anyone).

CPlayerBitVec	g_BanMasks[VOICE_MAX_PLAYERS];	// Tells which players don't want to hear each other.
												// These are indexed as clients and each bit represents a client
												// (so player entity is bit+1).

CPlayerBitVec	g_SentGameRulesMasks[VOICE_MAX_PLAYERS];	// These store the masks we last sent to each client so we can determine if
CPlayerBitVec	g_SentBanMasks[VOICE_MAX_PLAYERS];			// we need to resend them.
CPlayerBitVec	g_bWantModEnable;

cvar_t voice_serverdebug = {"voice_serverdebug", "0"};

// Set game rules to allow all clients to talk to each other.
// Muted players still can't talk to each other.
cvar_t sv_alltalk = {"sv_alltalk", "0", FCVAR_SERVER};

// ------------------------------------------------------------------------ //
// Static helpers.
// ------------------------------------------------------------------------ //

// Find a player with a case-insensitive name search.
static CBasePlayer* FindPlayerByName(const char *pTestName)
{
	for(int i=1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex(i);
		if(pEdict)
		{
			CBaseEntity *pEnt = CBaseEntity::Instance(pEdict);
			if(pEnt && pEnt->IsPlayer())
			{			
				const char *pNetName = STRING(pEnt->pev->netname);
				if(stricmp(pNetName, pTestName) == 0)
				{
					return (CBasePlayer*)pEnt;
				}
			}
		}
	}

	return NULL;
}

static void VoiceServerDebug( char const *pFmt, ... )
{
	char msg[4096];
	va_list marker;

	if( !voice_serverdebug.value )
		return;

	va_start( marker, pFmt );
	_vsnprintf( msg, sizeof(msg), pFmt, marker );
	va_end( marker );

	ALERT( at_console, "%s", msg );
}



// ------------------------------------------------------------------------ //
// CVoiceGameMgr.
// ------------------------------------------------------------------------ //

CVoiceGameMgr::CVoiceGameMgr()
{
	m_UpdateInterval = 0;
	m_nMaxPlayers = 0;
}


CVoiceGameMgr::~CVoiceGameMgr()
{
}


bool CVoiceGameMgr::Init(
	IVoiceGameMgrHelper *pHelper,
	int maxClients)
{		  
	m_pHelper = pHelper;
	m_nMaxPlayers = VOICE_MAX_PLAYERS < maxClients ? VOICE_MAX_PLAYERS : maxClients;
	g_engfuncs.pfnPrecacheModel("sprites/voiceicon.spr");

	m_msgPlayerVoiceMask = REG_USER_MSG( "VoiceMask", VOICE_MAX_PLAYERS_DW*4 * 2 );
	m_msgRequestState = REG_USER_MSG( "ReqState", 0 );
	
	// register voice_serverdebug if it hasn't been registered already
	if ( !CVAR_GET_POINTER( "voice_serverdebug" ) )
		CVAR_REGISTER( &voice_serverdebug );

	if( !CVAR_GET_POINTER( "sv_alltalk" ) )
		CVAR_REGISTER( &sv_alltalk );

	return true;
}


void CVoiceGameMgr::SetHelper(IVoiceGameMgrHelper *pHelper)
{
	m_pHelper = pHelper;
}


void CVoiceGameMgr::Update(double frametime)
{
	// Only update periodically.
	m_UpdateInterval += frametime;
	if(m_UpdateInterval < UPDATE_INTERVAL)
		return;

	UpdateMasks();
}


void CVoiceGameMgr::ClientConnected(edict_t *pEdict)
{
	int index = ENTINDEX(pEdict) - 1;
	
	// Clear out everything we use for deltas on this guy.
	g_bWantModEnable[index] = true;
	g_SentGameRulesMasks[index].Init(0);
	g_SentBanMasks[index].Init(0);
}

// Called to determine if the Receiver has muted (blocked) the Sender
// Returns true if the receiver has blocked the sender
bool CVoiceGameMgr::PlayerHasBlockedPlayer(CBasePlayer *pReceiver, CBasePlayer *pSender)
{
	int iReceiverIndex, iSenderIndex;

	if ( !pReceiver || !pSender )
		return false;

	iReceiverIndex = pReceiver->entindex() - 1;
	iSenderIndex   = pSender->entindex() - 1;

	if ( iReceiverIndex < 0 || iReceiverIndex >= m_nMaxPlayers || iSenderIndex < 0 || iSenderIndex >= m_nMaxPlayers )
		return false;

	return ( g_BanMasks[iReceiverIndex][iSenderIndex] ? true : false );
}

bool CVoiceGameMgr::ClientCommand(CBasePlayer *pPlayer, const char *cmd)
{
	int playerClientIndex = pPlayer->entindex() - 1;
	if(playerClientIndex < 0 || playerClientIndex >= m_nMaxPlayers)
	{
		VoiceServerDebug( "CVoiceGameMgr::ClientCommand: cmd %s from invalid client (%d)\n", cmd, playerClientIndex );
		return true;
	}

	bool bBan = stricmp(cmd, "vban") == 0;
	if(bBan && CMD_ARGC() >= 2)
	{
		for(int i=1; i < CMD_ARGC(); i++)
		{
			uint32 mask = 0;
			sscanf(CMD_ARGV(i), "%x", &mask);

			if(i <= VOICE_MAX_PLAYERS_DW)
			{
				VoiceServerDebug( "CVoiceGameMgr::ClientCommand: vban (0x%x) from %d\n", mask, playerClientIndex );
				g_BanMasks[playerClientIndex].SetDWord(i-1, mask);
			}
			else
			{
				VoiceServerDebug( "CVoiceGameMgr::ClientCommand: invalid index (%d)\n", i );
			}
		}

		// Force it to update the masks now.
		//UpdateMasks();		
		return true;
	}
	else if(stricmp(cmd, "VModEnable") == 0 && CMD_ARGC() >= 2)
	{
		VoiceServerDebug( "CVoiceGameMgr::ClientCommand: VModEnable (%d)\n", !!atoi(CMD_ARGV(1)) );
		g_PlayerModEnable[playerClientIndex] = !!atoi(CMD_ARGV(1));
		g_bWantModEnable[playerClientIndex] = false;
		//UpdateMasks();		
		return true;
	}
	else
	{
		return false;
	}
}


void CVoiceGameMgr::UpdateMasks()
{
	m_UpdateInterval = 0;

	bool bAllTalk = !!(sv_alltalk.value);

	for(int iClient=0; iClient < m_nMaxPlayers; iClient++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(iClient+1);
		if(!pEnt || !pEnt->IsPlayer())
			continue;

		// Request the state of their "VModEnable" cvar.
		if(g_bWantModEnable[iClient])
		{
			MESSAGE_BEGIN(MSG_ONE, m_msgRequestState, NULL, pEnt->pev);
			MESSAGE_END();
		}

		CBasePlayer *pPlayer = (CBasePlayer*)pEnt;

		CPlayerBitVec gameRulesMask;
		if( g_PlayerModEnable[iClient] )
		{
			// Build a mask of who they can hear based on the game rules.
			for(int iOtherClient=0; iOtherClient < m_nMaxPlayers; iOtherClient++)
			{
				CBaseEntity *pEnt = UTIL_PlayerByIndex(iOtherClient+1);
				if(pEnt && (bAllTalk || m_pHelper->CanPlayerHearPlayer(pPlayer, (CBasePlayer*)pEnt)) )
				{
					gameRulesMask[iOtherClient] = true;
				}
			}
		}

		// If this is different from what the client has, send an update. 
		if(gameRulesMask != g_SentGameRulesMasks[iClient] || 
			g_BanMasks[iClient] != g_SentBanMasks[iClient])
		{
			g_SentGameRulesMasks[iClient] = gameRulesMask;
			g_SentBanMasks[iClient] = g_BanMasks[iClient];

			MESSAGE_BEGIN(MSG_ONE, m_msgPlayerVoiceMask, NULL, pPlayer->pev);
				int dw;
				for(dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
				{
					WRITE_LONG(gameRulesMask.GetDWord(dw));
					WRITE_LONG(g_BanMasks[iClient].GetDWord(dw));
				}
			MESSAGE_END();
		}

		// Tell the engine.
		for(int iOtherClient=0; iOtherClient < m_nMaxPlayers; iOtherClient++)
		{
			bool bCanHear = gameRulesMask[iOtherClient] && !g_BanMasks[iClient][iOtherClient];
			g_engfuncs.pfnVoice_SetClientListening(iClient+1, iOtherClient+1, bCanHear);
		}
	}
}
