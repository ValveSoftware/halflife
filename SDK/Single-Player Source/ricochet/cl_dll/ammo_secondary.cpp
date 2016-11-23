/***
*
*	Copyright (c) 1999, 2000 Valve LLC. All rights reserved.
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
// ammo_secondary.cpp
//
// implementation of CHudAmmoSecondary class
//

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE( m_AmmoSecondary, SecAmmoVal );
DECLARE_MESSAGE( m_AmmoSecondary, SecAmmoIcon );

int CHudAmmoSecondary :: Init( void )
{
	HOOK_MESSAGE( SecAmmoVal );
	HOOK_MESSAGE( SecAmmoIcon );

	gHUD.AddHudElem(this);
	m_HUD_ammoicon = 0;

	for ( int i = 0; i < MAX_SEC_AMMO_VALUES; i++ )
		m_iAmmoAmounts[i] = -1;  // -1 means don't draw this value

	Reset();

	return 1;
}

void CHudAmmoSecondary :: Reset( void )
{
	m_fFade = 0;
}

int CHudAmmoSecondary :: VidInit( void )
{
	return 1;
}

int CHudAmmoSecondary :: Draw(float flTime)
{
	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// draw secondary ammo icons above normal ammo readout
	int a, x, y, r, g, b, AmmoWidth;
	UnpackRGB( r, g, b, RGB_YELLOWISH );
	a = (int) max( MIN_ALPHA, m_fFade );
	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);  // slowly lower alpha to fade out icons
	ScaleColors( r, g, b, a );

	AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	y = ScreenHeight - (gHUD.m_iFontHeight*4);  // this is one font height higher than the weapon ammo values
	x = ScreenWidth - AmmoWidth;

	if ( m_HUD_ammoicon )
	{
		// Draw the ammo icon
		x -= (gHUD.GetSpriteRect(m_HUD_ammoicon).right - gHUD.GetSpriteRect(m_HUD_ammoicon).left);
		y -= (gHUD.GetSpriteRect(m_HUD_ammoicon).top - gHUD.GetSpriteRect(m_HUD_ammoicon).bottom);

		SPR_Set( gHUD.GetSprite(m_HUD_ammoicon), r, g, b );
		SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(m_HUD_ammoicon) );
	}
	else
	{  // move the cursor by the '0' char instead, since we don't have an icon to work with
		x -= AmmoWidth;
		y -= (gHUD.GetSpriteRect(gHUD.m_HUD_number_0).top - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).bottom);
	}

	// draw the ammo counts, in reverse order, from right to left
	for ( int i = MAX_SEC_AMMO_VALUES-1; i >= 0; i-- )
	{
		if ( m_iAmmoAmounts[i] < 0 )
			continue; // negative ammo amounts imply that they shouldn't be drawn

		// half a char gap between the ammo number and the previous pic
		x -= (AmmoWidth / 2);

		// draw the number, right-aligned
		x -= (gHUD.GetNumWidth( m_iAmmoAmounts[i], DHN_DRAWZERO ) * AmmoWidth);
		gHUD.DrawHudNumber( x, y, DHN_DRAWZERO, m_iAmmoAmounts[i], r, g, b );

		if ( i != 0 )
		{
			// draw the divider bar
			x -= (AmmoWidth / 2);
			FillRGBA(x, y, (AmmoWidth/10), gHUD.m_iFontHeight, r, g, b, a);
		}
	}

	return 1;
}

// Message handler for Secondary Ammo Value
// accepts one value:
//		string:  sprite name
int CHudAmmoSecondary :: MsgFunc_SecAmmoIcon( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_HUD_ammoicon = gHUD.GetSpriteIndex( READ_STRING() );

	return 1;
}

// Message handler for Secondary Ammo Icon
// Sets an ammo value
// takes two values:
//		byte:  ammo index
//		byte:  ammo value
int CHudAmmoSecondary :: MsgFunc_SecAmmoVal( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int index = READ_BYTE();
	if ( index < 0 || index >= MAX_SEC_AMMO_VALUES )
		return 1;

	m_iAmmoAmounts[index] = READ_BYTE();
	m_iFlags |= HUD_ACTIVE;

	// check to see if there is anything left to draw
	int count = 0;
	for ( int i = 0; i < MAX_SEC_AMMO_VALUES; i++ )
	{
		count += max( 0, m_iAmmoAmounts[i] );
	}

	if ( count == 0 ) 
	{	// the ammo fields are all empty, so turn off this hud area
		m_iFlags &= ~HUD_ACTIVE;
		return 1;
	}

	// make the icons light up
	m_fFade = 200.0f;

	return 1;
}


