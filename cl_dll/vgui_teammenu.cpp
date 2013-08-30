//=========== (C) Copyright 1996-2002 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: TFC Team Menu
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================

#include "vgui_int.h"
#include "VGUI_Font.h"
#include "VGUI_ScrollPanel.h"
#include "VGUI_TextImage.h"

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

// Team Menu Dimensions
#define TEAMMENU_TITLE_X				XRES(40)
#define TEAMMENU_TITLE_Y				YRES(32)
#define TEAMMENU_TOPLEFT_BUTTON_X		XRES(40)
#define TEAMMENU_TOPLEFT_BUTTON_Y		YRES(80)
#define TEAMMENU_BUTTON_SIZE_X			XRES(124)
#define TEAMMENU_BUTTON_SIZE_Y			YRES(24)
#define TEAMMENU_BUTTON_SPACER_Y		YRES(8)
#define TEAMMENU_WINDOW_X				XRES(176)
#define TEAMMENU_WINDOW_Y				YRES(80)
#define TEAMMENU_WINDOW_SIZE_X			XRES(424)
#define TEAMMENU_WINDOW_SIZE_Y			YRES(312)
#define TEAMMENU_WINDOW_TITLE_X			XRES(16)
#define TEAMMENU_WINDOW_TITLE_Y			YRES(16)
#define TEAMMENU_WINDOW_TEXT_X			XRES(16)
#define TEAMMENU_WINDOW_TEXT_Y			YRES(48)
#define TEAMMENU_WINDOW_TEXT_SIZE_Y		YRES(178)
#define TEAMMENU_WINDOW_INFO_X			XRES(16)
#define TEAMMENU_WINDOW_INFO_Y			YRES(234)
      
// Creation
CTeamMenuPanel::CTeamMenuPanel(int iTrans, int iRemoveMe, int x,int y,int wide,int tall) : CMenuPanel(iTrans, iRemoveMe, x,y,wide,tall)
{
	// Get the scheme used for the Titles
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	// schemes
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hTeamWindowText = pSchemes->getSchemeHandle( "Briefing Text" );
	SchemeHandle_t hTeamInfoText = pSchemes->getSchemeHandle( "Team Info Text" );

	// get the Font used for the Titles
	Font *pTitleFont = pSchemes->getFont( hTitleScheme );
	int r, g, b, a;

	// Create the title
	Label *pLabel = new Label( "", TEAMMENU_TITLE_X, TEAMMENU_TITLE_Y );
	pLabel->setParent( this );
	pLabel->setFont( pTitleFont );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	pLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	pLabel->setBgColor( r, g, b, a );
	pLabel->setContentAlignment( vgui::Label::a_west );
	pLabel->setText( "%s", gHUD.m_TextMessage.BufferedLocaliseTextString("#Title_SelectYourTeam"));

	// Create the Info Window
	m_pTeamWindow  = new CTransparentPanel( 255, TEAMMENU_WINDOW_X, TEAMMENU_WINDOW_Y, TEAMMENU_WINDOW_SIZE_X, TEAMMENU_WINDOW_SIZE_Y );
	m_pTeamWindow->setParent( this );
	m_pTeamWindow->setBorder( new LineBorder( Color(255*0.7,170*0.7,0,0 )) );

	// Create the Map Name Label
	m_pMapTitle = new Label( "", TEAMMENU_WINDOW_TITLE_X, TEAMMENU_WINDOW_TITLE_Y );
	m_pMapTitle->setFont( pTitleFont ); 
	m_pMapTitle->setParent( m_pTeamWindow );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	m_pMapTitle->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	m_pMapTitle->setBgColor( r, g, b, a );
	m_pMapTitle->setContentAlignment( vgui::Label::a_west );

	// Create the Scroll panel
	m_pScrollPanel = new CTFScrollPanel( TEAMMENU_WINDOW_TEXT_X, TEAMMENU_WINDOW_TEXT_Y, TEAMMENU_WINDOW_SIZE_X - (TEAMMENU_WINDOW_TEXT_X * 2), TEAMMENU_WINDOW_TEXT_SIZE_Y );
	m_pScrollPanel->setParent(m_pTeamWindow);
	m_pScrollPanel->setScrollBarVisible(false, false);

	// Create the Map Briefing panel
	m_pBriefing = new TextPanel("", 0,0, TEAMMENU_WINDOW_SIZE_X - TEAMMENU_WINDOW_TEXT_X, TEAMMENU_WINDOW_TEXT_SIZE_Y );
	m_pBriefing->setParent( m_pScrollPanel->getClient() );
	m_pBriefing->setFont( pSchemes->getFont(hTeamWindowText) );
	pSchemes->getFgColor( hTeamWindowText, r, g, b, a );
	m_pBriefing->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hTeamWindowText, r, g, b, a );
	m_pBriefing->setBgColor( r, g, b, a );

	m_pBriefing->setText( gHUD.m_TextMessage.BufferedLocaliseTextString("#Map_Description_not_available") );
	
	// Team Menu buttons
	for (int i = 1; i <= 5; i++)
	{
		char sz[256]; 

		int iYPos = TEAMMENU_TOPLEFT_BUTTON_Y + ( (TEAMMENU_BUTTON_SIZE_Y + TEAMMENU_BUTTON_SPACER_Y) * i );

		// Team button
		m_pButtons[i] = new CommandButton( "", TEAMMENU_TOPLEFT_BUTTON_X, iYPos, TEAMMENU_BUTTON_SIZE_X, TEAMMENU_BUTTON_SIZE_Y, true);
		m_pButtons[i]->setParent( this );
		m_pButtons[i]->setContentAlignment( vgui::Label::a_west );
		m_pButtons[i]->setVisible( false );

		// AutoAssign button uses special case
		if (i == 5)
		{
			m_pButtons[5]->setBoundKey( '5' );
			m_pButtons[5]->setText( gHUD.m_TextMessage.BufferedLocaliseTextString("#Team_AutoAssign") );
			m_pButtons[5]->setVisible( true );
		}

		// Create the Signals
		sprintf(sz, "jointeam %d", i);
		m_pButtons[i]->addActionSignal( new CMenuHandler_StringCommandWatch( sz, true ) );
		m_pButtons[i]->addInputSignal( new CHandler_MenuButtonOver(this, i) );

		// Create the Team Info panel
		m_pTeamInfoPanel[i] = new TextPanel("", TEAMMENU_WINDOW_INFO_X, TEAMMENU_WINDOW_INFO_Y, TEAMMENU_WINDOW_SIZE_X - TEAMMENU_WINDOW_INFO_X, TEAMMENU_WINDOW_SIZE_X - TEAMMENU_WINDOW_INFO_Y );
		m_pTeamInfoPanel[i]->setParent( m_pTeamWindow );
		m_pTeamInfoPanel[i]->setFont( pSchemes->getFont(hTeamInfoText) );
		m_pTeamInfoPanel[i]->setFgColor(	iTeamColors[i % iNumberOfTeamColors][0],
											iTeamColors[i % iNumberOfTeamColors][1],
											iTeamColors[i % iNumberOfTeamColors][2],
											0 );
		m_pTeamInfoPanel[i]->setBgColor( 0,0,0, 255 );
	}

	// Create the Cancel button
	m_pCancelButton = new CommandButton( CHudTextMessage::BufferedLocaliseTextString( "#Menu_Cancel" ), TEAMMENU_TOPLEFT_BUTTON_X, 0, TEAMMENU_BUTTON_SIZE_X, TEAMMENU_BUTTON_SIZE_Y);
	m_pCancelButton->setParent( this );
	m_pCancelButton->addActionSignal( new CMenuHandler_TextWindow(HIDE_TEXTWINDOW) );

	// Create the Spectate button
	m_pSpectateButton = new SpectateButton( CHudTextMessage::BufferedLocaliseTextString( "#Menu_Spectate" ), TEAMMENU_TOPLEFT_BUTTON_X, 0, TEAMMENU_BUTTON_SIZE_X, TEAMMENU_BUTTON_SIZE_Y, true);
	m_pSpectateButton->setParent( this );
	m_pSpectateButton->addActionSignal( new CMenuHandler_StringCommand( "spectate", true ) );
	m_pSpectateButton->setBoundKey( '6' );
	m_pSpectateButton->addInputSignal( new CHandler_MenuButtonOver(this, 6) );

	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose: Called each time a new level is started.
//-----------------------------------------------------------------------------
void CTeamMenuPanel::Initialize( void )
{
	m_bUpdatedMapName = false;
	m_iCurrentInfo = 0;
	m_pScrollPanel->setScrollValue( 0, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Called everytime the Team Menu is displayed
//-----------------------------------------------------------------------------
void CTeamMenuPanel::Update( void )
{
	int	 iYPos = TEAMMENU_TOPLEFT_BUTTON_Y;

	// Set the team buttons
	for (int i = 1; i <= 4; i++)
	{
		if (m_pButtons[i])
		{
			if ( i <= gViewPort->GetNumberOfTeams() )
			{
				m_pButtons[i]->setText( gViewPort->GetTeamName(i) );

				// bound key replacement
				char sz[32]; 
				sprintf( sz, "%d", i );
				m_pButtons[i]->setBoundKey( sz[0] );

				m_pButtons[i]->setVisible( true );
				m_pButtons[i]->setPos( TEAMMENU_TOPLEFT_BUTTON_X, iYPos );
				iYPos += TEAMMENU_BUTTON_SIZE_Y + TEAMMENU_BUTTON_SPACER_Y;

				// Start with the first option up
				if (!m_iCurrentInfo)
					SetActiveInfo( i );

				char szPlayerList[ (MAX_PLAYER_NAME_LENGTH + 3) * 31 ];  // name + ", "
				strcpy(szPlayerList, "\n");
				// Update the Team Info
				// Now count the number of teammembers of this class
				int iTotal = 0;
				for ( int j = 1; j < MAX_PLAYERS; j++ )
				{
					if ( g_PlayerInfoList[j].name == NULL )
						continue; // empty player slot, skip
					if ( g_PlayerInfoList[j].thisplayer )
						continue; // skip this player
					if ( g_PlayerExtraInfo[j].teamnumber != i )
						continue; // skip over players in other teams

					iTotal++;
					if (iTotal > 1)
						strncat( szPlayerList, ", ", sizeof(szPlayerList) - strlen(szPlayerList) );
					strncat( szPlayerList, g_PlayerInfoList[j].name, sizeof(szPlayerList) - strlen(szPlayerList) );
					szPlayerList[ sizeof(szPlayerList) - 1 ] = '\0';
				}

				if (iTotal > 0)
				{
					// Set the text of the info Panel
					char szText[ ((MAX_PLAYER_NAME_LENGTH + 3) * 31) + 256 ]; 
					if (iTotal == 1)
						sprintf(szText, "%s: %d Player (%d points)", gViewPort->GetTeamName(i), iTotal, g_TeamInfo[i].frags );
					else
						sprintf(szText, "%s: %d Players (%d points)", gViewPort->GetTeamName(i), iTotal, g_TeamInfo[i].frags );
					strncat( szText, szPlayerList, sizeof(szText) - strlen(szText) );
					szText[ sizeof(szText) - 1 ] = '\0';

					m_pTeamInfoPanel[i]->setText( szText );
				}
				else
				{
					m_pTeamInfoPanel[i]->setText( "" );
				}
			}
			else
			{
				// Hide the button (may be visible from previous maps)
				m_pButtons[i]->setVisible( false );
			}
		}
	}

	// Move the AutoAssign button into place
	m_pButtons[5]->setPos( TEAMMENU_TOPLEFT_BUTTON_X, iYPos );
	iYPos += TEAMMENU_BUTTON_SIZE_Y + TEAMMENU_BUTTON_SPACER_Y;

	// Spectate button
	if (m_pSpectateButton->IsNotValid())
	{
		m_pSpectateButton->setVisible( false );
	}
	else
	{
		m_pSpectateButton->setPos( TEAMMENU_TOPLEFT_BUTTON_X, iYPos );
		m_pSpectateButton->setVisible( true );
		iYPos += TEAMMENU_BUTTON_SIZE_Y + TEAMMENU_BUTTON_SPACER_Y;
	}
	
	// If the player is already in a team, make the cancel button visible
	if ( g_iTeamNumber )
	{
		m_pCancelButton->setPos( TEAMMENU_TOPLEFT_BUTTON_X, iYPos );
		iYPos += TEAMMENU_BUTTON_SIZE_Y + TEAMMENU_BUTTON_SPACER_Y;
		m_pCancelButton->setVisible( true );
	}
	else
	{
		m_pCancelButton->setVisible( false );
	}

	// Set the Map Title
	if (!m_bUpdatedMapName)
	{
		const char *level = gEngfuncs.pfnGetLevelName();
		if (level && level[0])
		{
			char sz[256]; 
			char szTitle[256]; 
			char *ch;

			// Update the level name
			strcpy( sz, level );
			ch = strchr( sz, '/' );
			if (!ch)
				ch = strchr( sz, '\\' );
			strcpy( szTitle, ch+1 );
			ch = strchr( szTitle, '.' );
			*ch = '\0';
			m_pMapTitle->setText( "%s", szTitle );
			*ch = '.';

			// Update the map briefing
			strcpy( sz, level );
			ch = strchr( sz, '.' );
			*ch = '\0';
			strcat( sz, ".txt" );
			char *pfile = (char*)gEngfuncs.COM_LoadFile( sz, 5, NULL );
			if (pfile)
			{
				m_pBriefing->setText( pfile );

				// Get the total size of the Briefing text and resize the text panel
				int iXSize, iYSize;
				m_pBriefing->getTextImage()->getTextSize( iXSize, iYSize );
				m_pBriefing->setSize( iXSize, iYSize );
				gEngfuncs.COM_FreeFile( pfile );
			}

			m_bUpdatedMapName = true;
		}
	}

	m_pScrollPanel->validate();
}

//=====================================
// Key inputs
bool CTeamMenuPanel::SlotInput( int iSlot )
{
	// Check for AutoAssign
	if ( iSlot == 5)
	{
		m_pButtons[5]->fireActionSignal();
		return true;
	}

	// Spectate
	if ( iSlot == 6)
	{
		m_pSpectateButton->fireActionSignal();
		return true;
	}

	// Otherwise, see if a particular team is selectable
	if ( (iSlot < 1) || (iSlot > gViewPort->GetNumberOfTeams()) )
		return false;
	if ( !m_pButtons[ iSlot ] )
		return false;

	// Is the button pushable?
	if ( m_pButtons[ iSlot ]->isVisible() )
	{
		m_pButtons[ iSlot ]->fireActionSignal();
		return true;
	}

	return false;
}

//======================================
// Update the Team menu before opening it
void CTeamMenuPanel::Open( void )
{
	Update();
	CMenuPanel::Open();
}

void CTeamMenuPanel::paintBackground()
{
	// make sure we get the map briefing up
	if ( !m_bUpdatedMapName )
		Update();

	CMenuPanel::paintBackground();
}

//======================================
// Mouse is over a team button, bring up the class info
void CTeamMenuPanel::SetActiveInfo( int iInput )
{
	// Remove all the Info panels and bring up the specified one
	m_pSpectateButton->setArmed( false );
	for (int i = 1; i <= 5; i++)
	{
		m_pButtons[i]->setArmed( false );
		m_pTeamInfoPanel[i]->setVisible( false );
	}

	// 6 is Spectate
	if (iInput == 6)
	{
		m_pSpectateButton->setArmed( true );
	}
	else
	{
		m_pButtons[iInput]->setArmed( true );
		m_pTeamInfoPanel[iInput]->setVisible( true );
	}

	m_iCurrentInfo = iInput;

	m_pScrollPanel->validate();
}
