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
// pm_shared.h
//
#if !defined( PM_SHAREDH )
#define PM_SHAREDH
#pragma once

void PM_Init( struct playermove_s *ppmove );
void PM_Move ( struct playermove_s *ppmove, int server );
char PM_FindTextureType( char *name );

// Spectator flags
#define SPEC_IS_SPECTATOR		(1<<0)
#define SPEC_SMOOTH_ANGLES		(1<<1)
#define SPEC_SMOOTH_ORIGIN		(1<<2)

#endif