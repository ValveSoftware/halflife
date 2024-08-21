//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Client DLL VGUI Viewport
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include<VGUI_Cursor.h>
#include<VGUI_Frame.h>
#include<VGUI_Label.h>
#include<VGUI_Surface.h>
#include<VGUI_BorderLayout.h>
#include<VGUI_Panel.h>
#include<VGUI_ImagePanel.h>
#include<VGUI_Button.h>
#include<VGUI_ActionSignal.h>
#include<VGUI_InputSignal.h>
#include<VGUI_MenuSeparator.h>
#include<VGUI_TextPanel.h>
#include<VGUI_LoweredBorder.h>
#include<VGUI_LineBorder.h>
#include<VGUI_Scheme.h>
#include<VGUI_Font.h>
#include<VGUI_App.h>
#include<VGUI_BuildGroup.h>

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
#include "keydefs.h"
#include "demo.h"
#include "demo_api.h"
#include "ammohistory.h"

#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ServerBrowser.h"
#include "vgui_ScorePanel.h"
#include "vgui_discobjects.h"

extern WeaponsResource gWR;
extern int g_iVisibleMouse;
class CCommandMenu;
int g_iPlayerClass;
int g_iTeamNumber;
int g_iUser1;
int g_iUser2;

// Scoreboard positions
#define SBOARD_INDENT_X			XRES(104)
#define SBOARD_INDENT_Y			YRES(40)

// low-res scoreboard indents
#define SBOARD_INDENT_X_512		30
#define SBOARD_INDENT_Y_512		30

#define SBOARD_INDENT_X_400		0
#define SBOARD_INDENT_Y_400		20

void IN_ResetMouse( void );
void IN_ResetRelativeMouseState(void);
extern CMenuPanel *CMessageWindowPanel_Create( const char *szMOTD, const char *szTitle, int iShadeFullscreen, int iRemoveMe, int x, int y, int wide, int tall );

using namespace vgui;

// Team Colors
int iTeamColors[5][3] =
{
	{ 255, 255, 255 },
	{ 66, 115, 247 },
	{ 220, 51, 38 },
	{ 240, 135, 0 },
	{ 115, 240, 115 },
};

// Used for Class specific buttons
char *sTFClasses[] =
{
	"",
	"SCOUT",
	"SNIPER",
	"SOLDIER",
	"DEMOMAN",
	"MEDIC",
	"HWGUY",
	"PYRO",
	"SPY",
	"ENGINEER",
	"CIVILIAN",
};

char *sLocalisedClasses[] = 
{
	"#Civilian",
	"#Scout",
	"#Sniper",
	"#Soldier",
	"#Demoman",
	"#Medic",
	"#HWGuy",
	"#Pyro",
	"#Spy",
	"#Engineer",
	"#Random",
	"#Civilian",
};

char *sTFClassSelection[] = 
{
	"civilian",
	"scout",
	"sniper",
	"soldier",
	"demoman",
	"medic",
	"hwguy",
	"pyro",
	"spy",
	"engineer",
	"randompc",
	"civilian",
};

int iBuildingCosts[] =
{
	BUILD_COST_DISPENSER,
	BUILD_COST_SENTRYGUN
};

// This maps class numbers to the Invalid Class bit.
// This is needed for backwards compatability in maps that were finished before
// all the classes were in TF. Hence the wacky sequence.
int sTFValidClassInts[] =
{
	0,
	TF_ILL_SCOUT,
	TF_ILL_SNIPER,
	TF_ILL_SOLDIER,
	TF_ILL_DEMOMAN,
	TF_ILL_MEDIC,
	TF_ILL_HVYWEP,
	TF_ILL_PYRO,
	TF_ILL_SPY,
	TF_ILL_ENGINEER,
	TF_ILL_RANDOMPC,
};

// Get the name of TGA file, based on GameDir
char* GetVGUITGAName(const char *pszName)
{
	int i;
	char sz[256]; 
	static char gd[256]; 
	const char *gamedir;

	if (ScreenWidth < 640)
		i = 320;
	else
		i = 640;
	sprintf(sz, pszName, i);

	gamedir = gEngfuncs.pfnGetGameDirectory();
	sprintf(gd, "%s/gfx/vgui/%s.tga",gamedir,sz);

	return gd;
}

//================================================================
// COMMAND MENU
//================================================================
void CCommandMenu::AddButton( CommandButton *pButton )
{
	if (m_iButtons >= MAX_BUTTONS)
		return;

	m_aButtons[m_iButtons] = pButton;
	m_iButtons++;
	pButton->setParent( this );
	pButton->setFont( Scheme::sf_primary3 );

	// give the button a default key binding
	if ( m_iButtons < 10 )
	{
		pButton->setBoundKey( m_iButtons + '0' );
	}
	else if ( m_iButtons == 10 )
	{
		pButton->setBoundKey( '0' );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tries to find a button that has a key bound to the input, and
//			presses the button if found
// Input  : keyNum - the character number of the input key
// Output : Returns true if the command menu should close, false otherwise
//-----------------------------------------------------------------------------
bool CCommandMenu::KeyInput( int keyNum )
{
	// loop through all our buttons looking for one bound to keyNum
	for ( int i = 0; i < m_iButtons; i++ )
	{
		if ( !m_aButtons[i]->IsNotValid() )
		{
			if ( m_aButtons[i]->getBoundKey() == keyNum )
			{
				// hit the button
				if ( m_aButtons[i]->GetSubMenu() )
				{
					// open the sub menu
					gViewPort->SetCurrentCommandMenu( m_aButtons[i]->GetSubMenu() );
					return false;
				}
				else
				{
					// run the bound command
					m_aButtons[i]->fireActionSignal();
					return true;
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: clears the current menus buttons of any armed (highlighted) 
//			state, and all their sub buttons
//-----------------------------------------------------------------------------
void CCommandMenu::ClearButtonsOfArmedState( void )
{
	for ( int i = 0; i < GetNumButtons(); i++ )
	{
		m_aButtons[i]->setArmed( false );

		if ( m_aButtons[i]->GetSubMenu() )
		{
			m_aButtons[i]->GetSubMenu()->ClearButtonsOfArmedState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSubMenu - 
// Output : CommandButton
//-----------------------------------------------------------------------------
CommandButton *CCommandMenu::FindButtonWithSubmenu( CCommandMenu *pSubMenu )
{
	for ( int i = 0; i < GetNumButtons(); i++ )
	{
		if ( m_aButtons[i]->GetSubMenu() == pSubMenu )
			return m_aButtons[i];
	}

	return NULL;
}

// Recalculate the visible buttons
bool CCommandMenu::RecalculateVisibles( int iNewYPos, bool bHideAll )
{
	int  iCurrentY = 0;
	int  iXPos, iYPos;
	bool bHasButton = false;

	if (iNewYPos)
		setPos( _pos[0], iNewYPos );

	// Cycle through all the buttons in this menu, and see which will be visible
	for (int i = 0; i < m_iButtons; i++)
	{
		int iClass = m_aButtons[i]->GetPlayerClass();
		if ( (iClass && iClass != g_iPlayerClass ) || ( m_aButtons[i]->IsNotValid() ) || bHideAll )
		{
			m_aButtons[i]->setVisible( false );
			if ( m_aButtons[i]->GetSubMenu() != NULL )
			{
				(m_aButtons[i]->GetSubMenu())->RecalculateVisibles( _pos[1] + iCurrentY, true );
			}
		}
		else
		{
 			// If it's got a submenu, force it to check visibilities
			if ( m_aButtons[i]->GetSubMenu() != NULL )
			{
				if ( !(m_aButtons[i]->GetSubMenu())->RecalculateVisibles( _pos[1] + iCurrentY, false ) )
				{
					// The submenu had no visible buttons, so don't display this button
					m_aButtons[i]->setVisible( false );
					continue;
				}
			}

			m_aButtons[i]->setVisible( true );

			// Make sure it's at the right Y position
			m_aButtons[i]->getPos( iXPos, iYPos );
			m_aButtons[i]->setPos( iXPos, iCurrentY );

			iCurrentY += BUTTON_SIZE_Y - 1;
			bHasButton = true;
		}
	}

	// Set Size
	setSize( _size[0], iCurrentY + 1 );

	return bHasButton;
}

// Make sure all submenus can fit on the screen
void CCommandMenu::RecalculatePositions( int iYOffset )
{
	int iNewYPos = _pos[1] + iYOffset;
	int iAdjust = 0;

	// Calculate if this is going to fit onscreen, and shuffle it up if it won't
	int iBottom = iNewYPos + _size[1];
	if ( iBottom > ScreenHeight )
	{
		// Move in increments of button sizes
		while (iAdjust < (iBottom - ScreenHeight))
		{
			iAdjust += BUTTON_SIZE_Y - 1;
		}
		iNewYPos -= iAdjust;

		// Make sure it doesn't move off the top of the screen (the menu's too big to fit it all)
		if ( iNewYPos < 0 )
		{
			iAdjust -= (0 - iNewYPos);
			iNewYPos = 0;
		}
	}

	// We need to force all menus below this one to update their positions now, because they
	// might have submenus riding off buttons in this menu that have just shifted.
	for (int i = 0; i < m_iButtons; i++)
		m_aButtons[i]->UpdateSubMenus( iAdjust );

	setPos( _pos[0], iNewYPos );
}


// Make this menu and all menus above it in the chain visible
void CCommandMenu::MakeVisible( CCommandMenu *pChildMenu )
{
/*
	// Push down the button leading to the child menu
	for (int i = 0; i < m_iButtons; i++)
	{
		if ( (pChildMenu != NULL) && (m_aButtons[i]->GetSubMenu() == pChildMenu) )
		{
			m_aButtons[i]->setArmed( true );
		}
		else
		{
			m_aButtons[i]->setArmed( false );
		}
	}
*/

	setVisible(true);
	if (m_pParentMenu)
		m_pParentMenu->MakeVisible( this );
}

//================================================================
// CreateSubMenu
CCommandMenu *TeamFortressViewport::CreateSubMenu( CommandButton *pButton, CCommandMenu *pParentMenu )
{
	int iXPos = 0;
	int iYPos = 0;
	int iWide = CMENU_SIZE_X;
	int iTall = 0;

	if (pParentMenu)
	{
		iXPos = pParentMenu->GetXOffset() + CMENU_SIZE_X - 1;
		iYPos = pParentMenu->GetYOffset() + BUTTON_SIZE_Y * (m_pCurrentCommandMenu->GetNumButtons() - 1);
	}

	CCommandMenu *pMenu = new CCommandMenu(pParentMenu, iXPos, iYPos, iWide, iTall );
	pMenu->setParent(this);
	pButton->AddSubMenu( pMenu );
	pButton->setFont( Scheme::sf_primary3 );

	// Create the Submenu-open signal
	InputSignal *pISignal = new CMenuHandler_PopupSubMenuInput(pButton, pMenu);
	pButton->addInputSignal(pISignal);

	// Put a > to show it's a submenu
	CImageLabel *pLabel = new CImageLabel( "arrow", CMENU_SIZE_X - SUBMENU_SIZE_X, SUBMENU_SIZE_Y );
	pLabel->setParent(pButton);
	pLabel->addInputSignal(pISignal);

	// Reposition
	pLabel->getPos( iXPos, iYPos );
	pLabel->setPos( CMENU_SIZE_X - pLabel->getImageWide(), (BUTTON_SIZE_Y - pLabel->getImageTall()) / 2 );

	// Create the mouse off signal for the Label too
	if (!pButton->m_bNoHighlight)
		pLabel->addInputSignal( new CHandler_CommandButtonHighlight(pButton) );

	return pMenu;
}

//-----------------------------------------------------------------------------
// Purpose: Makes sure the memory allocated for TeamFortressViewport is nulled out
// Input  : stAllocateBlock - 
// Output : void *
//-----------------------------------------------------------------------------
void *TeamFortressViewport::operator new( size_t stAllocateBlock )
{
//	void *mem = Panel::operator new( stAllocateBlock );
	void *mem = ::operator new( stAllocateBlock );
	memset( mem, 0, stAllocateBlock );
	return mem;
}

//-----------------------------------------------------------------------------
// Purpose: InputSignal handler for the main viewport
//-----------------------------------------------------------------------------
class CViewPortInputHandler : public InputSignal
{
public:
	bool bPressed;

	CViewPortInputHandler()
	{
	}

	virtual void cursorMoved(int x,int y,Panel* panel) {}
	virtual void cursorEntered(Panel* panel) {}
	virtual void cursorExited(Panel* panel) {}
	virtual void mousePressed(MouseCode code,Panel* panel) 
	{
		if ( code != MOUSE_LEFT )
		{
			// send a message to close the command menu
			// this needs to be a message, since a direct call screws the timing
			gEngfuncs.pfnClientCmd( "ForceCloseCommandMenu\n" );
		}
	}
	virtual void mouseReleased(MouseCode code,Panel* panel)
	{
	}

	virtual void mouseDoublePressed(MouseCode code,Panel* panel) {}
	virtual void mouseWheeled(int delta,Panel* panel) {}
	virtual void keyPressed(KeyCode code,Panel* panel) {}
	virtual void keyTyped(KeyCode code,Panel* panel) {}
	virtual void keyReleased(KeyCode code,Panel* panel) {}
	virtual void keyFocusTicked(Panel* panel) {}
};


//================================================================
TeamFortressViewport::TeamFortressViewport(int x,int y,int wide,int tall) : Panel(x,y,wide,tall), m_SchemeManager(wide,tall)
{
	gViewPort = this;
	m_iInitialized = false;
	m_pScoreBoard = NULL;
	m_pSpectatorMenu = NULL;
	m_pCurrentMenu = NULL;
	m_pCurrentCommandMenu = NULL;

	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round

	Initialize();
	addInputSignal( new CViewPortInputHandler );

	int r, g, b, a;
	
	Scheme* pScheme = App::getInstance()->getScheme();

	// primary text color
	// Get the colors
	//!! two different types of scheme here, need to integrate
	SchemeHandle_t hPrimaryScheme = m_SchemeManager.getSchemeHandle( "Primary Button Text" );
	{
		// font
		pScheme->setFont( Scheme::sf_primary1, m_SchemeManager.getFont(hPrimaryScheme) );

		// text color
		m_SchemeManager.getFgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor(Scheme::sc_primary1, r, g, b, a );		// sc_primary1 is non-transparent orange

		// background color (transparent black)
		m_SchemeManager.getBgColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor(Scheme::sc_primary3, r, g, b, a );

		// armed foreground color
		m_SchemeManager.getFgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor(Scheme::sc_secondary2, r, g, b, a );

		// armed background color
		m_SchemeManager.getBgArmedColor( hPrimaryScheme, r, g, b, a );
		pScheme->setColor(Scheme::sc_primary2, r, g, b, a );

		//!! need to get this color from scheme file
		// used for orange borders around buttons
		m_SchemeManager.getBorderColor( hPrimaryScheme, r, g, b, a );
		// pScheme->setColor(Scheme::sc_secondary1, r, g, b, a );
		pScheme->setColor(Scheme::sc_secondary1, 255*0.7, 170*0.7, 0, 0);
	}

	// Change the second primary font (used in the scoreboard)
	SchemeHandle_t hScoreboardScheme = m_SchemeManager.getSchemeHandle( "Scoreboard Text" );
	{
		pScheme->setFont(Scheme::sf_primary2, m_SchemeManager.getFont(hScoreboardScheme) );
	}
	
	// Change the third primary font (used in command menu)
	SchemeHandle_t hCommandMenuScheme = m_SchemeManager.getSchemeHandle( "CommandMenu Text" );
	{
		pScheme->setFont(Scheme::sf_primary3, m_SchemeManager.getFont(hCommandMenuScheme) );
	}

	App::getInstance()->setScheme(pScheme);

	// VGUI MENUS
	CreateScoreBoard();
	CreateCommandMenu();
	CreateServerBrowser();
	CreateSpectatorMenu();

	// Discwar
	CreateDiscIcons();
}

//-----------------------------------------------------------------------------
// Purpose: Called everytime a new level is started. Viewport clears out it's data.
//-----------------------------------------------------------------------------
void TeamFortressViewport::Initialize( void )
{
	// Force each menu to Initialize
	if (m_pScoreBoard)
	{
		m_pScoreBoard->Initialize();
		HideScoreBoard();
	}
	if (m_pSpectatorMenu)
	{
		// Spectator menu doesn't need initializing
		m_pSpectatorMenu->setVisible( false );
	}

	// Make sure all menus are hidden
	HideVGUIMenu();
	HideCommandMenu();

	// Clear out some data
	m_iGotAllMOTD = true;
	m_iRandomPC = false;
	m_flScoreBoardLastUpdated = 0;

	// reset player info
	g_iPlayerClass = 0;
	g_iTeamNumber = 0;

	strcpy(m_sMapName, "");
	strcpy(m_szServerName, "");
	for (int i = 0; i < 5; i++)
	{
		m_iValidClasses[i] = 0;
		strcpy(m_sTeamNames[i], "");
	}

	App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_none) );
}

class CException;
//-----------------------------------------------------------------------------
// Purpose: Read the Command Menu structure from the txt file and create the menu.
//-----------------------------------------------------------------------------
void TeamFortressViewport::CreateCommandMenu( void )
{
	// COMMAND MENU
	// Create the root of the Command Menu
	m_pCommandMenus[0] = new CCommandMenu(NULL, 0, CMENU_TOP, CMENU_SIZE_X, 300);	// This will be resized once we know how many items are in it
	m_pCommandMenus[0]->setParent(this);
	m_pCommandMenus[0]->setVisible(false);
	m_iNumMenus = 1;
	m_iCurrentTeamNumber = m_iUser1 = m_iUser2 = 0;

	// Read Command Menu from the txt file
	char token[1024];
	char *pfile = (char*)gEngfuncs.COM_LoadFile("commandmenu.txt", 5, NULL);
	if (!pfile)
	{
		gEngfuncs.Con_DPrintf( "Unable to open commandmenu.txt\n");
		SetCurrentCommandMenu( NULL );
		return;
	}

#ifdef _WIN32
try
{
#endif
	// First, read in the localisation strings

	// Detpack strings
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For5Seconds",   m_sDetpackStrings[0], MAX_BUTTON_SIZE );
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For20Seconds",   m_sDetpackStrings[1], MAX_BUTTON_SIZE );
	gHUD.m_TextMessage.LocaliseTextString( "#DetpackSet_For50Seconds",   m_sDetpackStrings[2], MAX_BUTTON_SIZE );

	// Now start parsing the menu structure
	m_pCurrentCommandMenu = m_pCommandMenus[0];
	char szLastButtonText[32] = "file start";
	pfile = gEngfuncs.COM_ParseFile(pfile, token);
	while ( ( strlen ( token ) > 0 ) && ( m_iNumMenus < MAX_MENUS ) )
	{
		// Keep looping until we hit the end of this menu
		while ( token[0] != '}' && ( strlen( token ) > 0 ) )
		{
			char cText[32] = "";
			char cBoundKey[32] = "";
			char cCustom[32] = "";
			static const int cCommandLength = 128;
			char cCommand[cCommandLength] = "";
			char szMap[MAX_MAPNAME] = "";
			int	 iPlayerClass = 0;
			int  iCustom = false;
			int  iTeamOnly = 0;
			bool bGetExtraToken = true;
			CommandButton *pButton = NULL;
			
			// We should never be here without a Command Menu
			if (!m_pCurrentCommandMenu)
			{
				gEngfuncs.Con_Printf("Error in Commandmenu.txt file after '%s'.\n", szLastButtonText );
				m_iInitialized = false;
				return;
			}

			// token should already be the bound key, or the custom name
			strncpy( cCustom, token, 32 );
			cCustom[31] = '\0';

			// See if it's a custom button
			if (!strcmp(cCustom, "CUSTOM") )
			{
				iCustom = true;

				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			// See if it's a map
			else if (!strcmp(cCustom, "MAP") )
			{
				// Get the mapname
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
				strncpy( szMap, token, MAX_MAPNAME );
				szMap[MAX_MAPNAME-1] = '\0';

				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			else if ( !strncmp(cCustom, "TEAM", 4) ) // TEAM1, TEAM2, TEAM3, TEAM4
			{
				// make it a team only button
				iTeamOnly = atoi( cCustom + 4 );
				
				// Get the next token
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
			else
			{
				// See if it's a Class
				for (int i = 1; i <= PC_ENGINEER; i++)
				{
					if ( !strcmp(token, sTFClasses[i]) )
					{
						// Save it off
						iPlayerClass = i;

						// Get the button text
						pfile = gEngfuncs.COM_ParseFile(pfile, token);
						break;
					}
				}
			}

			// Get the button bound key
			strncpy( cBoundKey, token, 32 );
			cText[31] = '\0';

			// Get the button text
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strncpy( cText, token, 32 );
			cText[31] = '\0';

			// save off the last button text we've come across (for error reporting)
			strcpy( szLastButtonText, cText );

			// Get the button command
			pfile = gEngfuncs.COM_ParseFile(pfile, token);
			strncpy( cCommand, token, cCommandLength );
			cCommand[cCommandLength - 1] = '\0';

			// Custom button handling
			if ( iCustom )
			{
				pButton = CreateCustomButton( cText, cCommand );

				// Get the next token to see if we're a menu
				pfile = gEngfuncs.COM_ParseFile(pfile, token);

				if ( token[0] == '{' )
				{
					strcpy( cCommand, token );
				}
				else
				{
					bGetExtraToken = false;
				}
			}
			else if ( szMap[0] != '\0' )
			{
				// create a map button
				pButton = new MapButton(szMap, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y);
			}
			else if ( iTeamOnly )
			{
				// button that only shows up if the player is on team iTeamOnly
				pButton = new TeamOnlyCommandButton( iTeamOnly, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y );
			}
			else
			{
				// normal button
				pButton = new CommandButton( iPlayerClass, cText,0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y );
			}

			// add the button into the command menu
			if ( pButton )
			{
				m_pCurrentCommandMenu->AddButton( pButton );
				pButton->setBoundKey( cBoundKey[0] );
				pButton->setParentMenu( m_pCurrentCommandMenu );

				// Override font in CommandMenu
				pButton->setFont( Scheme::sf_primary3 );
			}

			// Find out if it's a submenu or a button we're dealing with
			if ( cCommand[0] == '{' )
			{
				if ( m_iNumMenus >= MAX_MENUS )
				{
					gEngfuncs.Con_Printf( "Too many menus in commandmenu.txt past '%s'\n", szLastButtonText );
				}
				else
				{
					// Create the menu
					m_pCommandMenus[m_iNumMenus] = CreateSubMenu(pButton, m_pCurrentCommandMenu);
					m_pCurrentCommandMenu = m_pCommandMenus[m_iNumMenus];
					m_iNumMenus++;
				}
			}
			else if ( !iCustom )
			{
				// Create the button and attach it to the current menu
				pButton->addActionSignal(new CMenuHandler_StringCommand(cCommand));
				// Create an input signal that'll popup the current menu
				pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
			}

			// Get the next token
			if ( bGetExtraToken )
			{
				pfile = gEngfuncs.COM_ParseFile(pfile, token);
			}
		}

		// Move back up a menu
		m_pCurrentCommandMenu = m_pCurrentCommandMenu->GetParentMenu();

		pfile = gEngfuncs.COM_ParseFile(pfile, token);
	}
#ifdef _WIN32
}
catch( CException *e )
{
	e;
	//e->Delete();
	e = NULL;
	m_iInitialized = false;
	return;
}
#endif

	SetCurrentMenu( NULL );
	SetCurrentCommandMenu( NULL );
	gEngfuncs.COM_FreeFile( pfile );

	m_iInitialized = true;
}

//-----------------------------------------------------------------------------
// Purpose: Creates all the class choices under a spy's disguise menus, and
//			maps a command to them
// Output : CCommandMenu
//-----------------------------------------------------------------------------
CCommandMenu *TeamFortressViewport::CreateDisguiseSubmenu( CommandButton *pButton, CCommandMenu *pParentMenu, const char *commandText )
{
	// create the submenu, under which the class choices will be listed
	CCommandMenu *pMenu = CreateSubMenu( pButton, pParentMenu );
	m_pCommandMenus[m_iNumMenus] = pMenu;
	m_iNumMenus++;

	// create the class choice buttons
	for ( int i = PC_SCOUT; i <= PC_ENGINEER; i++ )
	{
		CommandButton *pDisguiseButton = new CommandButton( CHudTextMessage::BufferedLocaliseTextString( sLocalisedClasses[i] ), 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y );
		
		char sz[256]; 
		sprintf(sz, "%s %d", commandText, i );
		pDisguiseButton->addActionSignal(new CMenuHandler_StringCommand(sz));
		
		pMenu->AddButton( pDisguiseButton );
	}
	
	return pMenu;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pButtonText - 
//			*pButtonName - 
// Output : CommandButton
//-----------------------------------------------------------------------------
CommandButton *TeamFortressViewport::CreateCustomButton( char *pButtonText, char *pButtonName )
{
	CommandButton *pButton = NULL;
	CCommandMenu  *pMenu = NULL;

	// ChangeTeam
	if ( !strcmp( pButtonName, "!CHANGETEAM" ) )
	{
		// ChangeTeam Submenu
		pButton = new CommandButton(pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);

		// Create the submenu
		pMenu = CreateSubMenu(pButton, m_pCurrentCommandMenu);
		m_pCommandMenus[m_iNumMenus] = pMenu;
		m_iNumMenus++;

		// ChangeTeam buttons
		for (int i = 0; i < 4; i++)
		{
			char sz[256]; 
			sprintf(sz, "jointeam %d", i+1);
			m_pTeamButtons[i] = new TeamButton(i+1, "teamname", 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
			m_pTeamButtons[i]->addActionSignal(new CMenuHandler_StringCommandWatch( sz ));
			pMenu->AddButton( m_pTeamButtons[i] ); 
		}

		// Auto Assign button
		m_pTeamButtons[4] = new TeamButton(5, gHUD.m_TextMessage.BufferedLocaliseTextString( "#Team_AutoAssign" ), 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		m_pTeamButtons[4]->addActionSignal(new CMenuHandler_StringCommand( "jointeam 5" ));
		pMenu->AddButton( m_pTeamButtons[4] ); 

		// Spectate button
		m_pTeamButtons[5] = new SpectateButton( CHudTextMessage::BufferedLocaliseTextString( "#Menu_Spectate" ), 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y, false);
		m_pTeamButtons[5]->addActionSignal(new CMenuHandler_StringCommand( "spectate" ));
		pMenu->AddButton( m_pTeamButtons[5] ); 
	}
	// ChangeClass
	else if ( !strcmp( pButtonName, "!CHANGECLASS" ) )
	{
		// Create the Change class menu
		pButton = new ClassButton(-1, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y, false);

		// ChangeClass Submenu
		pMenu = CreateSubMenu(pButton, m_pCurrentCommandMenu);
		m_pCommandMenus[m_iNumMenus] = pMenu;
		m_iNumMenus++;

		for (int i = PC_SCOUT; i <= PC_RANDOM; i++ )
		{
			char sz[256]; 

			// ChangeClass buttons
			CHudTextMessage::LocaliseTextString( sLocalisedClasses[i], sz, 256 );
			ClassButton *pClassButton = new ClassButton( i, sz, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y, false);

			sprintf(sz, "%s", sTFClassSelection[i]);
			pClassButton->addActionSignal(new CMenuHandler_StringCommandClassSelect(sz));
			pMenu->AddButton( pClassButton );
		}
	}
	// Map Briefing
	else if ( !strcmp( pButtonName, "!MAPBRIEFING" ) )
	{
		pButton = new CommandButton(pButtonText, 0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_TextWindow(MENU_MAPBRIEFING));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Class Descriptions
	else if ( !strcmp( pButtonName, "!CLASSDESC" ) )
	{
		pButton = new ClassButton(0, pButtonText, 0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y, false);
		pButton->addActionSignal(new CMenuHandler_TextWindow(MENU_CLASSHELP));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!SERVERINFO" ) )
	{
		pButton = new ClassButton(0, pButtonText, 0, BUTTON_SIZE_Y * m_pCurrentCommandMenu->GetNumButtons(), CMENU_SIZE_X, BUTTON_SIZE_Y, false);
		pButton->addActionSignal(new CMenuHandler_TextWindow(MENU_INTRO));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Spy abilities
	else if ( !strcmp( pButtonName, "!SPY" ) )
	{
		pButton = new DisguiseButton( 0, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y );
	}
	// Feign
	else if ( !strcmp( pButtonName, "!FEIGN" ) )
	{
		pButton = new FeignButton(FALSE, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand( "feign" ));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Feign Silently
	else if ( !strcmp( pButtonName, "!FEIGNSILENT" ) )
	{
		pButton = new FeignButton(FALSE, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand( "sfeign" ));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Stop Feigning
	else if ( !strcmp( pButtonName, "!FEIGNSTOP" ) )
	{
		pButton = new FeignButton(TRUE, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand( "feign" ));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Disguise
	else if ( !strcmp( pButtonName, "!DISGUISEENEMY" ) )
	{
		// Create the disguise enemy button, which active only if there are 2 teams
		pButton = new DisguiseButton(DISGUISE_TEAM2, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		CreateDisguiseSubmenu( pButton, m_pCurrentCommandMenu, "disguise_enemy" );
	}
	else if ( !strcmp( pButtonName, "!DISGUISEFRIENDLY" ) )
	{
		// Create the disguise friendly button, which active only if there are 1 or 2 teams
		pButton = new DisguiseButton(DISGUISE_TEAM1 | DISGUISE_TEAM2, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		CreateDisguiseSubmenu( pButton, m_pCurrentCommandMenu, "disguise_friendly" );
	}
	else if ( !strcmp( pButtonName, "!DISGUISE" ) )
	{
		// Create the Disguise button
		pButton = new DisguiseButton( DISGUISE_TEAM3 | DISGUISE_TEAM4, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		CCommandMenu *pDisguiseMenu = CreateSubMenu( pButton, m_pCurrentCommandMenu );
		m_pCommandMenus[m_iNumMenus] = pDisguiseMenu;
		m_iNumMenus++;

		// Disguise Enemy submenu buttons
		for ( int i = 1; i <= 4; i++ )
		{
			// only show the 4th disguise button if we have 4 teams
			m_pDisguiseButtons[i] = new DisguiseButton( ((i < 4) ? DISGUISE_TEAM3 : 0) | DISGUISE_TEAM4, "Disguise", 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);

			pDisguiseMenu->AddButton( m_pDisguiseButtons[i] );

			char sz[256]; 
			sprintf( sz, "disguise %d", i );
			CreateDisguiseSubmenu( m_pDisguiseButtons[i], pDisguiseMenu, sz );
		}
	}
	// Start setting a Detpack
	else if ( !strcmp( pButtonName, "!DETPACKSTART" ) )
	{
		// Detpack Submenu
		pButton = new DetpackButton(2, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);

		// Create the submenu
		pMenu = CreateSubMenu(pButton, m_pCurrentCommandMenu);
		m_pCommandMenus[m_iNumMenus] = pMenu;
		m_iNumMenus++;

		// Set detpack buttons
		CommandButton *pDetButton;
		pDetButton = new CommandButton(m_sDetpackStrings[0], 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pDetButton->addActionSignal(new CMenuHandler_StringCommand("detstart 5"));
		pMenu->AddButton( pDetButton );
		pDetButton = new CommandButton(m_sDetpackStrings[1], 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pDetButton->addActionSignal(new CMenuHandler_StringCommand("detstart 20"));
		pMenu->AddButton( pDetButton );
		pDetButton = new CommandButton(m_sDetpackStrings[2], 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pDetButton->addActionSignal(new CMenuHandler_StringCommand("detstart 50"));
		pMenu->AddButton( pDetButton );
	}
	// Stop setting a Detpack
	else if ( !strcmp( pButtonName, "!DETPACKSTOP" ) )
	{
		pButton = new DetpackButton(1, pButtonText, 0, BUTTON_SIZE_Y, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand( "detstop" ));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Engineer building
	else if ( !strcmp( pButtonName, "!BUILD" ) )
	{
		// only appears if the player is an engineer, and either they have built something or have enough metal to build
		pButton = new BuildButton( BUILDSTATE_BASE, 0, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
	}
	else if ( !strcmp( pButtonName, "!BUILDSENTRY" ) )
	{
		pButton = new BuildButton( BUILDSTATE_CANBUILD, BuildButton::SENTRYGUN, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("build 2"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!BUILDDISPENSER" ) )
	{
		pButton = new BuildButton( BUILDSTATE_CANBUILD, BuildButton::DISPENSER, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("build 1"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!ROTATESENTRY180" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::SENTRYGUN, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("rotatesentry180"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!ROTATESENTRY" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::SENTRYGUN, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("rotatesentry"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!DISMANTLEDISPENSER" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::DISPENSER, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("dismantle 1"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!DISMANTLESENTRY" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::SENTRYGUN, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("dismantle 2"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!DETONATEDISPENSER" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::DISPENSER, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("detdispenser"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	else if ( !strcmp( pButtonName, "!DETONATESENTRY" ) )
	{
		pButton = new BuildButton( BUILDSTATE_HASBUILDING, BuildButton::SENTRYGUN, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("detsentry"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}
	// Stop building
	else if ( !strcmp( pButtonName, "!BUILDSTOP" ) )
	{
		pButton = new BuildButton( BUILDSTATE_BUILDING, 0, pButtonText, 0, BUTTON_SIZE_Y * 2, CMENU_SIZE_X, BUTTON_SIZE_Y);
		pButton->addActionSignal(new CMenuHandler_StringCommand("build"));
		// Create an input signal that'll popup the current menu
		pButton->addInputSignal( new CMenuHandler_PopupSubMenuInput(pButton, m_pCurrentCommandMenu) );
	}

	return pButton;
}

void TeamFortressViewport::ToggleServerBrowser()
{
	if (!m_iInitialized)
		return;

	if ( !m_pServerBrowser )
		return;

	if ( m_pServerBrowser->isVisible() )
	{
		m_pServerBrowser->setVisible( false );
	}
	else
	{
		m_pServerBrowser->setVisible( true );
	}

	UpdateCursorState();
}

//=======================================================================
void TeamFortressViewport::ShowCommandMenu()
{
	if (!m_iInitialized)
		return;

	// Not visible while undefined
	if (g_iPlayerClass == 0)
		return;

	// is the command menu open?
	if ( m_pCurrentCommandMenu )
	{
		HideCommandMenu();
		return;
	}

	// Not visible while in intermission
	if ( gHUD.m_iIntermission )
		return;

	// Recalculate visible menus
	UpdateCommandMenu();
	HideVGUIMenu();

	SetCurrentCommandMenu( m_pCommandMenus[0] );
	m_flMenuOpenTime = gHUD.m_flTime;
	UpdateCursorState();

	// get command menu parameters
	for ( int i = 2; i < gEngfuncs.Cmd_Argc(); i++ )
	{
		const char *param = gEngfuncs.Cmd_Argv( i - 1 );
		if ( param )
		{
			if ( m_pCurrentCommandMenu->KeyInput(param[0]) )
			{
				// kill the menu open time, since the key input is final
				HideCommandMenu();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles the key input of "-commandmenu"
// Input  : 
//-----------------------------------------------------------------------------
void TeamFortressViewport::InputSignalHideCommandMenu()
{
	if (!m_iInitialized)
		return;

	// if they've just tapped the command menu key, leave it open
	if ( (m_flMenuOpenTime + 0.3) > gHUD.m_flTime )
		return;

	HideCommandMenu();
}

//-----------------------------------------------------------------------------
// Purpose: Hides the command menu
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideCommandMenu( void )
{
	if (!m_iInitialized)
		return;

	if ( m_pCommandMenus[0] )
	{
		m_pCommandMenus[0]->ClearButtonsOfArmedState();
	}

	m_flMenuOpenTime = 0.0f;
	SetCurrentCommandMenu( NULL );
	UpdateCursorState();
}

//-----------------------------------------------------------------------------
// Purpose: Bring up the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::ShowScoreBoard( void )
{
	if (m_pScoreBoard)
	{
		// No Scoreboard in single-player
		if ( gEngfuncs.GetMaxClients() > 1 )
		{
			m_pScoreBoard->Open();
			UpdateCursorState();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if the scoreboard is up
//-----------------------------------------------------------------------------
bool TeamFortressViewport::IsScoreBoardVisible( void )
{
	if (m_pScoreBoard)
		return m_pScoreBoard->isVisible();

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Hide the scoreboard
//-----------------------------------------------------------------------------
void TeamFortressViewport::HideScoreBoard( void )
{
	// Prevent removal of scoreboard during intermission
	if ( gHUD.m_iIntermission )
		return;

	if (m_pScoreBoard)
	{
		m_pScoreBoard->setVisible(false);

		GetClientVoiceMgr()->StopSquelchMode();
		
		UpdateCursorState();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activate's the player special ability
//			called when the player hits their "special" key
//-----------------------------------------------------------------------------
void TeamFortressViewport::InputPlayerSpecial( void )
{
	if (!m_iInitialized)
		return;

	if ( g_iPlayerClass == PC_ENGINEER || g_iPlayerClass == PC_SPY )
	{
		ShowCommandMenu();

		if ( m_pCurrentCommandMenu )
		{
			m_pCurrentCommandMenu->KeyInput( '7' );
		}
	}
	else
	{
		// if it's any other class, just send the command down to the server
		ClientCmd( "_special" );
	}
}

// Set the submenu of the Command Menu
void TeamFortressViewport::SetCurrentCommandMenu( CCommandMenu *pNewMenu )
{
	for (int i = 0; i < m_iNumMenus; i++)
		m_pCommandMenus[i]->setVisible(false);

	m_pCurrentCommandMenu = pNewMenu;

	if (m_pCurrentCommandMenu)
		m_pCurrentCommandMenu->MakeVisible( NULL );
}

void TeamFortressViewport::UpdateCommandMenu()
{
	m_pCommandMenus[0]->RecalculateVisibles( 0, false );
	m_pCommandMenus[0]->RecalculatePositions( 0 );
}

void TeamFortressViewport::UpdateSpectatorMenu()
{
	char sz[64];

	if (!m_pSpectatorMenu)
		return;

	// Don't pop up if the round windows are up
	if ( m_pDiscStartRound && m_pDiscStartRound->isVisible() )
		return;
	if ( m_pDiscEndRound && m_pDiscEndRound->isVisible() )
		return;

	if (m_iUser1)
	{
		m_pSpectatorMenu->setVisible( true );

		if (m_iUser2 > 0 && m_iUser1 != 4 )
		{
			// Locked onto a target, show the player's name
			sprintf(sz, "#Spec_Mode%d : %s", m_iUser1, g_PlayerInfoList[ m_iUser2 ].name);
			m_pSpectatorLabel->setText( CHudTextMessage::BufferedLocaliseTextString( sz ) );
		}
		else
		{
			sprintf(sz, "#Spec_Mode%d", m_iUser1);
			m_pSpectatorLabel->setText( CHudTextMessage::BufferedLocaliseTextString( sz ) );
		}
	}
	else
	{
		m_pSpectatorMenu->setVisible( false );
	}
}

//======================================================================
void TeamFortressViewport::CreateScoreBoard( void )
{
	m_pScoreBoard = new ScorePanel(SBOARD_INDENT_X,SBOARD_INDENT_Y, ScreenWidth - (SBOARD_INDENT_X * 2), ScreenHeight - (SBOARD_INDENT_Y * 2));
	m_pScoreBoard->setParent(this);
	m_pScoreBoard->setVisible(false);
}

void TeamFortressViewport::CreateServerBrowser( void )
{
	m_pServerBrowser = new ServerBrowser( 0, 0, ScreenWidth, ScreenHeight );
	m_pServerBrowser->setParent(this);
	m_pServerBrowser->setVisible(false);
}

//======================================================================
// Disc vgui objects
void TeamFortressViewport::CreateDiscIcons( void )
{
	m_iDiscPowerup = 0;

	// Create the disc ammo icons
	int iX = (ScreenWidth - DISC_ICON_WIDTH) / 2;
	int iXPos = iX - DISC_ICON_SPACER; 
	for (int i = 0; i < MAX_DISCS; i++)
	{
		m_pDiscIcons[i] = new CDiscPanel( iXPos + (i * DISC_ICON_SPACER), ScreenHeight - YRES(48), DISC_ICON_WIDTH, YRES(32) );
		m_pDiscIcons[i]->setParent(this);
	}

	// Create the Arena Windows
	m_pDiscStartRound = new CDiscArena_RoundStart();
	m_pDiscStartRound->setParent(this);
	m_pDiscEndRound = new CDiscArena_RoundEnd();
	m_pDiscEndRound->setParent(this);

	// Create the Powerup window
	m_pDiscPowerupWindow = new CDiscPowerups();
	m_pDiscPowerupWindow->setParent(this);

	// Create the Reward window
	m_pDiscRewardWindow = new CDiscRewards();
	m_pDiscRewardWindow->setParent(this);
}

//======================================================================
// Set the VGUI Menu
void TeamFortressViewport::SetCurrentMenu( CMenuPanel *pMenu )
{
	m_pCurrentMenu = pMenu;
	if ( m_pCurrentMenu )
	{
		// Don't open menus in demo playback
		if ( gEngfuncs.pDemoAPI->IsPlayingback() )
			return;

		m_pCurrentMenu->Open();
	}
}

//================================================================
// Text Window
CMenuPanel* TeamFortressViewport::CreateTextWindow( int iTextToShow )
{
	char sz[256];
	char *cText;
	char *pfile = NULL;
	static const int MAX_TITLE_LENGTH = 32;
	char cTitle[MAX_TITLE_LENGTH];

	if ( iTextToShow == SHOW_MOTD )
	{
		if (!m_szServerName || !m_szServerName[0])
			strcpy( cTitle, "Half-Life" );
		else
			strncpy( cTitle, m_szServerName, MAX_TITLE_LENGTH );
		cTitle[MAX_TITLE_LENGTH-1] = 0;
		cText = m_szMOTD;
	}
	else if ( iTextToShow == SHOW_MAPBRIEFING )
	{
		// Get the current mapname, and open it's map briefing text
		if (m_sMapName && m_sMapName[0])
		{
			strcpy( sz, "maps/");
			strcat( sz, m_sMapName );
			strcat( sz, ".txt" );
		}
		else
		{
			const char *level = gEngfuncs.pfnGetLevelName();
			if (!level)
				return NULL;

			strcpy( sz, level );
			char *ch = strchr( sz, '.' );
			*ch = '\0';
			strcat( sz, ".txt" );

			// pull out the map name
			strcpy( m_sMapName, level );
			ch = strchr( m_sMapName, '.' );
			if ( ch )
			{
				*ch = 0;
			}

			ch = strchr( m_sMapName, '/' );
			if ( ch )
			{
				// move the string back over the '/'
				memmove( m_sMapName, ch+1, strlen(ch)+1 );
			}
		}

		pfile = (char*)gEngfuncs.COM_LoadFile( sz, 5, NULL );

		if (!pfile)
			return NULL;

		cText = pfile;

		strncpy( cTitle, m_sMapName, MAX_TITLE_LENGTH );
		cTitle[MAX_TITLE_LENGTH-1] = 0;
	}
	else if ( iTextToShow == SHOW_CLASSDESC )
	{
		switch ( g_iPlayerClass )
		{
		case PC_SCOUT:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_scout" ); 
							CHudTextMessage::LocaliseTextString( "#Title_scout", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SNIPER:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_sniper" );
							CHudTextMessage::LocaliseTextString( "#Title_sniper", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SOLDIER:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_soldier" );
							CHudTextMessage::LocaliseTextString( "#Title_soldier", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_DEMOMAN:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_demoman" );
							CHudTextMessage::LocaliseTextString( "#Title_demoman", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_MEDIC:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_medic" );
							CHudTextMessage::LocaliseTextString( "#Title_medic", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_HVYWEAP:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_hwguy" );
							CHudTextMessage::LocaliseTextString( "#Title_hwguy", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_PYRO:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_pyro" );
							CHudTextMessage::LocaliseTextString( "#Title_pyro", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_SPY:		cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_spy" );
							CHudTextMessage::LocaliseTextString( "#Title_spy", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_ENGINEER:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_engineer" );
							CHudTextMessage::LocaliseTextString( "#Title_engineer", cTitle, MAX_TITLE_LENGTH ); break;
		case PC_CIVILIAN:	cText = CHudTextMessage::BufferedLocaliseTextString( "#Help_civilian" );
							CHudTextMessage::LocaliseTextString( "#Title_civilian", cTitle, MAX_TITLE_LENGTH ); break;
		default:
			return NULL;
		}

		if ( g_iPlayerClass == PC_CIVILIAN )
		{
			sprintf(sz, "classes/long_civilian.txt");
		}
		else
		{
			sprintf(sz, "classes/long_%s.txt", sTFClassSelection[ g_iPlayerClass ]);
		}
		char *pfile = (char*)gEngfuncs.COM_LoadFile( sz, 5, NULL );
		if (pfile)
		{
			cText = pfile;
		}
	}

	// if we're in the game (ie. have selected a class), flag the menu to be only grayed in the dialog box, instead of full screen
	CMenuPanel *pMOTDPanel = CMessageWindowPanel_Create( cText, cTitle, g_iPlayerClass == PC_UNDEFINED, false, 0, 0, ScreenWidth, ScreenHeight );
	pMOTDPanel->setParent( this );

	if ( pfile )
		gEngfuncs.COM_FreeFile( pfile );

	return pMOTDPanel;
}

//================================================================
// VGUI Menus
void TeamFortressViewport::ShowVGUIMenu( int iMenu )
{
	CMenuPanel *pNewMenu = NULL;

	// Don't open menus in demo playback
	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
		return;

	// Don't create one if it's already in the list
	if (m_pCurrentMenu)
	{
		CMenuPanel *pMenu = m_pCurrentMenu;
		while (pMenu != NULL)
		{
			if (pMenu->GetMenuID() == iMenu)
				return;
			pMenu = pMenu->GetNextMenu();
		}
	}

	switch ( iMenu )
	{
	// Map Briefing removed now that it appears in the team menu
	case MENU_MAPBRIEFING:
		pNewMenu = CreateTextWindow( SHOW_MAPBRIEFING );
		break;

	case MENU_INTRO:
		pNewMenu = CreateTextWindow( SHOW_MOTD );
		break;

	case MENU_CLASSHELP:
		pNewMenu = CreateTextWindow( SHOW_CLASSDESC );
		break;

	default:
		break;
	}

	if (!pNewMenu)
		return;

	// Close the Command Menu if it's open
	HideCommandMenu();

	pNewMenu->SetMenuID( iMenu );
	pNewMenu->SetActive( true );

	// See if another menu is visible, and if so, cache this one for display once the other one's finished
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->SetNextMenu( pNewMenu );
	}
	else
	{
		m_pCurrentMenu = pNewMenu;
		m_pCurrentMenu->Open();
		UpdateCursorState();
	}
}

// Removes all VGUI Menu's onscreen
void TeamFortressViewport::HideVGUIMenu()
{
	while (m_pCurrentMenu)
	{
		HideTopMenu();
	}
}

// Remove the top VGUI menu, and bring up the next one
void TeamFortressViewport::HideTopMenu()
{
	if (m_pCurrentMenu)
	{
		// Close the top one
		m_pCurrentMenu->Close();

		// Bring up the next one
		gViewPort->SetCurrentMenu( m_pCurrentMenu->GetNextMenu() );
	}

	UpdateCursorState();
}

// Return TRUE if the HUD's allowed to print text messages
bool TeamFortressViewport::AllowedToPrintText( void )
{
	// Prevent text messages when fullscreen menus are up
	if ( m_pCurrentMenu && g_iPlayerClass == 0 )
	{
		int iId = m_pCurrentMenu->GetMenuID();
		if ( iId == MENU_TEAM || iId == MENU_CLASS || iId == MENU_INTRO || iId == MENU_CLASSHELP )
			return FALSE;
	}

	return TRUE;
}

//======================================================================================
// SPECTATOR MENU
//======================================================================================
// Spectator "Menu" explaining the Spectator buttons
void TeamFortressViewport::CreateSpectatorMenu()
{
	// Create the Panel
	m_pSpectatorMenu = new CTransparentPanel(100, 0, ScreenHeight - YRES(60), ScreenWidth, YRES(60));
	m_pSpectatorMenu->setParent(this);
	m_pSpectatorMenu->setVisible(false);

	// Get the scheme used for the Titles
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	// schemes
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );
	SchemeHandle_t hHelpText = pSchemes->getSchemeHandle( "Primary Button Text" );

	// color schemes
	int r, g, b, a;

	// Create the title
	m_pSpectatorLabel = new Label( "Spectator", 0, 0, ScreenWidth, YRES(25) );
	m_pSpectatorLabel->setParent( m_pSpectatorMenu );
	m_pSpectatorLabel->setFont( pSchemes->getFont(hTitleScheme) );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	m_pSpectatorLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	m_pSpectatorLabel->setBgColor( r, g, b, 255 );
	m_pSpectatorLabel->setContentAlignment( vgui::Label::a_north );

	// Create the Help
	Label *pLabel = new Label( CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help" ), 0, YRES(25), ScreenWidth, YRES(15) );
	pLabel->setParent( m_pSpectatorMenu );
	pLabel->setFont( pSchemes->getFont(hHelpText) );
	pSchemes->getFgColor( hHelpText, r, g, b, a );
	pLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hHelpText, r, g, b, a );
	pLabel->setBgColor( r, g, b, 255 );
	pLabel->setContentAlignment( vgui::Label::a_north );

	pLabel = new Label( CHudTextMessage::BufferedLocaliseTextString( "#Spec_Help2" ), 0, YRES(40), ScreenWidth, YRES(20) );
	pLabel->setParent( m_pSpectatorMenu );
	pLabel->setFont( pSchemes->getFont(hHelpText) );
	pSchemes->getFgColor( hHelpText, r, g, b, a );
	pLabel->setFgColor( r, g, b, a );
	pSchemes->getBgColor( hHelpText, r, g, b, a );
	pLabel->setBgColor( r, g, b, 255 );
	pLabel->setContentAlignment( vgui::Label::a_center );
}

//======================================================================================
// UPDATE HUD SECTIONS
//======================================================================================
// We've got an update on player info
// Recalculate any menus that use it.
void TeamFortressViewport::UpdateOnPlayerInfo()
{
	if (m_pScoreBoard)
		m_pScoreBoard->Update();
}

void TeamFortressViewport::UpdateCursorState()
{
	// Need cursor if any VGUI window is up
	if ( m_pCurrentMenu || m_pServerBrowser->isVisible() || GetClientVoiceMgr()->IsInSquelchMode() )
	{
		g_iVisibleMouse = true;
		App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow) );
		return;
	}
	else if ( m_pCurrentCommandMenu )
	{
		// commandmenu doesn't have cursor if hud_capturemouse is turned off
		if ( gHUD.m_pCvarStealMouse->value != 0.0f )
		{
			g_iVisibleMouse = true;
			App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_arrow) );
			return;
		}
	}

	IN_ResetMouse();

    if (g_iVisibleMouse)
    {
        //Clear any residual input so our camera doesn't jerk when dismissing the UI
        IN_ResetRelativeMouseState();
    }

	g_iVisibleMouse = false;
	App::getInstance()->setCursorOveride( App::getInstance()->getScheme()->getCursor(Scheme::scu_none) );
}

void TeamFortressViewport::UpdateHighlights()
{
	if (m_pCurrentCommandMenu)
		m_pCurrentCommandMenu->MakeVisible( NULL );
}

void TeamFortressViewport::GetAllPlayersInfo( void )
{
	for ( int i = 1; i < MAX_PLAYERS; i++ )
	{
		GetPlayerInfo( i, &g_PlayerInfoList[i] );

		if ( g_PlayerInfoList[i].thisplayer )
			m_pScoreBoard->m_iPlayerNum = i;  // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
	}
}

void TeamFortressViewport::paintBackground()
{
	int wide, tall;
	getParent()->getSize( wide, tall );
	setSize( wide, tall );

	if (m_pScoreBoard)
	{
		int x, y;
		getApp()->getCursorPos(x, y);
		m_pScoreBoard->cursorMoved(x, y, m_pScoreBoard);
	}

	// See if the command menu is visible and needs recalculating due to some external change
	if ( g_iTeamNumber != m_iCurrentTeamNumber )
	{
		UpdateCommandMenu();
		for (int i = 0; i < MAX_DISCS; i++)
		{
			if ( m_pDiscIcons[i] )
				m_pDiscIcons[i]->Update( i, false, m_iDiscPowerup );
		}

		m_iCurrentTeamNumber = g_iTeamNumber;
	}

	if ( g_iPlayerClass != m_iCurrentPlayerClass )
	{
		UpdateCommandMenu();

		m_iCurrentPlayerClass = g_iPlayerClass;
	}

	// Update the Reward window
	if ( m_flRewardOpenTime && ( m_flRewardOpenTime < gHUD.m_flTime ) )
	{
		m_pDiscRewardWindow->setVisible(false);
		m_flRewardOpenTime = 0;
	}

	// Update the disc icons the player has
	bool bVisible = true;
	if ( m_pDiscStartRound && m_pDiscStartRound->isVisible() )
		bVisible = false;
	if ( m_pDiscEndRound && m_pDiscEndRound->isVisible() )
		bVisible = false;
	if ( m_pSpectatorMenu && m_pSpectatorMenu->isVisible() )
		bVisible = false;
	for (int i = 0; i < MAX_DISCS; i++)
	{
		if ( bVisible == false )
		{
			m_pDiscIcons[i]->setVisible(false);
		}
		else
		{
			m_pDiscIcons[i]->Update(i, true, m_iDiscPowerup);
			m_pDiscIcons[i]->setVisible(true);
		}
	}

	// See if the Spectator Menu needs to be update
	if ( g_iUser1 != m_iUser1 || g_iUser2 != m_iUser2 )
	{
		m_iUser1 = g_iUser1;
		m_iUser2 = g_iUser2;
		UpdateSpectatorMenu();
	}

	// Update the Scoreboard, if it's visible
	if ( m_pScoreBoard->isVisible() && (m_flScoreBoardLastUpdated < gHUD.m_flTime) )
	{
		m_pScoreBoard->Update();
		m_flScoreBoardLastUpdated = gHUD.m_flTime + 0.5;
	}

	int extents[4];
	getAbsExtents(extents[0],extents[1],extents[2],extents[3]);
	VGui_ViewportPaintBackground(extents);
}

//================================================================
// Input Handler for Drag N Drop panels
void CDragNDropHandler::cursorMoved(int x,int y,Panel* panel)
{
	if(m_bDragging)
	{
		App::getInstance()->getCursorPos(x,y);			
		m_pPanel->setPos(m_iaDragOrgPos[0]+(x-m_iaDragStart[0]),m_iaDragOrgPos[1]+(y-m_iaDragStart[1]));
		
		if(m_pPanel->getParent()!=null)
		{			
			m_pPanel->getParent()->repaint();
		}
	}
}

void CDragNDropHandler::mousePressed(MouseCode code,Panel* panel)
{
	int x,y;
	App::getInstance()->getCursorPos(x,y);
	m_bDragging=true;
	m_iaDragStart[0]=x;
	m_iaDragStart[1]=y;
	m_pPanel->getPos(m_iaDragOrgPos[0],m_iaDragOrgPos[1]);
	App::getInstance()->setMouseCapture(panel);

	m_pPanel->setDragged(m_bDragging);
	m_pPanel->requestFocus();
} 

void CDragNDropHandler::mouseReleased(MouseCode code,Panel* panel)
{
	m_bDragging=false;
	m_pPanel->setDragged(m_bDragging);
	App::getInstance()->setMouseCapture(null);
}

//================================================================
// Number Key Input
bool TeamFortressViewport::SlotInput( int iSlot )
{
	// If there's a menu up, give it the input
	if ( m_pCurrentMenu )
		return m_pCurrentMenu->SlotInput( iSlot );

	return FALSE;
}

// Direct Key Input
int	TeamFortressViewport::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	// Enter gets out of Spectator Mode by bringing up the Team Menu
	if (m_iUser1 && gEngfuncs.Con_IsVisible() == false )
	{
		if ( down && keynum == K_SPACE )
			gEngfuncs.pfnClientCmd("ob_next");
		if ( down && keynum == K_ENTER )
			gEngfuncs.pfnClientCmd("ob_mode");
		if ( down && keynum == K_CTRL )
			gEngfuncs.pfnClientCmd("ob_prev");
	}

	// Open Text Window?
	if (m_pCurrentMenu && gEngfuncs.Con_IsVisible() == false)
	{
		int iMenuID = m_pCurrentMenu->GetMenuID();

		// Get number keys as Input for Team/Class menus
		if (iMenuID == MENU_TEAM || iMenuID == MENU_CLASS)
		{
			// Escape gets you out of Team/Class menus if the Cancel button is visible
			if ( keynum == K_ESCAPE )
			{
				if ( (iMenuID == MENU_TEAM && g_iTeamNumber) || (iMenuID == MENU_CLASS && g_iPlayerClass) )
				{
					HideTopMenu();
					return 0;
				}
			}

			for (int i = '0'; i <= '9'; i++)
			{
				if ( down && (keynum == i) )
				{
					SlotInput( i - '0' );
					return 0;
				}
			}
		}

		// Grab enter keys to close TextWindows
		if ( down && (keynum == K_ENTER || keynum == K_KP_ENTER || keynum == K_SPACE || keynum == K_ESCAPE) )
		{
			if ( iMenuID == MENU_MAPBRIEFING || iMenuID == MENU_INTRO || iMenuID == MENU_CLASSHELP )
			{
				HideTopMenu();
				return 0;
			}
		}

	}

	// if we're in a command menu, try hit one of it's buttons
	if ( down && m_pCurrentCommandMenu )
	{
		// Escape hides the command menu
		if ( keynum == K_ESCAPE )
		{
			HideCommandMenu();
			return 0;
		}

		// only trap the number keys
		if ( keynum >= '0' && keynum <= '9' )
		{
			if ( m_pCurrentCommandMenu->KeyInput(keynum) )
			{
				// a final command has been issued, so close the command menu
				HideCommandMenu();
			}

			return 0;
		}
	}

	return 1;
}

//================================================================
// Message Handlers
int TeamFortressViewport::MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	for (int i = 0; i < 5; i++)
		m_iValidClasses[i] = READ_SHORT();

	// Force the menu to update
	UpdateCommandMenu();

	return 1;
}

int TeamFortressViewport::MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	m_iNumberOfTeams = READ_BYTE();

	for (int i = 0; i < m_iNumberOfTeams; i++)
	{
		int teamNum = i + 1;

		gHUD.m_TextMessage.LocaliseTextString( READ_STRING(), m_sTeamNames[teamNum], MAX_TEAMNAME_SIZE );

		// Set the team name buttons
		if (m_pTeamButtons[i])
			m_pTeamButtons[i]->setText( m_sTeamNames[teamNum] );

		// Set the disguise buttons
		if (m_pDisguiseButtons[i])
			m_pDisguiseButtons[i]->setText( m_sTeamNames[teamNum] );
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_Feign(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	m_iIsFeigning = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu();

	return 1;
}

int TeamFortressViewport::MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iIsSettingDetpack = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu();

	return 1;
}

int TeamFortressViewport::MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iMenu = READ_BYTE();

	// Map briefing includes the name of the map (because it's sent down before the client knows what map it is)
	if (iMenu == MENU_MAPBRIEFING)
	{
		strncpy( m_sMapName, READ_STRING(), sizeof(m_sMapName) );
		m_sMapName[ sizeof(m_sMapName) - 1 ] = '\0';
	}

	// Bring up the menu6
	ShowVGUIMenu( iMenu );

	return 1;
}

int TeamFortressViewport::MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf )
{
	if (m_iGotAllMOTD)
		m_szMOTD[0] = 0;

	BEGIN_READ( pbuf, iSize );

	m_iGotAllMOTD = READ_BYTE();
	strncat( m_szMOTD, READ_STRING(), sizeof(m_szMOTD) - strlen(m_szMOTD) );
	m_szMOTD[ sizeof(m_szMOTD)-1 ] = '\0';

	if ( m_iGotAllMOTD )
	{
		ShowVGUIMenu( MENU_INTRO );
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_BuildSt( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iBuildState = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu();

	return 1;
}

int TeamFortressViewport::MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iRandomPC = READ_BYTE();

	return 1;
}

int TeamFortressViewport::MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	strncpy( m_szServerName, READ_STRING(), MAX_SERVERNAME_LENGTH );

	return 1;
}

int TeamFortressViewport::MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	short cl = READ_BYTE();
	short frags = READ_SHORT();
	short deaths = READ_SHORT();
	short playerclass = READ_SHORT();
	short teamnumber = READ_SHORT();

	if ( cl > 0 && cl <= MAX_PLAYERS )
	{
		g_PlayerExtraInfo[cl].frags = frags;
		g_PlayerExtraInfo[cl].deaths = deaths;
		g_PlayerExtraInfo[cl].playerclass = playerclass;
		g_PlayerExtraInfo[cl].teamnumber = teamnumber;

		UpdateOnPlayerInfo();
	}

	return 1;
}

// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths 
// if this message is never received, then scores will simply be the combined totals of the players.
int TeamFortressViewport::MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	char *TeamName = READ_STRING();

	// find the team matching the name
	int i;
	for ( i = 1; i <= m_pScoreBoard->m_iNumTeams; i++ )
	{
		if ( !stricmp( TeamName, g_TeamInfo[i].name ) )
			break;
	}

	if ( i > m_pScoreBoard->m_iNumTeams )
		return 1;

	// use this new score data instead of combined player scoresw
	g_TeamInfo[i].scores_overriden = TRUE;
	g_TeamInfo[i].frags = READ_SHORT();
	g_TeamInfo[i].deaths = READ_SHORT();

	return 1;
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
int TeamFortressViewport::MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf )
{
	if (!m_pScoreBoard)
		return 1;

	BEGIN_READ( pbuf, iSize );
	short cl = READ_BYTE();
	
	if ( cl > 0 && cl <= MAX_PLAYERS )
	{  
		// set the players team
		strncpy( g_PlayerExtraInfo[cl].teamname, READ_STRING(), MAX_TEAM_NAME );
	}

	// rebuild the list of teams
	m_pScoreBoard->RebuildTeams();

	return 1;
}

void TeamFortressViewport::DeathMsg( int killer, int victim )
{
	m_pScoreBoard->DeathMsg(killer,victim);
}

int TeamFortressViewport::MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	short cl = READ_BYTE();
	if ( cl > 0 && cl <= MAX_PLAYERS )
	{
		g_IsSpectator[cl] = READ_BYTE();
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iAllowSpectators = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu();

	return 1;
}

int TeamFortressViewport::MsgFunc_StartRnd(const char *pszName, int iSize, void *pbuf )
{
	if (m_pDiscStartRound)
		return m_pDiscStartRound->MsgFunc_GetPlayers( pszName, iSize, pbuf );

	return 1;
}

int TeamFortressViewport::MsgFunc_EndRnd(const char *pszName, int iSize, void *pbuf )
{
	if (m_pDiscEndRound)
		return m_pDiscEndRound->MsgFunc_GetPlayers( pszName, iSize, pbuf );

	return 1;
}

int TeamFortressViewport::MsgFunc_Frozen(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iFrozen = READ_BYTE();

	if ( iFrozen ) 
	{
		gHUD.m_Health.m_bitsDamage = DMG_FREEZE;
		gHUD.m_Health.m_iFlags |= HUD_ACTIVE;
	}
	else 
	{
		gHUD.m_Health.m_bitsDamage = 0;
		gHUD.m_Health.m_iFlags &= ~HUD_ACTIVE;
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_Powerup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_iDiscPowerup = READ_BYTE();

	// Force the disc icons to update
	for (int i = 0; i < MAX_DISCS; i++)
	{
		if ( m_pDiscIcons[i] )
			m_pDiscIcons[i]->Update( i, false, m_iDiscPowerup );
	}	

	// Update the powerup window
	m_pDiscPowerupWindow->RecalculateText( m_iDiscPowerup );

	return 1;
}

int TeamFortressViewport::MsgFunc_Reward( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iReward = READ_SHORT();

	// Update the reward window
	m_pDiscRewardWindow->RecalculateText( iReward );
	m_flRewardOpenTime = gHUD.m_flTime + 2.0;

	return 1;
}


