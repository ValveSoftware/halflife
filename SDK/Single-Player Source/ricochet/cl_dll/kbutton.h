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
#if !defined( KBUTTONH )
#define KBUTTONH
#pragma once

typedef struct kbutton_s
{
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state
} kbutton_t;

#endif // !KBUTTONH