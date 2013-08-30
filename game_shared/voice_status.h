//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_STATUS_H
#define VOICE_STATUS_H
#pragma once

#include "voice_common.h"
#include "voice_banmgr.h"



// This is provided by each mod to access data that may not be the same across mods.
class IVoiceStatusHelper
{
public:
	virtual					~IVoiceStatusHelper()	{}

	// Get RGB color for voice status text about this player.
	virtual void			GetPlayerTextColor(int entindex, int color[3]) = 0;

	// Return the height above the bottom that the voice ack icons should be drawn at.
	virtual int				GetAckIconHeight() = 0;

	// Return true if the voice manager is allowed to show speaker labels
	// (mods usually return false when the scoreboard is up).
	virtual bool			CanShowSpeakerLabels() = 0;

	// return a pre-translated string for the player's location.  Defaults to empty string for games without locations
	virtual const char * GetPlayerLocation(int entindex) { return ""; }
};


class IVoiceStatus
{
public:

	// returns the state of this player using the enum above
	virtual int GetSpeakerStatus(int iPlayer) = 0;
	virtual bool IsTalking() = 0;
	virtual bool ServerAcked() = 0;

};



class IVoiceHud
{
public:

	virtual int Init(IVoiceStatusHelper *pHelper, IVoiceStatus *pStatus)=0;
	
	// ackPosition is the bottom position of where CVoiceStatus will draw the voice acknowledgement labels.
	virtual int VidInit() = 0;

	// Call from the HUD_CreateEntities function so it can add sprites above player heads.
	virtual void	CreateEntities() = 0;

	// Sets a player's location (can be a #-prefixed string for localization).
	virtual void UpdateLocation(int entindex, const char *location) = 0;

	virtual void UpdateSpeakerStatus(int entindex, bool bTalking) = 0;

	virtual void RepositionLabels() = 0;

};


class CVoiceStatus: public IVoiceStatusHelper, public IVoiceStatus
{

public:

	CVoiceStatus();
	~CVoiceStatus();

	void Init(IVoiceStatusHelper *pHelper);

	// Called when a player starts or stops talking.
	// entindex is -1 to represent the local client talking (before the data comes back from the server). 
	// When the server acknowledges that the local client is talking, then entindex will be gEngfuncs.GetLocalPlayer().
	// entindex is -2 to represent the local client's voice being acked by the server.
	void	UpdateSpeakerStatus(int entindex, bool bTalking);

	// returns the state of this player using the enum above
	int GetSpeakerStatus(int iPlayer);


		// Called when the server registers a change to who this client can hear.
	void	HandleVoiceMaskMsg(int iSize, void *pbuf);

	// The server sends this message initially to tell the client to send their state.
	void	HandleReqStateMsg(int iSize, void *pbuf);


	void Frame(double frametime);

		// When you enter squelch mode, pass in 
	void	StartSquelchMode();
	void	StopSquelchMode();
	bool	IsInSquelchMode();

	// returns true if the target client has been banned
	// playerIndex is of range 1..maxplayers
	bool	IsPlayerBlocked(int iPlayerIndex);

	// returns false if the player can't hear the other client due to game rules (eg. the other team)
	bool    IsPlayerAudible(int iPlayerIndex);

	// blocks the target client from being heard
	void	SetPlayerBlockedState(int iPlayerIndex, bool blocked);

	void			UpdateServerState(bool bForce);

	virtual bool CanShowSpeakerLabels()
	{
		return m_pHelper->CanShowSpeakerLabels();
	}

	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		m_pHelper->GetPlayerTextColor(entindex,color);
	}

	virtual int	GetAckIconHeight()
	{
		return m_pHelper->GetAckIconHeight();	
	}

	bool IsTalking()
	{
		return m_bTalking;
	}

	bool ServerAcked()
	{
		return m_bServerAcked;
	}

	CVoiceBanMgr *GetBanMgr() { return &m_BanMgr; }



	enum 
	{
		VOICE_TALKING=1, // start from one because ImageList's don't use pos 0
		VOICE_BANNED,
		VOICE_NEVERSPOKEN,
		VOICE_NOTTALKING,
	}; // various voice states


private:
	float			m_LastUpdateServerState;		// Last time we called this function.
	int				m_bServerModEnable;				// What we've sent to the server about our "voice_modenable" cvar.

	CPlayerBitVec	m_VoicePlayers;		// Who is currently talking. Indexed by client index.
	
	// This is the gamerules-defined list of players that you can hear. It is based on what teams people are on 
	// and is totally separate from the ban list. Indexed by client index.
	CPlayerBitVec	m_AudiblePlayers;

	// Players who have spoken at least once in the game so far
	CPlayerBitVec	m_VoiceEnabledPlayers;	

	// This is who the server THINKS we have banned (it can become incorrect when a new player arrives on the server).
	// It is checked periodically, and the server is told to squelch or unsquelch the appropriate players.
	CPlayerBitVec	m_ServerBannedPlayers;

	
	IVoiceStatusHelper	*m_pHelper;		// Each mod provides an implementation of this.

	// Squelch mode stuff.
	bool				m_bInSquelchMode;

	
	bool				m_bTalking;				// Set to true when the client thinks it's talking.
	bool				m_bServerAcked;			// Set to true when the server knows the client is talking.


	CVoiceBanMgr		m_BanMgr;				// Tracks which users we have squelched and don't want to hear.

	bool				m_bBanMgrInitialized;

		// Cache the game directory for use when we shut down
	char *				m_pchGameDir;



};

CVoiceStatus* GetClientVoice();

// Get the (global) voice manager. 
IVoiceHud* GetClientVoiceHud();





#endif // VOICE_STATUS_H