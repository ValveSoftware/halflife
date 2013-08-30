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

#include "VGUI_Font.h"

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "parsemsg.h"
#include "ammo.h"
#include "string.h"
#include "ammohistory.h"

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ServerBrowser.h"
#include "vgui_discobjects.h"

// Positions and Dimensions
#define ARENAWINDOW_SIZE_X		(ScreenWidth)
#define ARENAWINDOW_SIZE_Y		YRES(128)
#define ARENAWINDOW_X			((ScreenWidth - ARENAWINDOW_SIZE_X) / 2)
#define ARENAWINDOW_Y			(ScreenHeight - ARENAWINDOW_SIZE_Y)

#define POWERUP_SIZE_X			(ScreenWidth)
#define POWERUP_SIZE_Y			YRES(32)
#define POWERUP_X				((ScreenWidth - POWERUP_SIZE_X) / 2)
#define POWERUP_Y				(ScreenHeight - POWERUP_SIZE_Y)

#define REWARD_SIZE_X			(ScreenWidth)
#define REWARD_SIZE_Y			YRES(48)
#define REWARD_X				((ScreenWidth - POWERUP_SIZE_X) / 2)
#define REWARD_Y				(ScreenHeight / 6)

extern WeaponsResource gWR;
int	   g_iCannotFire;

//===========================================================
// Disc ammo icon
CDiscPanel::CDiscPanel(int x,int y,int wide,int tall) : Label("", x,y,wide,tall)
{
	setContentFitted(true);

	// Standard discs
	m_pDiscTGA_Red = LoadTGAForRes("discred");
	m_pDiscTGA_RedGlow = LoadTGAForRes("discred2");
	m_pDiscTGA_Blue = LoadTGAForRes("discblue");
	m_pDiscTGA_BlueGlow = LoadTGAForRes("discblue2");
	m_pDiscTGA_Grey = LoadTGAForRes("discgrey");

	// Powerup discs
	m_pDiscTGA_Fast = LoadTGAForRes("fast");
	m_pDiscTGA_Freeze = LoadTGAForRes("freeze");
	m_pDiscTGA_Hard = LoadTGAForRes("hard");
	m_pDiscTGA_Triple = LoadTGAForRes("triple");

	setImage( m_pDiscTGA_Red );
}

void CDiscPanel::Update( int iDiscNo, bool bGlow, int iPowerup )
{
	int iDiscs = gWR.GetAmmo( 1 );

	// Grey disc for missing discs
	if ( iDiscs < iDiscNo+1 )
	{
		setImage( m_pDiscTGA_Grey );
	}
	// Powerups override team colored discs
	else if ( iPowerup & POW_TRIPLE )
	{
		setImage( m_pDiscTGA_Triple );
	}
	else if ( iPowerup & POW_FAST )
	{
		setImage( m_pDiscTGA_Fast );
	}
	else if ( iPowerup & POW_FREEZE )
	{
		setImage( m_pDiscTGA_Freeze );
	}
	else if ( iPowerup & POW_HARD )
	{
		setImage( m_pDiscTGA_Hard );
	}
	else if (g_iTeamNumber == 1)
	{
		if ( gWR.GetAmmo( 1 ) == 3 )
			setImage( m_pDiscTGA_RedGlow );
		else
			setImage( m_pDiscTGA_Red );
	}
	else
	{
		if ( gWR.GetAmmo( 1 ) == 3 )
			setImage( m_pDiscTGA_BlueGlow );
		else
			setImage( m_pDiscTGA_Blue );
	}
}

//===========================================================
// Arena window
CDiscArenaPanel::CDiscArenaPanel( int x, int y, int wide, int tall ) : CTransparentPanel(255, x,y,wide,tall)
{
	m_iNumPlayers = 0;
}

//===========================================================
// Message handler. Gets the Ids of the players in the round.
int CDiscArenaPanel::MsgFunc_GetPlayers(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iRoundNumber = READ_BYTE();
	m_iSecondsToGo = READ_BYTE();

	m_iNumPlayers = READ_BYTE();
	if ( m_iNumPlayers > 0 && m_iNumPlayers <= MAX_PLAYERS )
	{
		for (int i = 0; i < m_iNumPlayers; i++)
			m_iClients[i] = READ_SHORT();
	}

	RecalculateText();

	return 1;
}

//===========================================================
// Message handler. Gets the Ids of the players in the round.
void CDiscArenaPanel::GetClientList( char *pszString )
{
	strcpy( pszString, "" );
	for (int i = 0; i < m_iNumPlayers; i++ )
	{
		if ( m_iClients[i] <= 0 || m_iClients[i] > MAX_PLAYERS )
		{
			gEngfuncs.Con_Printf( "Combatant %d out of range: %d\n", i, m_iClients[i] );
			continue;
		}

		if ( g_PlayerInfoList[ m_iClients[i] ].name && g_PlayerInfoList[ m_iClients[i] ].name[0] )
		{	
			if ( i > 0 )
			{
				if ( i == (m_iNumPlayers - 1) )
				{
					strcat( pszString, CHudTextMessage::BufferedLocaliseTextString( "#And" ) );
				}
				else
				{
					strcat( pszString, ", " );
				}
			}

			strcat( pszString, g_PlayerInfoList[ m_iClients[i] ].name );
		}
	}
}

//===========================================================
// Round start window
#define ROUND_Y				YRES(0)
#define TEAMONE_Y			(ROUND_Y + YRES(32))
#define	VERSUS_Y			(TEAMONE_Y + YRES(32))
#define TEAMTWO_Y			(VERSUS_Y + YRES(32))

CDiscArena_RoundStart::CDiscArena_RoundStart( void ) : CDiscArenaPanel( ARENAWINDOW_X, ARENAWINDOW_Y, ARENAWINDOW_SIZE_X, ARENAWINDOW_SIZE_Y )
{
	m_pRound = new Label( "Round 1", 0, ROUND_Y, getWide(), YRES(32) );
	m_pRound->setParent( this );
	m_pRound->setBgColor( 0, 0, 0, 128 );
	m_pRound->setFgColor( 255,255,255, 0 );
	m_pRound->setContentAlignment( vgui::Label::a_center );

	m_pTeamOne = new Label( "Team One", 0, TEAMONE_Y, getWide(), YRES(32) );
	m_pTeamOne->setParent( this );
	m_pTeamOne->setBgColor( 128, 0, 0, 128 );
	m_pTeamOne->setFgColor( 255,255,255, 0 );
	m_pTeamOne->setContentAlignment( vgui::Label::a_center );

	// Trim the trailing \n from the VS string
	char sz[32];
	strcpy( sz, CHudTextMessage::BufferedLocaliseTextString( "#Versus" ) );
	sz[ strlen(sz) - 1 ] = '\0';
	Label *pLabel = new Label( sz, 0, VERSUS_Y, getWide(), YRES(32) );
	pLabel->setParent( this );
	pLabel->setBgColor( 0, 0, 0, 255 );
	pLabel->setFgColor( 255,255,255, 0 );
	pLabel->setContentAlignment( vgui::Label::a_center );

	m_pTeamTwo = new Label( "Team Two", 0, TEAMTWO_Y, getWide(), YRES(32) );
	m_pTeamTwo->setParent( this );
	m_pTeamTwo->setBgColor( 0, 0, 128, 128 );
	m_pTeamTwo->setFgColor( 255,255,255, 0 );
	m_pTeamTwo->setContentAlignment( vgui::Label::a_center );

	setVisible(false);
}

// Recalculate the Text in the window
void CDiscArena_RoundStart::RecalculateText( void )
{
	char sz[1024];
	char szOpponents[1024];
	char szTemp[256];
	char szTemp2[256];
	char szTemp3[256];
	char *pszLocalized = NULL;

	// Round started?
	if (m_iSecondsToGo == 0)
	{
		setVisible(false);
		g_iCannotFire = FALSE;

		// Force spectator menu to update
		if (gViewPort)
			gViewPort->m_iUser1 = 0;
		return;
	}

	g_iCannotFire = TRUE;

	// Round Number
	if ( m_iSecondsToGo != 1 )
	{
		pszLocalized =  "#Round_Start_n_Seconds";
	}
	else
	{
		pszLocalized =  "#Round_Start_1_Second";
	}

	strncpy( szTemp3, CHudTextMessage::BufferedLocaliseTextString( pszLocalized ), sizeof( szTemp3 ) - 1 );
	szTemp3[ sizeof( szTemp3 ) - 1 ] = '\0';
	sprintf( sz, szTemp3, m_iRoundNumber, m_iSecondsToGo );

	m_pRound->setText( sz );

	// We may have just got an update for the time to go. If so, m_iNumPlayers will be 0.
	if ( !m_iNumPlayers )
		return;

	if (gViewPort)
		gViewPort->GetAllPlayersInfo();

	// Find out what team this client's on (if a new battle's just starting)
	strcpy( szOpponents, "" );
	int iMyTeamNumber = 0;
	if ( m_iRoundNumber == 1 )
	{
		for (int i = 0; i < m_iNumPlayers; i++ )
		{
			if ( g_PlayerInfoList[ m_iClients[i] ].thisplayer )
				iMyTeamNumber = (i < (m_iNumPlayers / 2)) ? 1 : 2;
		}
	}

	// Team 1
	strcpy( sz, "" );
	int i;
	for (i = 0; i < (m_iNumPlayers / 2); i++ )
	{
		if ( g_PlayerInfoList[ m_iClients[i] ].name && g_PlayerInfoList[ m_iClients[i] ].name[0] )
			strcat( sz, g_PlayerInfoList[ m_iClients[i] ].name );

		if ( iMyTeamNumber == 2 )
		{
			strcpy( szTemp, CHudTextMessage::BufferedLocaliseTextString( "#Opponent" ) );
			sprintf( szTemp2, szTemp,  g_PlayerInfoList[ m_iClients[i] ].name, g_PlayerExtraInfo[ m_iClients[i] ].deaths, g_PlayerExtraInfo[ m_iClients[i] ].frags );
			strcat( szOpponents, szTemp2 );
		}
	}
	m_pTeamOne->setText( sz );

	// Team 2
	strcpy( sz, "" );
	for ( ; i < m_iNumPlayers; i++ )
	{
		if ( g_PlayerInfoList[ m_iClients[i] ].name && g_PlayerInfoList[ m_iClients[i] ].name[0] )
			strcat( sz, g_PlayerInfoList[ m_iClients[i] ].name );

		if ( iMyTeamNumber == 1 )
		{
			strcpy( szTemp, CHudTextMessage::BufferedLocaliseTextString( "#Opponent" ) );
			sprintf( szTemp2, szTemp,  g_PlayerInfoList[ m_iClients[i] ].name, g_PlayerExtraInfo[ m_iClients[i] ].deaths, g_PlayerExtraInfo[ m_iClients[i] ].frags );
			strcat( szOpponents, szTemp2 );
		}
	}
	m_pTeamTwo->setText( sz );

	// Bring up the Opponent details
	if (gViewPort)
		gViewPort->m_pDiscRewardWindow->SetMessage( szOpponents );

	// Become visible
	setVisible(true);

	// Hide the other windows if it's up
	if (gViewPort)
	{
		gViewPort->m_pSpectatorMenu->setVisible( false );
		gViewPort->m_pDiscPowerupWindow->setVisible( false );
		gViewPort->m_pDiscEndRound->setVisible( false );
	}
}

//===========================================================
// Round end window
CDiscArena_RoundEnd::CDiscArena_RoundEnd( void ) : CDiscArenaPanel( ARENAWINDOW_X, ARENAWINDOW_Y, ARENAWINDOW_SIZE_X, ARENAWINDOW_SIZE_Y )
{
	m_pRound = new Label( "Round 1 Won By", 0, ROUND_Y, getWide(), YRES(32) );
	m_pRound->setParent( this );
	m_pRound->setBgColor( 0, 0, 0, 128 );
	m_pRound->setFgColor( 255,255,255, 0 );
	m_pRound->setContentAlignment( vgui::Label::a_center );

	m_pWinners = new Label( "Winners", 0, TEAMONE_Y, getWide(), YRES(32) );
	m_pWinners->setParent( this );
	m_pWinners->setBgColor( 128, 0, 0, 128 );
	m_pWinners->setFgColor( 255,255,255, 0 );
	m_pWinners->setContentAlignment( vgui::Label::a_center );

	m_pWinningTeam = new Label( "Winners", 0, TEAMTWO_Y, getWide(), YRES(32) );
	m_pWinningTeam->setParent( this );
	m_pWinningTeam->setBgColor( 128, 0, 0, 128 );
	m_pWinningTeam->setFgColor( 255,255,255, 0 );
	m_pWinningTeam->setContentAlignment( vgui::Label::a_center );

	setVisible(false);
}

// Recalculate the Text in the window
void CDiscArena_RoundEnd::RecalculateText( void )
{
	char sz[1024];
	char szTemp1[256];
	char szTemp2[256];

	// Sends down a 0 for time when this should be removed
	if (m_iSecondsToGo == 0)
	{
		setVisible(false);
		g_iCannotFire = FALSE;

		// Force spectator menu to update
		if (gViewPort)
			gViewPort->m_iUser1 = 0;
		return;
	}

	g_iCannotFire = TRUE;

	// Round Number
	strncpy( szTemp1, CHudTextMessage::BufferedLocaliseTextString( "#Round_Won" ), sizeof( szTemp1 ) - 1 );
	szTemp1[ sizeof( szTemp1 ) - 1 ] = '\0';
	sprintf( sz, szTemp1, m_iRoundNumber );

	m_pRound->setText( sz );

	if (gViewPort)
		gViewPort->GetAllPlayersInfo();

	// Winners
	GetClientList( sz );
	m_pWinners->setText( sz );

	// Scores
	m_iNumPlayers = READ_BYTE();
	if ( m_iNumPlayers >= 0 && m_iNumPlayers <= MAX_PLAYERS )
	{
		for (int i = 0; i < m_iNumPlayers; i++)
			m_iClients[i] = READ_SHORT();

		int iWinningScore = READ_BYTE();
		int iLosingScore = READ_BYTE();
		int iBattleOver = READ_BYTE();

		// Battle over?
		if ( iBattleOver )
		{
			GetClientList( sz );

			strncpy( szTemp2, CHudTextMessage::BufferedLocaliseTextString( "#Round_Won_Scores" ), sizeof( szTemp2 ) - 1 );
			szTemp2[ sizeof( szTemp2 ) - 1 ] = '\0';			

			_snprintf( sz, sizeof( sz ) - 1, szTemp2, sz, iWinningScore, iLosingScore );
		}
		// Tied?
		else if ( iWinningScore == iLosingScore )
		{
			strncpy( szTemp2, CHudTextMessage::BufferedLocaliseTextString( "#Round_Tied" ), sizeof( szTemp2 ) - 1 );
			szTemp2[ sizeof( szTemp2 ) - 1 ] = '\0';			

			_snprintf( sz, sizeof( sz ) - 1, szTemp2, iWinningScore );
		}
		else 
		{
			char *pszTemp = NULL;

			GetClientList( sz );

			if ( m_iNumPlayers == 1 )
			{
				pszTemp = "#Round_Leads";
			}
			else
			{
				pszTemp = "#Round_Lead";
			}

			strncpy( szTemp2, CHudTextMessage::BufferedLocaliseTextString( pszTemp ), sizeof( szTemp2 ) - 1 );
			szTemp2[ sizeof( szTemp2 ) - 1 ] = '\0';			

			_snprintf( sz, sizeof( sz ) - 1, szTemp2, sz, iWinningScore, iLosingScore );
		}

		sz[ sizeof( sz ) - 1 ] = '\0';
		m_pWinningTeam->setText( sz );
	}

	// Become visible
	setVisible(true);

	// Hide the other windows if it's up
	if (gViewPort)
	{
		gViewPort->m_pSpectatorMenu->setVisible( false );
		gViewPort->m_pDiscPowerupWindow->setVisible( false );
		gViewPort->m_pDiscStartRound->setVisible( false );
	}
}

//===========================================================
// Powerup name window
CDiscPowerups::CDiscPowerups() : CTransparentPanel( 255, POWERUP_X, POWERUP_Y, POWERUP_SIZE_X, POWERUP_SIZE_Y )
{
	m_pLabel = new Label( "Powerups Go Here", 0, ROUND_Y, getWide(), YRES(32) );
	m_pLabel->setParent( this );
	m_pLabel->setBgColor( 0, 0, 0, 255 );
	m_pLabel->setFgColor( 255,255,255, 0 );
	m_pLabel->setContentAlignment( vgui::Label::a_center );
	setVisible(false);
};

void CDiscPowerups::RecalculateText( int iPowerup )
{
	char sz[512];
	bool bAnd = false;

	// Don't appear if a round message is up
	if (gViewPort)
	{
		if ( gViewPort->m_pDiscStartRound->isVisible() || gViewPort->m_pDiscEndRound->isVisible() )
			return;
	}

	sprintf(sz, "");

	if ( iPowerup & POW_TRIPLE )
	{
		strcat(sz, CHudTextMessage::BufferedLocaliseTextString("#Triple") );
		bAnd = true;
	}
	
	if ( iPowerup & POW_FAST )
	{
		if (bAnd)
			strcat(sz, ", ");
		strcat(sz, CHudTextMessage::BufferedLocaliseTextString("#Fast") );
		bAnd = true;
	}
	
	if ( iPowerup & POW_FREEZE )
	{
		if (bAnd)
			strcat(sz, ", ");
		strcat(sz, CHudTextMessage::BufferedLocaliseTextString("#Freeze") );
		bAnd = true;
	}
	
	if ( iPowerup & POW_HARD )
	{
		if (bAnd)
			strcat(sz, ", ");
		strcat(sz, CHudTextMessage::BufferedLocaliseTextString("#Hard") );
	}

	m_pLabel->setText( sz );

	// Become visible
	if (sz && sz[0])
		setVisible(true);
	else
		setVisible(false);
}

//===========================================================
// Reward menu
CDiscRewards::CDiscRewards() : CTransparentPanel( 255, REWARD_X, REWARD_Y, REWARD_SIZE_X, REWARD_SIZE_Y )
{
	m_pReward = new Label( "Well Done!", 0, ROUND_Y, getWide(), (REWARD_SIZE_Y / 2) );
	m_pReward->setParent( this );
	m_pReward->setBgColor( 0, 0, 0, 255 );
	m_pReward->setFgColor( 255,255,255, 0 );
	m_pReward->setContentAlignment( vgui::Label::a_center );
	setVisible(false);

	m_pTeleBonus = new Label( CHudTextMessage::BufferedLocaliseTextString( "#Hit_Tele" ), 0, (REWARD_SIZE_Y / 2), getWide(), (REWARD_SIZE_Y / 2) );
	m_pTeleBonus->setParent( this );
	m_pTeleBonus->setBgColor( 0, 0, 0, 255 );
	m_pTeleBonus->setFgColor( 255,255,255, 0 );
	m_pTeleBonus->setContentAlignment( vgui::Label::a_center );
};

void CDiscRewards::RecalculateText( int iReward )
{
	char sz[512];

	// Don't appear if a round message is up
	if (gViewPort)
	{
		if ( gViewPort->m_pDiscStartRound->isVisible() || gViewPort->m_pDiscEndRound->isVisible() )
			return;
	}

	if ( !iReward )
	{
		setVisible( false );
		return;
	}

	// Rewards
	if ( iReward & REWARD_BOUNCE_NONE )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_Direct" ) );
	if ( iReward & REWARD_BOUNCE_ONE )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_One" ) );
	if ( iReward & REWARD_BOUNCE_TWO )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_Two" ) );
	if ( iReward & REWARD_BOUNCE_THREE )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_Three" ) );
	if ( iReward & REWARD_DECAPITATE )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_Decap" ) );
	if ( iReward & REWARD_DOUBLEKILL )
		sprintf( sz, CHudTextMessage::BufferedLocaliseTextString( "#Hit_Multiple" ) );

	if ( iReward & REWARD_TELEPORT )
		m_pTeleBonus->setVisible( true );
	else
		m_pTeleBonus->setVisible( false );

	m_pReward->setText( sz );
	setVisible( true );
}

void CDiscRewards::SetMessage( char *pMessage )
{
	if (!pMessage)
	{
		setVisible(false);
		return;
	}

	m_pTeleBonus->setVisible( false );
	m_pReward->setText( pMessage );
	setVisible( true );

	if (gViewPort)
		gViewPort->m_flRewardOpenTime = gHUD.m_flTime + 5.0;
}