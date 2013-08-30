/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#ifndef MONSTEREVENT_H
#define MONSTEREVENT_H

typedef struct
{
	int			event;
	char		*options;
} MonsterEvent_t;

#define EVENT_SPECIFIC			0
#define EVENT_SCRIPTED			1000
#define EVENT_SHARED			2000
#define EVENT_CLIENT			5000

#define MONSTER_EVENT_BODYDROP_LIGHT	2001
#define MONSTER_EVENT_BODYDROP_HEAVY	2002

#define MONSTER_EVENT_SWISHSOUND		2010

#endif		// MONSTEREVENT_H
