//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// vgui_SpectatorPanel.cpp: implementation of the SpectatorPanel class.
//
//////////////////////////////////////////////////////////////////////

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "pm_shared.h"
#include "vgui_viewport.h"
#include "vgui_SpectatorPanel.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SpectatorPanel::SpectatorPanel(int x,int y,int wide,int tall) : Panel(x,y,wide,tall)
{
}

SpectatorPanel::~SpectatorPanel()
{

}

void SpectatorPanel::ActionSignal(int cmd)
{
	switch (cmd)
	{
		case SPECTATOR_PANEL_CMD_NONE :			break;

		case SPECTATOR_PANEL_CMD_OPTIONS :		gViewPort->ShowCommandMenu( gViewPort->m_SpectatorMenu );
												break;

		case SPECTATOR_PANEL_CMD_NEXTPLAYER :	gHUD.m_Spectator.FindNextPlayer(true);
												break;

		case SPECTATOR_PANEL_CMD_PREVPLAYER :	gHUD.m_Spectator.FindNextPlayer(false);
												break;

		case SPECTATOR_PANEL_CMD_HIDEMENU	:	ShowMenu(false); 
												break;



		case SPECTATOR_PANEL_CMD_TOGGLE_INSET : gHUD.m_Spectator.SetModes( -1, 
													gHUD.m_Spectator.ToggleInset(false) );
												break;
		

		default : 	gEngfuncs.Con_DPrintf("Unknown SpectatorPanel ActionSingal %i.\n",cmd); break;
	}

}


void SpectatorPanel::Initialize()
{
	int x,y,wide,tall;
	
	getBounds(x,y,wide,tall);

	CSchemeManager * pSchemes = gViewPort->GetSchemeManager();

	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	
	m_TopBorder = new CTransparentPanel(64, 0, 0, ScreenWidth, YRES(32));
	m_TopBorder->setParent(this);

	m_BottomBorder = new CTransparentPanel(64, 0, ScreenHeight - YRES(32), ScreenWidth, YRES(32));
	m_BottomBorder->setParent(this);

	setPaintBackgroundEnabled(false);

	// Initialize the bottom title.
	m_BottomMainLabel = new Label( "Spectator Bottom", XRES(6+64+6+24+6), YRES(4), XRES(428), YRES(24) );
	m_BottomMainLabel->setParent(m_BottomBorder);
	m_BottomMainLabel->setFont( pSchemes->getFont( hTitleScheme ) );
	m_BottomMainLabel->setPaintBackgroundEnabled(false);
	m_BottomMainLabel->setFgColor( Scheme::sc_primary1 );
	m_BottomMainLabel->setContentAlignment( vgui::Label::a_center );

	LineBorder * border = new LineBorder(1, Scheme::sc_secondary1);
	m_BottomMainLabel->setBorder(border);
	m_BottomMainLabel->setPaintBorderEnabled(true);

	// Initialize the top title.
	m_TopMainLabel = new Label( "Spectator Top", 0, 0, wide, YRES(32) );
	m_TopMainLabel->setParent(m_TopBorder);
	m_TopMainLabel->setFont( pSchemes->getFont( hTitleScheme ) );
	m_TopMainLabel->setPaintBackgroundEnabled(false);
	m_TopMainLabel->setFgColor( Scheme::sc_primary1 );
	m_TopMainLabel->setContentAlignment( vgui::Label::a_center );
	
	// Initialize command buttons.
	m_OptionButton = new CommandButton("Options", XRES(6), YRES(6), XRES(64), YRES(20) );
	m_OptionButton->setParent( m_BottomBorder );
	m_OptionButton->setContentAlignment( vgui::Label::a_center );
	m_OptionButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_OptionButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_OPTIONS) );

	m_PrevPlayerButton= new CommandButton("<<", XRES(6+64+6), YRES(6), XRES(24), YRES(20) );
	m_PrevPlayerButton->setParent( m_BottomBorder );
	m_PrevPlayerButton->setContentAlignment( vgui::Label::a_center );
	m_PrevPlayerButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_PrevPlayerButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_PREVPLAYER) );

	m_NextPlayerButton= new CommandButton(">>", XRES(640-6-64-6-24), YRES(6), XRES(24), YRES(20) );
	m_NextPlayerButton->setParent( m_BottomBorder );
	m_NextPlayerButton->setContentAlignment( vgui::Label::a_center );
	m_NextPlayerButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_NextPlayerButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_NEXTPLAYER) );

	m_HideButton = new CommandButton("Hide", XRES(640-6-64), YRES(6), XRES(64), YRES(20) );
	m_HideButton->setParent( m_BottomBorder );
	m_HideButton->setContentAlignment( vgui::Label::a_center );
	m_HideButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_HideButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_HIDEMENU) );


	m_InsetViewButton = new CommandButton("", XRES(2), YRES(2), XRES(240), YRES(180) );
	m_InsetViewButton->setParent( this );
	m_InsetViewButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_TOGGLE_INSET) );


	m_menuVisible = false;
	m_HideButton->setVisible(false);
	m_OptionButton->setVisible(false);
	m_NextPlayerButton->setVisible(false);
	m_PrevPlayerButton->setVisible(false);
	m_BottomMainLabel->setPaintBorderEnabled(false);
	m_TopMainLabel->setVisible(false);
	
}

void SpectatorPanel::ShowMenu(bool isVisible)
{
	m_HideButton->setVisible(isVisible);
	m_OptionButton->setVisible(isVisible);
	m_NextPlayerButton->setVisible(isVisible);
	m_PrevPlayerButton->setVisible(isVisible);
	m_BottomMainLabel->setPaintBorderEnabled(isVisible);
	m_TopMainLabel->setVisible(isVisible);

	if ( !isVisible )
	{
		gViewPort->HideCommandMenu();

		// if switching from visible menu to invisible menu, show help text
		if ( m_menuVisible && this->isVisible() )
		{
			char string[ 64 ];

			_snprintf( string, sizeof( string ) - 1, "%c%s", HUD_PRINTCENTER, CHudTextMessage::BufferedLocaliseTextString( "#Spec_Duck" ) );
			string[ sizeof( string ) - 1 ] = '\0';

			gHUD.m_TextMessage.MsgFunc_TextMsg( NULL, strlen( string ) + 1, string );
		}
	}

	m_menuVisible = isVisible;

	gViewPort->UpdateCursorState();
}

void SpectatorPanel::EnableInsetView(bool isEnabled)
{
	int x = gHUD.m_Spectator.m_OverviewData.insetWindowX;
	int y = gHUD.m_Spectator.m_OverviewData.insetWindowY;
	int wide = gHUD.m_Spectator.m_OverviewData.insetWindowWidth;
	int tall = gHUD.m_Spectator.m_OverviewData.insetWindowHeight;

	if ( isEnabled )
	{
		// short black bar to see full inset
		m_TopBorder->setBounds(	XRES(x+wide+2), 0, XRES(640 - (x+wide+2) ), YRES(32) );

		m_TopMainLabel->setBounds(	0, 0, XRES(640 - (x+wide+2)), YRES(32) );

		m_InsetViewButton->setBounds(	XRES( x ), YRES( y ), 
										XRES( wide ), YRES( tall ) );
		m_InsetViewButton->setVisible(true);
	}
	else
	{
		// full black bar, no inset border
		m_TopBorder->setBounds( 0, 0, ScreenWidth, YRES(32) );
		m_TopMainLabel->setBounds( 0, 0, ScreenWidth, YRES(32) );
		m_InsetViewButton->setVisible(false);
	}
}



