//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VOICE_STATUS_H
#define VOICE_STATUS_H
#pragma once


#include "VGUI_Label.h"
#include "VGUI_LineBorder.h"
#include "VGUI_ImagePanel.h"
#include "VGUI_BitmapTGA.h"
#include "VGUI_InputSignal.h"
#include "VGUI_Button.h"
#include "voice_common.h"
#include "cl_entity.h"
#include "voice_banmgr.h"
#include "vgui_checkbutton2.h"
#include "vgui_defaultinputsignal.h"


class CVoiceStatus;


class CVoiceLabel
{
public:
	vgui::Label			*m_pLabel;
	vgui::Label			*m_pBackground;
	vgui::ImagePanel	*m_pIcon;		// Voice icon next to player name.
	int					m_clientindex;	// Client index of the speaker. -1 if this label isn't being used.
};


// This is provided by each mod to access data that may not be the same across mods.
class IVoiceStatusHelper
{
public:
	virtual					~IVoiceStatusHelper()	{}

	// Get RGB color for voice status text about this player.
	virtual void			GetPlayerTextColor(int entindex, int color[3]) = 0;

	// Force it to update the cursor state.
	virtual void			UpdateCursorState() = 0;

	// Return the height above the bottom that the voice ack icons should be drawn at.
	virtual int				GetAckIconHeight() = 0;

	// Return true if the voice manager is allowed to show speaker labels
	// (mods usually return false when the scoreboard is up).
	virtual bool			CanShowSpeakerLabels() = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Holds a color for the shared image
//-----------------------------------------------------------------------------
class VoiceImagePanel : public vgui::ImagePanel
{
	virtual void paintBackground()
	{
		if (_image!=null)
		{
			vgui::Color col;
			getFgColor(col);
			_image->setColor(col);
			_image->doPaint(this);
		}
	}
};


class CVoiceStatus : public CHudBase, public vgui::CDefaultInputSignal
{
public:
				CVoiceStatus();
	virtual		~CVoiceStatus();

// CHudBase overrides.
public:
	
	// Initialize the cl_dll's voice manager.
	virtual int Init(
		IVoiceStatusHelper *m_pHelper,
		vgui::Panel **pParentPanel);
	
	// ackPosition is the bottom position of where CVoiceStatus will draw the voice acknowledgement labels.
	virtual int VidInit();


public:
	
	// Call from HUD_Frame each frame.
	void	Frame(double frametime);

	// Called when a player starts or stops talking.
	// entindex is -1 to represent the local client talking (before the data comes back from the server). 
	// When the server acknowledges that the local client is talking, then entindex will be gEngfuncs.GetLocalPlayer().
	// entindex is -2 to represent the local client's voice being acked by the server.
	void	UpdateSpeakerStatus(int entindex, qboolean bTalking);

	// sets the correct image in the label for the player
	void	UpdateSpeakerImage(vgui::Label *pLabel, int iPlayer);

	// Call from the HUD_CreateEntities function so it can add sprites above player heads.
	void	CreateEntities();

	// Called when the server registers a change to who this client can hear.
	void	HandleVoiceMaskMsg(int iSize, void *pbuf);

	// The server sends this message initially to tell the client to send their state.
	void	HandleReqStateMsg(int iSize, void *pbuf);


// Squelch mode functions.
public:

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

public:

	CVoiceLabel*	FindVoiceLabel(int clientindex);	// Find a CVoiceLabel representing the specified speaker. 
													// Returns NULL if none.
													// entindex can be -1 if you want a currently-unused voice label.
	CVoiceLabel*	GetFreeVoiceLabel();			// Get an unused voice label. Returns NULL if none.

	void			RepositionLabels();

	void			FreeBitmaps();

	void			UpdateServerState(bool bForce);

	// Update the button artwork to reflect the client's current state.
	void			UpdateBanButton(int iClient);


public:

	enum			{MAX_VOICE_SPEAKERS=7};

	float			m_LastUpdateServerState;		// Last time we called this function.
	int				m_bServerModEnable;				// What we've sent to the server about our "voice_modenable" cvar.

	vgui::Panel		**m_pParentPanel;
	CPlayerBitVec	m_VoicePlayers;		// Who is currently talking. Indexed by client index.
	
	// This is the gamerules-defined list of players that you can hear. It is based on what teams people are on 
	// and is totally separate from the ban list. Indexed by client index.
	CPlayerBitVec	m_AudiblePlayers;

	// Players who have spoken at least once in the game so far
	CPlayerBitVec	m_VoiceEnabledPlayers;	

	// This is who the server THINKS we have banned (it can become incorrect when a new player arrives on the server).
	// It is checked periodically, and the server is told to squelch or unsquelch the appropriate players.
	CPlayerBitVec	m_ServerBannedPlayers;

	cl_entity_s		m_VoiceHeadModels[VOICE_MAX_PLAYERS];			// These aren't necessarily in the order of players. They are just
																	// a place for it to put data in during CreateEntities.

	IVoiceStatusHelper	*m_pHelper;		// Each mod provides an implementation of this.

	
	// Scoreboard icons.
	double			m_BlinkTimer;			// Blink scoreboard icons..
	vgui::BitmapTGA	*m_pScoreboardNeverSpoken;
	vgui::BitmapTGA	*m_pScoreboardNotSpeaking;
	vgui::BitmapTGA	*m_pScoreboardSpeaking;
	vgui::BitmapTGA	*m_pScoreboardSpeaking2;
	vgui::BitmapTGA	*m_pScoreboardSquelch;
	vgui::BitmapTGA	*m_pScoreboardBanned;
	
	vgui::Label		   *m_pBanButtons[VOICE_MAX_PLAYERS];		// scoreboard buttons.

	// Squelch mode stuff.
	bool				m_bInSquelchMode;
	
	HSPRITE				m_VoiceHeadModel;		// Voice head model (goes above players who are speaking).
	float				m_VoiceHeadModelHeight;	// Height above their head to place the model.

	vgui::Image			*m_pSpeakerLabelIcon;	// Icon next to speaker labels.

	// Lower-right icons telling when the local player is talking..
	vgui::BitmapTGA		*m_pLocalBitmap;		// Represents the local client talking.
	vgui::BitmapTGA		*m_pAckBitmap;			// Represents the server ack'ing the client talking.
	vgui::ImagePanel	*m_pLocalLabel;			// Represents the local client talking.

	bool				m_bTalking;				// Set to true when the client thinks it's talking.
	bool				m_bServerAcked;			// Set to true when the server knows the client is talking.

public:
	
	CVoiceBanMgr		m_BanMgr;				// Tracks which users we have squelched and don't want to hear.

public:

	bool				m_bBanMgrInitialized;

	// Labels telling who is speaking.
	CVoiceLabel			m_Labels[MAX_VOICE_SPEAKERS];

	// Cache the game directory for use when we shut down
	char *				m_pchGameDir;
};


// Get the (global) voice manager. 
CVoiceStatus* GetClientVoiceMgr();


#endif // VOICE_STATUS_H
