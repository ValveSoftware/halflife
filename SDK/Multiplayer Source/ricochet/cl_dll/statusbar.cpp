/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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

int CHudStatusBar :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( StatusText );
	HOOK_MESSAGE( StatusValue );

	Reset();

	CVAR_CREATE( "hud_centerid", "0", FCVAR_ARCHIVE );

	return 1;
}

int CHudStatusBar :: VidInit( void )
{
	// Load sprites here

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
	// localise string first
	char szBuffer[MAX_STATUSTEXT_LENGTH];
	memset( szBuffer, 0, sizeof szBuffer );
	gHUD.m_TextMessage.LocaliseTextString( m_szStatusText[line_num], szBuffer, MAX_STATUSTEXT_LENGTH );

	// parse m_szStatusText & m_iStatusValues into m_szStatusBar
	memset( m_szStatusBar[line_num], 0, MAX_STATUSTEXT_LENGTH );
	char *src = szBuffer;
	char *dst = m_szStatusBar[line_num];

	char *src_start = src, *dst_start = dst;

	while ( *src != 0 )
	{
		while ( *src == '\n' )
			src++;  // skip over any newlines

		if ( ((src - src_start) >= MAX_STATUSTEXT_LENGTH) || ((dst - dst_start) >= MAX_STATUSTEXT_LENGTH) )
			break;

		int index = atoi( src );
		// should we draw this line?
		if ( (index >= 0 && index < MAX_STATUSBAR_VALUES) && (m_iStatusValues[index] != 0) )
		{  // parse this line and append result to the status bar
			while ( *src >= '0' && *src <= '9' )
				src++;

			if ( *src == '\n' || *src == 0 )
				continue; // no more left in this text line

			// copy the text, char by char, until we hit a % or a \n
			while ( *src != '\n' && *src != 0 )
			{
				if ( *src != '%' )
				{  // just copy the character
					*dst = *src;
					dst++, src++;
				}
				else
				{
					// get the descriptor
					char valtype = *(++src); // move over %

					// if it's a %, draw a % sign
					if ( valtype == '%' )
					{
						*dst = valtype;
						dst++, src++;
						continue;
					}

					// move over descriptor, then get and move over the index
					index = atoi( ++src ); 
					while ( *src >= '0' && *src <= '9' )
						src++;

					if ( index >= 0 && index < MAX_STATUSBAR_VALUES )
					{
						int indexval = m_iStatusValues[index];

						// get the string to substitute in place of the %XX
						char szRepString[MAX_PLAYER_NAME_LENGTH];
						switch ( valtype )
						{
						case 'p':  // player name
							GetPlayerInfo( indexval, &g_PlayerInfoList[indexval] );
							if ( g_PlayerInfoList[indexval].name != NULL )
							{
								strncpy( szRepString, g_PlayerInfoList[indexval].name, MAX_PLAYER_NAME_LENGTH );
							}
							else
							{
								strcpy( szRepString, "******" );
							}
							break;
						case 'i':  // number
							sprintf( szRepString, "%d", indexval );
							break;
						default:
							szRepString[0] = 0;
						}

						for ( char *cp = szRepString; *cp != 0 && ((dst - dst_start) < MAX_STATUSTEXT_LENGTH); cp++, dst++ )
							*dst = *cp;
					}
				}
			}
		}
		else
		{
			// skip to next line of text
			while ( *src != 0 && *src != '\n' )
				src++;
		}
	}
}

int CHudStatusBar :: Draw( float fTime )
{
	if ( m_bReparseString )
	{
		for ( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
			ParseStatusString( i );
		m_bReparseString = FALSE;
	}

	// Draw the status bar lines
	for ( int i = 0; i < MAX_STATUSBAR_LINES; i++ )
	{
		int TextHeight, TextWidth;
		GetConsoleStringSize( m_szStatusBar[i], &TextWidth, &TextHeight );

		int Y_START;
		if ( ScreenHeight >= 480 )
			Y_START = ScreenHeight - 45;
		else
			Y_START = ScreenHeight - 35;

		int x = 5;
		int y = Y_START - ( TextHeight * i ); // draw along bottom of screen

		// let user set status ID bar centering
		if ( (i == STATUSBAR_ID_LINE) && CVAR_GET_FLOAT("hud_centerid") )
		{
			x = max( 0, max(2, (ScreenWidth - TextWidth)) / 2 );
			y = (ScreenHeight / 2) + (TextHeight*CVAR_GET_FLOAT("hud_centerid"));
		}

		DrawConsoleString( x, y, m_szStatusBar[i] );
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

	m_bReparseString = TRUE;
	
	return 1;
}