//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: VGUI objects for Discwar
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_DISCOBJECTS_H
#define VGUI_DISCOBJECTS_H
#pragma once

//===========================================================
// Disc ammo icon
class CDiscPanel : public Label
{
private:
	BitmapTGA	*m_pDiscTGA_Red;
	BitmapTGA	*m_pDiscTGA_RedGlow;
	BitmapTGA	*m_pDiscTGA_Blue;
	BitmapTGA	*m_pDiscTGA_BlueGlow;
	BitmapTGA	*m_pDiscTGA_Grey;
	BitmapTGA	*m_pDiscTGA_Fast;
	BitmapTGA	*m_pDiscTGA_Freeze;
	BitmapTGA	*m_pDiscTGA_Hard;
	BitmapTGA	*m_pDiscTGA_Triple;
public:
	CDiscPanel(int x,int y,int wide,int tall);
	void Update( int iDiscNo, bool bGlow, int iPowerup );

	virtual void paintBackground()
	{
		// Do nothing, so the background's left transparent.
	}
};

//===========================================================
// Powerup
class CDiscPowerups : public CTransparentPanel
{
public:
	CDiscPowerups();

	void	RecalculateText( int iPowerup );
	Label	*m_pLabel;
};

class CDiscRewards : public CTransparentPanel
{
public:
	CDiscRewards();

	void	RecalculateText( int iReward );
	void	SetMessage( char *pMessage );
	Label	*m_pReward;
	Label	*m_pTeleBonus;
};

//===========================================================
// Arena windows
class CDiscArenaPanel : public CTransparentPanel
{
public:
	CDiscArenaPanel( int x, int y, int wide, int tall );
	int  MsgFunc_GetPlayers(const char *pszName, int iSize, void *pbuf );
	virtual void RecalculateText( void ) {};
	void GetClientList( char *pszString );

	int  m_iNumPlayers;
	int	 m_iClients[ MAX_PLAYERS ];
	int	 m_iRoundNumber;
	int	 m_iSecondsToGo;
};

class CDiscArena_RoundStart : public CDiscArenaPanel
{
public:
	CDiscArena_RoundStart();

	void RecalculateText( void );

	Label	*m_pRound;
	Label	*m_pTeamOne;
	Label	*m_pTeamTwo;
};

class CDiscArena_RoundEnd : public CDiscArenaPanel
{
public:
	CDiscArena_RoundEnd();

	void RecalculateText( void );

	Label	*m_pRound;
	Label	*m_pWinners;
	Label	*m_pWinningTeam;
};

#endif // VGUI_DISCOBJECTS_H
