//=========== (C) Copyright 1996-2002 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Contains implementation of various VGUI-derived objects
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

#include "vgui_int.h"
#include "vgui_viewport.h"
#include "vgui_ServerBrowser.h"
#include "VGUI_BitmapTGA.h"


// Arrow filenames
char *sArrowFilenames[] =
{
	"arrowup",
	"arrowdn", 
	"arrowlt",
	"arrowrt", 
};


//-----------------------------------------------------------------------------
// Purpose: Loads a .tga file and returns a pointer to the VGUI tga object
//-----------------------------------------------------------------------------
BitmapTGA *LoadTGA( const char* pImageName )
{
	BitmapTGA	*pTGA;

	char sz[256];
	sprintf(sz, "%%d_%s", pImageName);

	// Load the Image
	FileInputStream* fis = new FileInputStream( GetVGUITGAName(sz), false );
	pTGA = new BitmapTGA(fis,true);
	fis->close();

	return pTGA;
}

//===========================================================
// All TFC Hud buttons are derived from this one.
CommandButton::CommandButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight) : Button("",x,y,wide,tall)
{
	m_iPlayerClass = 0;
	m_bNoHighlight = bNoHighlight;
	Init();
	setText( text );
}

CommandButton::CommandButton( int iPlayerClass, const char* text,int x,int y,int wide,int tall) : Button("",x,y,wide,tall)
{
	m_iPlayerClass = iPlayerClass;
	m_bNoHighlight = false;
	Init();
	setText( text );
}

void CommandButton::Init( void )
{
	m_pSubMenu = NULL;
	m_pSubLabel = NULL;
	m_pParentMenu = NULL;

	// Set text color to orange
	setFgColor(Scheme::sc_primary1);

	// left align
	setContentAlignment( vgui::Label::a_west );

	// Add the Highlight signal
	if (!m_bNoHighlight)
		addInputSignal( new CHandler_CommandButtonHighlight(this) );

	// not bound to any button yet
	m_cBoundKey = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Prepends the button text with the current bound key
//			if no bound key, then a clear space ' ' instead
//-----------------------------------------------------------------------------
void CommandButton::RecalculateText( void )
{
	char szBuf[128];

	if ( m_cBoundKey != 0 )
	{
		if ( m_cBoundKey == (char)255 )
		{
			strcpy( szBuf, m_sMainText );
		}
		else
		{
			sprintf( szBuf, "  %c  %s", m_cBoundKey, m_sMainText );
		}
		szBuf[MAX_BUTTON_SIZE-1] = 0;
	}
	else
	{
		// just draw a space if no key bound
		sprintf( szBuf, "     %s", m_sMainText );
		szBuf[MAX_BUTTON_SIZE-1] = 0;
	}

	Button::setText( szBuf );
}

void CommandButton::setText( const char *text )
{
	strncpy( m_sMainText, text, MAX_BUTTON_SIZE );
	m_sMainText[MAX_BUTTON_SIZE-1] = 0;

	RecalculateText();
}

void CommandButton::setBoundKey( char boundKey )
{
	m_cBoundKey = boundKey;
	RecalculateText();
}

char CommandButton::getBoundKey( void )
{
	return m_cBoundKey;
}

void CommandButton::AddSubMenu( CCommandMenu *pNewMenu )
{
	m_pSubMenu = pNewMenu;

	// Prevent this button from being pushed
	setMouseClickEnabled( MOUSE_LEFT, false );
}

void CommandButton::UpdateSubMenus( int iAdjustment )
{
	if ( m_pSubMenu )
		m_pSubMenu->RecalculatePositions( iAdjustment );
}

void CommandButton::paint()
{
	// Make the sub label paint the same as the button
	if ( m_pSubLabel )
	{
		if ( isSelected() )
			m_pSubLabel->PushDown();
		else
			m_pSubLabel->PushUp();
	}

	// draw armed button text in white
	if ( isArmed() )
	{
		setFgColor( Scheme::sc_secondary2 );
	}
	else
	{
		setFgColor( Scheme::sc_primary1 );
	}
	
	Button::paint();
}

void CommandButton::paintBackground()
{
	if ( isArmed() )
	{
		// Orange highlight background
		drawSetColor( Scheme::sc_primary2 );
		drawFilledRect(0,0,_size[0],_size[1]);
	}

	// Orange Border
	drawSetColor( Scheme::sc_secondary1 );
	drawOutlinedRect(0,0,_size[0],_size[1]);
}

//-----------------------------------------------------------------------------
// Purpose: Highlights the current button, and all it's parent menus
//-----------------------------------------------------------------------------
void CommandButton::cursorEntered( void )
{
	// unarm all the other buttons in this menu
	CCommandMenu *containingMenu = getParentMenu();
	if ( containingMenu )
	{
		containingMenu->ClearButtonsOfArmedState();

		// make all our higher buttons armed
		CCommandMenu *pCParent = containingMenu->GetParentMenu();
		if ( pCParent )
		{
			CommandButton *pParentButton = pCParent->FindButtonWithSubmenu( containingMenu );

			pParentButton->cursorEntered();
		}
	}

	// arm ourselves
	setArmed( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CommandButton::cursorExited( void )
{
	// only clear ourselves if we have do not have a containing menu
	// only stay armed if we have a sub menu
	// the buttons only unarm themselves when another button is armed instead
	if ( !getParentMenu() || !GetSubMenu() )
	{
		setArmed( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the command menu that the button is part of, if any
// Output : CCommandMenu *
//-----------------------------------------------------------------------------
CCommandMenu *CommandButton::getParentMenu( void )
{ 
	return m_pParentMenu; 
}

//-----------------------------------------------------------------------------
// Purpose: Sets the menu that contains this button
// Input  : *pParentMenu - 
//-----------------------------------------------------------------------------
void CommandButton::setParentMenu( CCommandMenu *pParentMenu )
{
	m_pParentMenu = pParentMenu;
}


//===========================================================
int ClassButton::IsNotValid()
{
	return false;
}

//===========================================================
// Button with Class image beneath it
CImageLabel::CImageLabel( const char* pImageName,int x,int y ) : Label( "", x,y )
{
	setContentFitted(true);
	m_pTGA = LoadTGA(pImageName);
	setImage( m_pTGA );
}

CImageLabel::CImageLabel( const char* pImageName,int x,int y,int wide,int tall ) : Label( "", x,y,wide,tall )
{
	setContentFitted(true);
	m_pTGA = LoadTGA(pImageName);
	setImage( m_pTGA );
}

//===========================================================
// Image size
int CImageLabel::getImageWide( void )
{
	int iXSize, iYSize;
	m_pTGA->getSize( iXSize, iYSize );
	return iXSize;
}

int CImageLabel::getImageTall( void )
{
	int iXSize, iYSize;
	m_pTGA->getSize( iXSize, iYSize );
	return iYSize;
}

//===========================================================
// Various overloaded paint functions for Custom VGUI objects
void CCommandMenu::paintBackground()
{
	// Transparent black background
	drawSetColor(Scheme::sc_primary3);
	drawFilledRect(0,0,_size[0],_size[1]);
}

//=================================================================================
// CUSTOM SCROLLPANEL
//=================================================================================
CTFScrollButton::CTFScrollButton(int iArrow, const char* text,int x,int y,int wide,int tall) : CommandButton(text,x,y,wide,tall)
{
	// Set text color to orange
	setFgColor(Scheme::sc_primary1);

	// Load in the arrow
	m_pTGA = LoadTGA( sArrowFilenames[iArrow] );
	setImage( m_pTGA );

	// Highlight signal
	InputSignal *pISignal = new CHandler_CommandButtonHighlight(this);
	addInputSignal(pISignal);
}

void CTFScrollButton::paint( void )
{
	// draw armed button text in white
	if ( isArmed() )
	{
		m_pTGA->setColor( Color(255,255,255, 0) );
	}
	else
	{
		m_pTGA->setColor( Color(255,255,255, 128) );
	}

	m_pTGA->doPaint(this);
}

void CTFScrollButton::paintBackground( void )
{
/*
	if ( isArmed() )
	{
		// Orange highlight background
		drawSetColor( Scheme::sc_primary2 );
		drawFilledRect(0,0,_size[0],_size[1]);
	}

	// Orange Border
	drawSetColor( Scheme::sc_secondary1 );
	drawOutlinedRect(0,0,_size[0]-1,_size[1]);
*/
}

void CTFSlider::paintBackground( void )
{
	int wide,tall,nobx,noby;
	getPaintSize(wide,tall);
	getNobPos(nobx,noby);

	// Border
	drawSetColor( Scheme::sc_secondary1 );
	drawOutlinedRect( 0,0,wide,tall );

	if( isVertical() )
	{
		// Nob Fill
		drawSetColor( Scheme::sc_primary2 );
		drawFilledRect( 0,nobx,wide,noby );

		// Nob Outline
		drawSetColor( Scheme::sc_primary1 );
		drawOutlinedRect( 0,nobx,wide,noby );
	}
	else
	{
		// Nob Fill
		drawSetColor( Scheme::sc_primary2 );
		drawFilledRect( nobx,0,noby,tall );

		// Nob Outline
		drawSetColor( Scheme::sc_primary1 );
		drawOutlinedRect( nobx,0,noby,tall );
	}
}

CTFScrollPanel::CTFScrollPanel(int x,int y,int wide,int tall) : ScrollPanel(x,y,wide,tall)
{
	ScrollBar *pScrollBar = getVerticalScrollBar();
	pScrollBar->setButton( new CTFScrollButton( ARROW_UP, "", 0,0,16,16 ), 0 );
	pScrollBar->setButton( new CTFScrollButton( ARROW_DOWN, "", 0,0,16,16 ), 1 );
	pScrollBar->setSlider( new CTFSlider(0,wide-1,wide,(tall-(wide*2))+2,true) ); 
	pScrollBar->setPaintBorderEnabled(false);
	pScrollBar->setPaintBackgroundEnabled(false);
	pScrollBar->setPaintEnabled(false);

	pScrollBar = getHorizontalScrollBar();
	pScrollBar->setButton( new CTFScrollButton( ARROW_LEFT, "", 0,0,16,16 ), 0 );
	pScrollBar->setButton( new CTFScrollButton( ARROW_RIGHT, "", 0,0,16,16 ), 1 );
	pScrollBar->setSlider( new CTFSlider(tall,0,wide-(tall*2),tall,false) );
	pScrollBar->setPaintBorderEnabled(false);
	pScrollBar->setPaintBackgroundEnabled(false);
	pScrollBar->setPaintEnabled(false);
}


//=================================================================================
// CUSTOM HANDLERS
//=================================================================================
void CHandler_MenuButtonOver::cursorEntered(Panel *panel)
{
	if ( gViewPort && m_pMenuPanel )
	{
		m_pMenuPanel->SetActiveInfo( m_iButton );
	}
}

void CMenuHandler_StringCommandClassSelect::actionPerformed(Panel* panel)
{
	CMenuHandler_StringCommand::actionPerformed( panel );

	bool bAutoKill = CVAR_GET_FLOAT( "hud_classautokill" ) != 0;
	
}

