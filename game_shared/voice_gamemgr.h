//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_GAMEMGR_H
#define VOICE_GAMEMGR_H

#ifdef _WIN32
#pragma once
#endif


#include "voice_common.h"


class CGameRules;
class CBasePlayer;


class IVoiceGameMgrHelper
{
public:
	virtual				~IVoiceGameMgrHelper() {}

	// Called each frame to determine which players are allowed to hear each other.	This overrides
	// whatever squelch settings players have.
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker) = 0;
};


// CVoiceGameMgr manages which clients can hear which other clients.
class CVoiceGameMgr
{
public:
						CVoiceGameMgr();
	virtual				~CVoiceGameMgr();
	
	bool				Init(
		IVoiceGameMgrHelper *m_pHelper,
		int maxClients
		);

	void				SetHelper(IVoiceGameMgrHelper *pHelper);

	// Updates which players can hear which other players.
	// If gameplay mode is DM, then only players within the PVS can hear each other.
	// If gameplay mode is teamplay, then only players on the same team can hear each other.
	// Player masks are always applied.
	void				Update(double frametime);

	// Called when a new client connects (unsquelches its entity for everyone).
	void				ClientConnected(struct edict_s *pEdict);

	// Called on ClientCommand. Checks for the squelch and unsquelch commands.
	// Returns true if it handled the command.
	bool				ClientCommand(CBasePlayer *pPlayer, const char *cmd);

	// Called to determine if the Receiver has muted (blocked) the Sender
	// Returns true if the receiver has blocked the sender
	bool				PlayerHasBlockedPlayer(CBasePlayer *pReceiver, CBasePlayer *pSender);


private:

	// Force it to update the client masks.
	void				UpdateMasks();


private:
	int					m_msgPlayerVoiceMask;
	int					m_msgRequestState;

	IVoiceGameMgrHelper	*m_pHelper;
	int					m_nMaxPlayers;
	double				m_UpdateInterval;						// How long since the last update.
};


#endif // VOICE_GAMEMGR_H
