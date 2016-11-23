/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// statusbar.cpp
//
// generic text status bar, set by game dll
// runs across bottom of screen
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>

DECLARE_MESSAGE( m_StatusBar, StatusText );
DECLARE_MESSAGE( m_StatusBar, StatusValue );

#define STATUSBAR_ID_LINE		1

extern int GetTeamIndex( int clientIndex );
int g_iNameColors;

int CHudStatusBar :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( StatusText );
	HOOK_MESSAGE( StatusValue );

	Reset();

	return 1;
}

int CHudStatusBar :: VidInit( void )
{
	// Load sprites here
	m_iArmorSpriteIndex = gHUD.GetSpriteIndex( "armor_bar" );
	m_hArmor = gHUD.GetSprite( m_iArmorSpriteIndex );

	m_iHealthSpriteIndex = gHUD.GetSpriteIndex( "health_bar" );
	m_hHealth = gHUD.GetSprite( m_iHealthSpriteIndex );

	return 1;
}

void CHudStatusBar :: Reset( void )
{
	m_iFlags &= ~HUD_ACTIVE;  // start out inactive
	for ( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
		m_szStatusText[i][0] = 0;
	memset( m_iStatusValues, 0, sizeof m_iStatusValues );

	m_iStatusValues[0] = 1;  // 0 is the special index, which always returns true
}

void CHudStatusBar :: ParseStatusString( int line_num )
{
	int indexval;
	
	indexval = m_iStatusValues[ 1 ];

	GetPlayerInfo( indexval, &g_PlayerInfoList[indexval] );

	if ( g_PlayerInfoList[indexval].name != NULL )
	{
		strncpy( m_szName[line_num], g_PlayerInfoList[indexval].name, MAX_PLAYER_NAME_LENGTH );
	}
	else
	{
	    strncpy( m_szName[line_num], "******", MAX_PLAYER_NAME_LENGTH );
	}

	g_iNameColors = GetTeamIndex( indexval );
	
	indexval = m_iStatusValues[ 2 ];
    sprintf( m_szHealth[ line_num ], ":%d", indexval );
	
	indexval = m_iStatusValues[ 3 ];
    sprintf( m_szArmor[ line_num ], ":%d", indexval );

	m_iTeamMate[ line_num ] = m_iStatusValues[ 5 ];
}

int CHudStatusBar :: Draw( float fTime )
{
	int r , g, b, a, name_r, name_g, name_b;	

	if ( m_bReparseString )
	{
		for ( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
			ParseStatusString( i );
		m_bReparseString = FALSE;
	}

	//Not Watching anyone
	if ( m_iStatusValues[ 1 ] == 0 )
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 1;
	}

	// Draw the status bar lines
	for ( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
	{
		int TextHeight = 0;
		int TotalTextWidth = 0;

		//Ugly way to get
		if ( m_iTeamMate[i] )
		{
			TotalTextWidth += gHUD.ReturnStringPixelLength ( m_szName[i] );
			TotalTextWidth += gHUD.ReturnStringPixelLength ( m_szHealth[i] );
			TotalTextWidth += gHUD.ReturnStringPixelLength ( m_szArmor[i] );
			TotalTextWidth += 48;
			TextHeight = gHUD.m_scrinfo.iCharHeight;
		}
		else
			TotalTextWidth += gHUD.ReturnStringPixelLength ( m_szName[i] );

		TextHeight = gHUD.m_scrinfo.iCharHeight;
	
		if ( g_iNameColors == 1 )
		{
			name_r = 255;
			name_g = 50;
			name_b = 50;
		}
		else if ( g_iNameColors == 2 )
		{
			name_r = 50;
			name_g = 50;
			name_b = 255;
		}
		else
			name_r = name_g = name_b = 255;
			 
		int Y_START;
		if ( ScreenHeight >= 480 )
			Y_START = ScreenHeight - 55;
		else
			Y_START = ScreenHeight - 45;

		int x = gHUD.m_Ammo.m_iNumberXPosition;
		int y = Y_START; // = ( ScreenHeight / 2 ) + ( TextHeight * 3 );
			
		int x_offset;
		a = 200;

		UnpackRGB( r, g, b, RGB_NORMAL);
		ScaleColors( r, g, b, a );
		ScaleColors( name_r, name_g, name_b, 125 );
	
		//Draw the name First
		gHUD.DrawHudString( x, y, 1024, m_szName[i], name_r, name_g, name_b );

		if ( !m_iTeamMate[i] )
			continue;

		//Get the length in pixels for the name
		x_offset = gHUD.ReturnStringPixelLength ( m_szName[i] );

		//Add the offset
		x += ( x_offset + 8 );
		
		//Now draw the Sprite for the health
		SPR_Set( m_hHealth, r, g, b );
		SPR_DrawHoles( 0, x , y, &gHUD.GetSpriteRect( m_iHealthSpriteIndex ) );
	
		//Add the sprite width size
		x += 16;

		//Draw the health value ( x + offset for the name lenght + width of the sprite )
		gHUD.DrawHudString( x, y, 1024, m_szHealth[i], name_r, name_g, name_b );

		//Get the length in pixels for the health
		x_offset = gHUD.ReturnStringPixelLength ( m_szHealth[i] );

		//Add the offset
		x += ( x_offset + 8 );

		//Now draw the Sprite for the Armor
		SPR_Set( m_hArmor, r, g, b );
		SPR_DrawHoles( 0, x, y, &gHUD.GetSpriteRect( m_iArmorSpriteIndex ) );

		x += 16;

		//Draw the armor value ( x + offset for the name lenght + width of the sprite )
		gHUD.DrawHudString( x, y, 1024, m_szArmor[i], name_r, name_g, name_b );
	}

	return 1;
}

// Message handler for StatusText message
// accepts two values:
//		byte: line number of status bar text 
//		string: status bar text
// this string describes how the status bar should be drawn
// a semi-regular expression:
// ( slotnum ([a..z] [%pX] [%iX])*)*
// where slotnum is an index into the Value table (see below)
// if slotnum is 0, the string is always drawn
// if StatusValue[slotnum] != 0, the following string is drawn, upto the next newline - otherwise the text is skipped upto next newline
// %pX, where X is an integer, will substitute a player name here, getting the player index from StatusValue[X]
// %iX, where X is an integer, will substitute a number here, getting the number from StatusValue[X]
int CHudStatusBar :: MsgFunc_StatusText( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int line = READ_BYTE();

	if ( line < 0 || line >= MAX_STATUSBAR_LINES )
		return 1;

	strncpy( m_szStatusText[line], READ_STRING(), MAX_STATUSTEXT_LENGTH );
	m_szStatusText[line][MAX_STATUSTEXT_LENGTH-1] = 0;  // ensure it's null terminated ( strncpy() won't null terminate if read string too long)

	if ( m_szStatusText[0] == 0 )
		m_iFlags &= ~HUD_ACTIVE;
	else
		m_iFlags |= HUD_ACTIVE;  // we have status text, so turn on the status bar

	m_bReparseString = TRUE;

	return 1;
}

// Message handler for StatusText message
// accepts two values:
//		byte: index into the status value array
//		short: value to store
int CHudStatusBar :: MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int index = READ_BYTE();
	if ( index < 1 || index >= MAX_STATUSBAR_VALUES )
		return 1; // index out of range

	m_iStatusValues[index] = READ_SHORT();

	m_iFlags |= HUD_ACTIVE;

	m_bReparseString = TRUE;
	
	return 1;
}