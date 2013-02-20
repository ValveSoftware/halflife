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
// hud_redraw.cpp
//
#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include "vgui_viewport.h"

extern int g_iVisibleMouse;

#define MAX_LOGO_FRAMES 56

int grgLogoFrame[MAX_LOGO_FRAMES] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
	16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
	29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31 
};


// Think
void CHud::Think(void)
{
	HUDLIST *pList = m_pHudList;
	while (pList)
	{
		if (pList->p->m_iFlags & HUD_ACTIVE)
			pList->p->Think();
		pList = pList->pNext;
	}

	// think about default fov
	if ( m_iFOV == 0 )
	{  // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = max( default_fov->value, 90 );  
	}

	
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud :: Redraw( float flTime, int intermission )
{
	m_fOldTime = m_flTime;	// save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static m_flShotTime = 0;
	
	// Clock was reset, reset delta
	if ( m_flTimeDelta < 0 )
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if ( m_iIntermission && !intermission )
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			gViewPort->UpdateSpectatorPanel();
		}
		else if ( !m_iIntermission && intermission )
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if ( CVAR_GET_FLOAT( "hud_takesshots" ) != 0 )
				m_flShotTime = flTime + 1.0;	// Take a screenshot in a second
		}
	}

	if (m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}
	// if no redrawing is necessary
	// return 0;
	
	// draw all registered HUD elements
	if ( m_pCvarDraw->value )
	{
		HUDLIST *pList = m_pHudList;

		while (pList)
		{
			if ( !intermission )
			{
				if ((pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL))
					pList->p->Draw(flTime);
			}
			else
			{  // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if ( pList->p->m_iFlags & HUD_INTERMISSION )
					pList->p->Draw( flTime );
			}

			pList = pList->pNext;
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250 );
		
		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0)/2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}
	

	return 1;
}

void ScaleColors( int &r, int &g, int &b, int a )
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}



/*
===========================
int ReturnStringPixelLength ( char *Hihi )

Returns a integer representing the length of the string passed
===========================
*/
int CHud :: ReturnStringPixelLength ( char *Hihi )
{
	int iNameLength = 0;

	int strleng = ( strlen( Hihi ) );

	for ( int har = 0; har < strleng; har++)
		iNameLength += gHUD.m_scrinfo.charWidths[ Hihi[har] ];

	return iNameLength;
}

int LastColor;

int CHud :: DrawHudStringCTF(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b )
{
	int WantColor = 0;
	int Color = 0;
	
	// draw the string until we hit the null character or a newline character
	for ( ; *szIt != 0 && *szIt != '\n'; szIt++ )
	{
		int next;// = xpos + gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		
	/*	if ( next > iMaxX )
			return xpos;*/


		if (*szIt == '\\')
		{
			if (Color > 0)
				Color = 0;

			WantColor = 1;
			
		}

		if (WantColor == 1 && *szIt == 'w')
		{
			Color = 1;
			LastColor = Color;
		}
			
		
		if (WantColor == 1 && *szIt == 'g')
		{
			Color = 2;
			LastColor = Color;
		}
			
		

		if (WantColor == 1 && *szIt == 'b')
		{
			Color = 3;
			LastColor = Color;
		}
			
		
		if (WantColor == 1 && *szIt == 'r')
		{
			Color = 4;
			LastColor = Color;
		}
		
		

		if (WantColor == 1 && *szIt == 'y')
		{
			Color = 5;
			LastColor = Color;
		}
			
		

		if (WantColor == 1 && *szIt == 'q')
		{
			Color = 6;
			LastColor = Color;
		}

			

		if (Color == 0 && WantColor == 0)
		{
			if (LastColor == 1)
				TextMessageDrawChar( xpos, ypos, *szIt, 255, 255, 255 );
			if (LastColor == 2)
				TextMessageDrawChar( xpos, ypos, *szIt, 0, 79, 0);
			if (LastColor == 3)
				TextMessageDrawChar( xpos, ypos, *szIt, 0, 0, 200);
			if (LastColor == 4)
				TextMessageDrawChar( xpos, ypos, *szIt, 200, 0, 0 );
			if (LastColor == 5)
				TextMessageDrawChar( xpos, ypos, *szIt, 198, 221, 66 );
			if (LastColor == 6)
				TextMessageDrawChar( xpos, ypos, *szIt, 136, 136, 136 );
			
			else if (LastColor == 0)
				TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );

			next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ];
		}
		
		else if (Color > 0 && WantColor == 0 )
		{
			if (Color == 1)
				TextMessageDrawChar( xpos, ypos, *szIt, 255, 255, 255 );
			if (Color == 2)
				TextMessageDrawChar( xpos, ypos, *szIt, 0, 79, 0);
			if (Color == 3)
				TextMessageDrawChar( xpos, ypos, *szIt, 0, 0, 200);
			if (Color == 4)
				TextMessageDrawChar( xpos, ypos, *szIt, 200, 0, 0 );
			if (Color == 5)
				TextMessageDrawChar( xpos, ypos, *szIt, 198, 221, 66 );
			if (Color == 6)
				TextMessageDrawChar( xpos, ypos, *szIt, 136, 136, 136 );

			next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ];
		}

		else if (Color > 0 && WantColor == 1)
		{
			//next = xpos + (gHUD.m_scrinfo.charWidths[ *szIt ] * 2); // variable-width fonts look cool
			WantColor = 0;
		}

			
		xpos = next;		
	}

	return xpos;
}


int CHud :: DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b )
{
	// draw the string until we hit the null character or a newline character
	for ( ; *szIt != 0 && *szIt != '\n'; szIt++ )
	{
		int next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next > iMaxX )
			return xpos;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
		xpos = next;		
	}

	return xpos;
}

int CHud :: DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b )
{
	char szString[32];
	sprintf( szString, "%d", iNumber );
	return DrawHudStringReverse( xpos, ypos, iMinX, szString, r, g, b );

}

// draws a string from right to left (right-aligned)
int CHud :: DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b )
{
	// find the end of the string
	for ( char *szIt = szString; *szIt != 0; szIt++ )
	{ // we should count the length?		
	}

	// iterate throug the string in reverse
	for ( szIt--;  szIt != (szString-1);  szIt-- )	
	{
		int next = xpos - gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next < iMinX )
			return xpos;
		xpos = next;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
	}

	return xpos;
}

int CHud :: DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;
	
	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			 k = iNumber/100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100)/10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
		SPR_DrawAdditive(0,  x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	} 
	else if (iFlags & DHN_DRAWZERO) 
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b );

		// SPR_Draw 100's
		if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		
		SPR_DrawAdditive( 0,  x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth( int iNumber, int iFlags )
{
	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;

}	


