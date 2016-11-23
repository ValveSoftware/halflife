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

#define _A( a ) { a, #a }

activity_map_t activity_map[] =
{
_A(	ACT_IDLE ),
_A(	ACT_HOP ),

_A(	ACT_FALL_FORWARD ),
_A(	ACT_FALL_BACKWARD ),
_A(	ACT_FALL_LEFT ),
_A(	ACT_FALL_RIGHT ),

_A(	ACT_FLINCH_BACK ),
_A(	ACT_FLINCH_LEFT ),
_A(	ACT_FLINCH_RIGHT ),
_A(	ACT_FLINCH_FORWARD ),

_A(	ACT_DIE_HEADSHOT ),
_A(	ACT_DIEFORWARD ),
_A(	ACT_DIEBACKWARD ),
0, NULL
};
