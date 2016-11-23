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
#include "vgui_TeamFortressViewport.h"
#include "vgui_SpectatorPanel.h"
#include "vgui_scorepanel.h"

#define PANEL_HEIGHT 32


#define BANNER_WIDTH	256
#define BANNER_HEIGHT	64


#define OPTIONS_BUTTON_X 96
#define CAMOPTIONS_BUTTON_X 200

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

		case SPECTATOR_PANEL_CMD_OPTIONS :		gViewPort->ShowCommandMenu( gViewPort->m_SpectatorOptionsMenu );
												break;

		case SPECTATOR_PANEL_CMD_NEXTPLAYER :	gHUD.m_Spectator.FindNextPlayer(true);
												break;

		case SPECTATOR_PANEL_CMD_PREVPLAYER :	gHUD.m_Spectator.FindNextPlayer(false);
												break;

		case SPECTATOR_PANEL_CMD_HIDEMENU	:	ShowMenu(false); 
												break;

		case SPECTATOR_PANEL_CMD_CAMERA :		gViewPort->ShowCommandMenu( gViewPort->m_SpectatorCameraMenu );
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

	SchemeHandle_t hSmallScheme = pSchemes->getSchemeHandle( "Team Info Text" );
	
	m_TopBorder = new CTransparentPanel(64, 0, 0, ScreenWidth, YRES(PANEL_HEIGHT));
	m_TopBorder->setParent(this);

	m_BottomBorder = new CTransparentPanel(64, 0, ScreenHeight - YRES(32), ScreenWidth, YRES(PANEL_HEIGHT));
	m_BottomBorder->setParent(this);

	setPaintBackgroundEnabled(false);

	m_ExtraInfo = new Label( "Extra Info", 0, 0, wide, YRES(PANEL_HEIGHT) );
	m_ExtraInfo->setParent(m_TopBorder);
	m_ExtraInfo->setFont( pSchemes->getFont(hSmallScheme) );

	m_ExtraInfo->setPaintBackgroundEnabled(false);
	m_ExtraInfo->setFgColor( 143, 143, 54, 0 );
	m_ExtraInfo->setContentAlignment( vgui::Label::a_west );

	

	m_TimerImage = new CImageLabel( "timer", 0, 0, 14, 14 );
	m_TimerImage->setParent(m_TopBorder);

	m_TopBanner = new CImageLabel( "banner", 0, 0, XRES(BANNER_WIDTH), YRES(BANNER_HEIGHT) );
	m_TopBanner->setParent(this);

	m_CurrentTime = new Label( "00:00", 0, 0, wide, YRES(PANEL_HEIGHT) );
	m_CurrentTime->setParent(m_TopBorder);
	m_CurrentTime->setFont( pSchemes->getFont(hSmallScheme) );
	m_CurrentTime->setPaintBackgroundEnabled(false);
	m_CurrentTime->setFgColor( 143, 143, 54, 0 );
	m_CurrentTime->setContentAlignment( vgui::Label::a_west );

	m_Separator = new Panel( 0, 0, XRES( 64 ), YRES( 96 ));
	m_Separator->setParent( m_TopBorder );
	m_Separator->setFgColor( 59, 58, 34, 48 );
	m_Separator->setBgColor( 59, 58, 34, 48 );
	
	for ( int j= 0; j < TEAM_NUMBER; j++ )
	{
		m_TeamScores[j] = new Label( "   ", 0, 0, wide, YRES(PANEL_HEIGHT) );
		m_TeamScores[j]->setParent( m_TopBorder );
		m_TeamScores[j]->setFont( pSchemes->getFont(hSmallScheme) );
		m_TeamScores[j]->setPaintBackgroundEnabled(false);
		m_TeamScores[j]->setFgColor( 143, 143, 54, 0 );
		m_TeamScores[j]->setContentAlignment( vgui::Label::a_west );
		m_TeamScores[j]->setVisible ( false );
	}
	
	
	// Initialize command buttons.
	m_OptionButton = new ColorButton( CHudTextMessage::BufferedLocaliseTextString( "#SPECT_OPTIONS" ), XRES(15), YRES(6), XRES(OPTIONS_BUTTON_X), YRES(20), false, false );
	m_OptionButton->setParent( m_BottomBorder );
	m_OptionButton->setContentAlignment( vgui::Label::a_center );
	m_OptionButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_OptionButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_OPTIONS) );
	m_OptionButton->setUnArmedBorderColor ( 59, 58, 34, 48 );
	m_OptionButton->setArmedBorderColor ( 194, 202, 54, 0 );
	m_OptionButton->setUnArmedColor ( 143, 143, 54, 0 );
	m_OptionButton->setArmedColor ( 194, 202, 54, 0 );

	m_CamButton = new ColorButton( CHudTextMessage::BufferedLocaliseTextString( "#CAM_OPTIONS" ),  ScreenWidth - ( XRES ( CAMOPTIONS_BUTTON_X ) + 15 ), YRES(6), XRES ( CAMOPTIONS_BUTTON_X ), YRES(20), false, false );
	m_CamButton->setParent( m_BottomBorder );
	m_CamButton->setContentAlignment( vgui::Label::a_center );
	m_CamButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_CamButton->addActionSignal( new CSpectatorHandler_Command( this, SPECTATOR_PANEL_CMD_CAMERA ) );
	m_CamButton->setUnArmedBorderColor ( 59, 58, 34, 48 );
	m_CamButton->setArmedBorderColor ( 194, 202, 54, 0 );
	m_CamButton->setUnArmedColor ( 143, 143, 54, 0 );
	m_CamButton->setArmedColor ( 194, 202, 54, 0 );

	m_PrevPlayerButton= new ColorButton("<", XRES( 15 + OPTIONS_BUTTON_X + 15 ), YRES(6), XRES(24), YRES(20), false, false );
	m_PrevPlayerButton->setParent( m_BottomBorder );
	m_PrevPlayerButton->setContentAlignment( vgui::Label::a_center );
	m_PrevPlayerButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_PrevPlayerButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_PREVPLAYER) );
	m_PrevPlayerButton->setUnArmedBorderColor ( 59, 58, 34, 48 );
	m_PrevPlayerButton->setArmedBorderColor ( 194, 202, 54, 0 );
	m_PrevPlayerButton->setUnArmedColor ( 143, 143, 54, 0 );
	m_PrevPlayerButton->setArmedColor ( 194, 202, 54, 0 );

	m_NextPlayerButton= new ColorButton(">", (ScreenWidth - (XRES ( CAMOPTIONS_BUTTON_X ) + 15)) - XRES ( 24 + 15 ), YRES(6), XRES(24), YRES(20),false, false );
	m_NextPlayerButton->setParent( m_BottomBorder );
	m_NextPlayerButton->setContentAlignment( vgui::Label::a_center );
	m_NextPlayerButton->setBoundKey( (char)255 );	// special no bound to avoid leading spaces in name 
	m_NextPlayerButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_NEXTPLAYER) );
	m_NextPlayerButton->setUnArmedBorderColor ( 59, 58, 34, 48 );
	m_NextPlayerButton->setArmedBorderColor ( 194, 202, 54, 0 );
	m_NextPlayerButton->setUnArmedColor ( 143, 143, 54, 0 );
	m_NextPlayerButton->setArmedColor ( 194, 202, 54, 0 );
	
	// Initialize the bottom title.

	float flLabelSize = ( (ScreenWidth - (XRES ( CAMOPTIONS_BUTTON_X ) + 15)) - XRES ( 24 + 15 ) ) - XRES( (15 + OPTIONS_BUTTON_X + 15) + 38 );

	m_BottomMainLabel = new Label( "Spectator Bottom", XRES( ( 15 + OPTIONS_BUTTON_X + 15 ) + 31 ), YRES(6), flLabelSize, YRES(20) );
	m_BottomMainLabel->setParent(m_BottomBorder);
	m_BottomMainLabel->setPaintBackgroundEnabled(false);
	m_BottomMainLabel->setFgColor( Scheme::sc_primary1 );
	m_BottomMainLabel->setContentAlignment( vgui::Label::a_center );
	m_BottomMainLabel->setBorder( new LineBorder( Color( 59, 58, 34, 48 ) ) );

	m_InsetViewButton = new ColorButton("", XRES(2), YRES(2), XRES(240), YRES(180), false, false );
	m_InsetViewButton->setParent( this );
	m_InsetViewButton->setBoundKey( (char)255 );
	m_InsetViewButton->addActionSignal( new CSpectatorHandler_Command(this,SPECTATOR_PANEL_CMD_TOGGLE_INSET) );
	m_InsetViewButton->setUnArmedBorderColor ( 59, 58, 34, 48 );
	m_InsetViewButton->setArmedBorderColor ( 194, 202, 54, 0 );
	m_InsetViewButton->setUnArmedColor ( 143, 143, 54, 0 );
	m_InsetViewButton->setArmedColor ( 194, 202, 54, 0 );


	m_menuVisible = false;
	m_insetVisible = false;
//	m_HideButton->setVisible(false);
	m_CamButton->setVisible(false);
	m_OptionButton->setVisible(false);
	m_NextPlayerButton->setVisible(false);
	m_PrevPlayerButton->setVisible(false);
	m_TopBanner->setVisible( false );
	m_ExtraInfo->setVisible( false );
	m_Separator->setVisible( false );
	m_TimerImage->setVisible( false );
		
}

void SpectatorPanel::ShowMenu(bool isVisible)
{
//	m_HideButton->setVisible(isVisible);	m_HideButton->setArmed( false );
	m_OptionButton->setVisible(isVisible);		m_OptionButton->setArmed( false );
	m_CamButton->setVisible(isVisible);			m_CamButton->setArmed( false );
	m_NextPlayerButton->setVisible(isVisible);	m_NextPlayerButton->setArmed( false );
	m_PrevPlayerButton->setVisible(isVisible);	m_PrevPlayerButton->setArmed( false );
	
	if ( !isVisible )
	{
		int iLabelSizeX, iLabelSizeY;
		m_BottomMainLabel->getSize( iLabelSizeX, iLabelSizeY );
		m_BottomMainLabel->setPos( ( ScreenWidth / 2 ) - (iLabelSizeX/2), YRES(6) );
	}
	else
		m_BottomMainLabel->setPos( XRES( ( 15 + OPTIONS_BUTTON_X + 15 ) + 31 ), YRES(6) );

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


const char *GetSpectatorLabel ( int iMode )
{
	switch ( iMode )
	{
		case OBS_CHASE_LOCKED:
			return "#OBS_CHASE_LOCKED";

		case OBS_CHASE_FREE:
			return "#OBS_CHASE_FREE";

		case OBS_ROAMING:
			return "#OBS_ROAMING";
		
		case OBS_IN_EYE:
			return "#OBS_IN_EYE";

		case OBS_MAP_FREE:
			return "#OBS_MAP_FREE";

		case OBS_MAP_CHASE:
			return "#OBS_MAP_CHASE";

		case OBS_NONE:
		default:
			return "#OBS_NONE";
	}

	return "";
}

void SpectatorPanel::EnableInsetView(bool isEnabled)
{
	int x = gHUD.m_Spectator.m_OverviewData.insetWindowX;
	int y = gHUD.m_Spectator.m_OverviewData.insetWindowY;
	int wide = gHUD.m_Spectator.m_OverviewData.insetWindowWidth;
	int tall = gHUD.m_Spectator.m_OverviewData.insetWindowHeight;
	int offset = x + wide + 2;
	
	if ( isEnabled )
	{
		// short black bar to see full inset
		m_TopBorder->setBounds(	XRES(offset), 0, XRES(640 - offset ), YRES(PANEL_HEIGHT) );

		if ( gEngfuncs.IsSpectateOnly() )
		{
			m_TopBanner->setVisible( true );
			m_TopBanner->setPos( XRES(offset), 0 );
		}
		else
			m_TopBanner->setVisible( false );
		
		m_InsetViewButton->setBounds(	XRES( x ), YRES( y ), 
										XRES( wide ), YRES( tall ) );
		m_InsetViewButton->setVisible(true);
	}
	else
	{	
		// full black bar, no inset border
		// show banner only in real HLTV mode
		if ( gEngfuncs.IsSpectateOnly() )
		{
			m_TopBanner->setVisible( true );
			m_TopBanner->setPos( 0,0 );
		}
		else
			m_TopBanner->setVisible( false );

		m_TopBorder->setBounds( 0, 0, ScreenWidth, YRES(PANEL_HEIGHT) );
						
		m_InsetViewButton->setVisible(false);
	}

	m_insetVisible = isEnabled;

	Update();

	m_CamButton->setText( CHudTextMessage::BufferedLocaliseTextString( GetSpectatorLabel( g_iUser1 ) ) );
}




void SpectatorPanel::Update()
{
	int iTextWidth, iTextHeight;
	int iTimeHeight, iTimeWidth;
	int offset,j;

	if ( m_insetVisible )
		offset = gHUD.m_Spectator.m_OverviewData.insetWindowX + gHUD.m_Spectator.m_OverviewData.insetWindowWidth + 2;
	else
		offset = 0;

	bool visible = gHUD.m_Spectator.m_drawstatus->value != 0;
	
	m_ExtraInfo->setVisible( visible );
	m_TimerImage->setVisible( visible );
	m_CurrentTime->setVisible( visible );
	m_Separator->setVisible( visible );

	for ( j= 0; j < TEAM_NUMBER; j++ )
		m_TeamScores[j]->setVisible( visible );

	if ( !visible )
		return;
		
	m_ExtraInfo->getTextSize( iTextWidth, iTextHeight );
	m_CurrentTime->getTextSize( iTimeWidth, iTimeHeight );

	iTimeWidth += XRES ( 14 ); // +timer icon
	iTimeWidth += ( 4-(iTimeWidth%4) );

	if ( iTimeWidth > iTextWidth )
		iTextWidth = iTimeWidth;

	int xPos = ScreenWidth - ( iTextWidth + XRES ( 4 + offset ) );

	m_ExtraInfo->setBounds( xPos, YRES( 1 ), iTextWidth, iTextHeight );

	m_TimerImage->setBounds( xPos, YRES( 2 ) + iTextHeight , XRES(14), YRES(14) );
	
	m_CurrentTime->setBounds( xPos + XRES ( 14 + 1 ), YRES( 2 ) + iTextHeight , iTimeWidth, iTimeHeight );

	m_Separator->setPos( ScreenWidth - ( iTextWidth + XRES ( 4+2+4+offset ) ) , YRES( 1 ) );
	m_Separator->setSize( XRES( 4 ), YRES( PANEL_HEIGHT - 2 ) );

	for ( j= 0; j < TEAM_NUMBER; j++ )
	{
		int iwidth, iheight;
			
		m_TeamScores[j]->getTextSize( iwidth, iheight );
		m_TeamScores[j]->setBounds( ScreenWidth - ( iTextWidth + XRES ( 4+2+4+2+offset ) + iwidth ), YRES( 1 ) + ( iheight * j ), iwidth, iheight );
	}
}
