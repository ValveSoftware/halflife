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
#ifndef PROGS_H
#define PROGS_H

#include "progdefs.h"

// 16 simultaneous events, max
#define MAX_EVENT_QUEUE 64

#define DEFAULT_EVENT_RESENDS 1

#include "event_flags.h"

typedef struct event_info_s event_info_t;

#include "event_args.h"

struct event_info_s
{
	unsigned short index;			  // 0 implies not in use

	short packet_index;      // Use data from state info for entity in delta_packet .  -1 implies separate info based on event
	                         // parameter signature
	short entity_index;      // The edict this event is associated with

	float fire_time;        // if non-zero, the time when the event should be fired ( fixed up on the client )
	
	event_args_t args;

// CLIENT ONLY	
	int	  flags;			// Reliable or not, etc.

};

typedef struct event_state_s event_state_t;

struct event_state_s
{
	struct event_info_s ei[ MAX_EVENT_QUEUE ];
};

#if !defined( ENTITY_STATEH )
#include "entity_state.h"
#endif

#if !defined( EDICT_H )
#include "edict.h"
#endif

#define	STRUCT_FROM_LINK(l,t,m) ((t *)((byte *)l - (int)&(((t *)0)->m)))
#define	EDICT_FROM_AREA(l) STRUCT_FROM_LINK(l,edict_t,area)

//============================================================================

extern	char			*pr_strings;
extern	globalvars_t	gGlobalVariables;

//============================================================================

edict_t		*ED_Alloc (void);
void		ED_Free (edict_t *ed);
void		ED_LoadFromFile (char *data);

edict_t		*EDICT_NUM(int n);
int			NUM_FOR_EDICT(const edict_t *e);

#define PROG_TO_EDICT(e) ((edict_t *)((byte *)sv.edicts + e))

#endif // PROGS_H