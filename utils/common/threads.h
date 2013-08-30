/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

extern	int		numthreads;

void ThreadSetDefault (void);
int	GetThreadWork (void);
void RunThreadsOnIndividual (int workcnt, qboolean showpacifier, void(*func)(int));
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int));
void ThreadLock (void);
void ThreadUnlock (void);

#ifndef NO_THREAD_NAMES
#define RunThreadsOn(n,p,f) { if (p) printf("%-20s ", #f ":"); RunThreadsOn(n,p,f); }
#define RunThreadsOnIndividual(n,p,f) { if (p) printf("%-20s ", #f ":"); RunThreadsOnIndividual(n,p,f); }
#endif

