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
/*

===== quake_gun.h ========================================================

  This is a half-life weapon that fires every one of the quake weapons.
  It's automatically given to all players.

*/
#include "effects.h"

class CQuakeGun : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p);

	int SuperDamageSound( void );

	void PrimaryAttack( void );
	BOOL Deploy( void );

	void UpdateEffect( void );

	void CreateEffect ( void );
	void DestroyEffect ( void );

	CBeam	*m_pBeam;
};
