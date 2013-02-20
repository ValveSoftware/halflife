/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "cmdlib.h"
#define NO_THREAD_NAMES
#include "threads.h"

#define	MAX_THREADS	64

int		dispatch;
int		workcount;
int		oldf;
qboolean		pacifier;

qboolean	threaded;

/*
=============
GetThreadWork

=============
*/
int	GetThreadWork (void)
{
	int	r;
	int	f;

	ThreadLock ();

	if (dispatch == workcount)
	{
		ThreadUnlock ();
		return -1;
	}

	f = 10*dispatch / workcount;
	if (f != oldf)
	{
		oldf = f;
		if (pacifier)
			printf ("%i...", f);
	}

	r = dispatch;
	dispatch++;
	ThreadUnlock ();

	return r;
}


void (*workfunction) (int);

void ThreadWorkerFunction (int threadnum)
{
	int		work;

	while (1)
	{
		work = GetThreadWork ();
		if (work == -1)
			break;
		workfunction(work);
	}
}

void RunThreadsOnIndividual (int workcnt, qboolean showpacifier, void(*func)(int))
{
	workfunction = func;
	RunThreadsOn (workcnt, showpacifier, ThreadWorkerFunction);
}


/*
===================================================================

WIN32

===================================================================
*/
#ifdef WIN32

#define	USED

#include <windows.h>

int		numthreads = -1;
CRITICAL_SECTION		crit;
static int enter;

void ThreadSetDefault (void)
{
	SYSTEM_INFO info;

	if (numthreads == -1)	// not set manually
	{
		GetSystemInfo (&info);
		numthreads = info.dwNumberOfProcessors;
		if (numthreads < 1 || numthreads > 32)
			numthreads = 1;
	}

	qprintf ("%i threads\n", numthreads);
}


void ThreadLock (void)
{
	if (!threaded)
		return;
	EnterCriticalSection (&crit);
	if (enter)
		Error ("Recursive ThreadLock\n");
	enter = 1;
}

void ThreadUnlock (void)
{
	if (!threaded)
		return;
	if (!enter)
		Error ("ThreadUnlock without lock\n");
	enter = 0;
	LeaveCriticalSection (&crit);
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	int		threadid[MAX_THREADS];
	HANDLE	threadhandle[MAX_THREADS];
	int		i;
	int		start, end;

	start = I_FloatTime ();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = true;
	//
	// run threads in parallel
	//
	InitializeCriticalSection (&crit);
	for (i=0 ; i<numthreads ; i++)
	{
		threadhandle[i] = CreateThread(
		   NULL,	// LPSECURITY_ATTRIBUTES lpsa,
		   0,		// DWORD cbStack,
		   (LPTHREAD_START_ROUTINE)func,	// LPTHREAD_START_ROUTINE lpStartAddr,
		   (LPVOID)i,	// LPVOID lpvThreadParm,
		   0,			//   DWORD fdwCreate,
		   &threadid[i]);
	}

	for (i=0 ; i<numthreads ; i++)
		WaitForSingleObject (threadhandle[i], INFINITE);
	DeleteCriticalSection (&crit);

	threaded = false;
	end = I_FloatTime ();
	if (pacifier)
		printf (" (%i)\n", end-start);
}


#endif

/*
===================================================================

OSF1

===================================================================
*/

#ifdef __osf__
#define	USED

int		numthreads = 4;

void ThreadSetDefault (void)
{
	numthreads = 4;
}


#include <pthread.h>

pthread_mutex_t	*my_mutex;

void ThreadLock (void)
{
	if (my_mutex)
		pthread_mutex_lock (my_mutex);
}

void ThreadUnlock (void)
{
	if (my_mutex)
		pthread_mutex_unlock (my_mutex);
}


/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	int		i;
	pthread_t	work_threads[MAX_THREADS];
	pthread_addr_t	status;
	pthread_attr_t	attrib;
	pthread_mutexattr_t	mattrib;
	int		start, end;

	start = I_FloatTime ();
	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	threaded = true;

	if (pacifier)
		setbuf (stdout, NULL);

	if (!my_mutex)
	{
		my_mutex = malloc (sizeof(*my_mutex));
		if (pthread_mutexattr_create (&mattrib) == -1)
			Error ("pthread_mutex_attr_create failed");
		if (pthread_mutexattr_setkind_np (&mattrib, MUTEX_FAST_NP) == -1)
			Error ("pthread_mutexattr_setkind_np failed");
		if (pthread_mutex_init (my_mutex, mattrib) == -1)
			Error ("pthread_mutex_init failed");
	}

	if (pthread_attr_create (&attrib) == -1)
		Error ("pthread_attr_create failed");
	if (pthread_attr_setstacksize (&attrib, 0x100000) == -1)
		Error ("pthread_attr_setstacksize failed");
	
	for (i=0 ; i<numthreads ; i++)
	{
  		if (pthread_create(&work_threads[i], attrib
		, (pthread_startroutine_t)func, (pthread_addr_t)i) == -1)
			Error ("pthread_create failed");
	}
		
	for (i=0 ; i<numthreads ; i++)
	{
		if (pthread_join (work_threads[i], &status) == -1)
			Error ("pthread_join failed");
	}

	threaded = false;

	end = I_FloatTime ();
	if (pacifier)
		printf (" (%i)\n", end-start);
}


#endif

/*
=======================================================================

  SINGLE THREAD

=======================================================================
*/

#ifndef USED

int		numthreads = 1;

void ThreadSetDefault (void)
{
	numthreads = 1;
}

void ThreadLock (void)
{
}

void ThreadUnlock (void)
{
}

/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn (int workcnt, qboolean showpacifier, void(*func)(int))
{
	int		i;
	int		start, end;

	dispatch = 0;
	workcount = workcnt;
	oldf = -1;
	pacifier = showpacifier;
	start = I_FloatTime (); 
#ifdef NeXT
	if (pacifier)
		setbuf (stdout, NULL);
#endif
	func(0);

	end = I_FloatTime ();
	if (pacifier)
		printf (" (%i)\n", end-start);
}

#endif
