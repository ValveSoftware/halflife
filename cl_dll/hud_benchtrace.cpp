// hud_benchtrace.cpp
// Functions for spawning a thread to get a hopcount to a particular ip address and returning the result in a specified
//  variable

#ifdef _WIN32
#include "winsani_in.h"
#include <windows.h>
#include "winsani_out.h"
#else
#include "port.h"
#include <dlfcn.h>
#endif

// For tracking the trace threads
typedef struct
{
	// Inputs
	char	server[ 256 ];

	// Outputs
	int		*p_nresults;
	int		*p_ndone;
	
	// Local variables
	DWORD	hThreadId;
	HANDLE	hThread;
	HANDLE	hEventDone;
} trace_params_t;

// Static forces it to be zeroed out
static trace_params_t tp;

// For doing the actual traceroute
struct trace_options_s
{
	unsigned char ucTTL;
	unsigned char a[7];
};

struct
{
	DWORD			dwAddress;
	unsigned long	ulStatus, ulRoundTripTime;
	unsigned char	a[8];
	struct trace_options_s Options;
} traceReturn;

/*
==============
Trace_GetHopCount

Performs a synchronous hopcount on the specified server
==============
*/
int Trace_GetHopCount( char *pServer, int nMaxHops )
{
#ifdef _WIN32
	HMODULE					hICMP;			// Handle to ICMP .dll
	HANDLE					hIP;			// Handle to icmp session
	DWORD					*dwIPAddr;		// remote IP Address as a DWORD
	struct hostent			*pHostEnt;		// Name of remote host
	struct trace_options_s	traceOptions;	// Input options
	int						c;				// Hop counter

	// Prototypes
	HANDLE ( WINAPI *pfnICMPCreateFile ) ( VOID );
	BOOL ( WINAPI *pfnICMPCloseFile ) ( HANDLE );
	DWORD (WINAPI *pfnICMPSendEcho) ( HANDLE, DWORD, LPVOID, WORD, LPVOID, LPVOID, DWORD, DWORD );

	hICMP = ::LoadLibrary( "ICMP.DLL" );
	
	pfnICMPCreateFile	= ( HANDLE ( WINAPI *)(VOID ) )::GetProcAddress( hICMP,"IcmpCreateFile");
	pfnICMPCloseFile	= ( BOOL ( WINAPI *) ( HANDLE ) )::GetProcAddress( hICMP,"IcmpCloseHandle");
	pfnICMPSendEcho		= ( DWORD ( WINAPI * ) ( HANDLE, DWORD, LPVOID, WORD, LPVOID, LPVOID, DWORD,DWORD ) )::GetProcAddress( hICMP,"IcmpSendEcho" );
	
	if ( !pfnICMPCreateFile ||
		 !pfnICMPCloseFile ||
		 !pfnICMPSendEcho )
	{
		return -1;
	}

	hIP = pfnICMPCreateFile();
	if ( !hIP )
	{
		return -1;
	}

	// DNS lookup on remote host
	pHostEnt = gethostbyname( pServer );
	if ( !pHostEnt )
	{
		return -1;
	}

	// Take first IP address returned
	dwIPAddr = ( DWORD * )( *pHostEnt->h_addr_list );

	// Fixme:  If not tracing, can use a "binary search" method to do the trace route
	for ( c = 1; c <= nMaxHops ; c++)
	{
		// Set TTL correctly
		traceOptions.ucTTL = (unsigned char)c;

		// Clear out return structure
		memset( &traceReturn, 0, sizeof( traceReturn ) );

		// Send echo request, 2000 milliseconds maximum waiting time
		pfnICMPSendEcho ( hIP, *dwIPAddr, 0, 0, &traceOptions, &traceReturn, sizeof(traceReturn), 2000 );
		
		// Found requrested remote address, c contains the correct hopcount
		if ( traceReturn.dwAddress == *dwIPAddr )
			break;
	}

	/*
	// This is how you do a raw ping
	npings = 1;
	pfnICMPSendEcho( hIP, *dwIPAddr, 0, 0, NULL, &E, sizeof( E ), 2000 );
	*ping = (double)E.RoundTripTime / 1000.0;
	*/

	// Clean up file and dll handles
	pfnICMPCloseFile( hIP );

	::FreeLibrary( hICMP );

	// Failure?
	if ( c > nMaxHops )
	{
		return -1;
	}

	return c;
#else
	return -1;
#endif
}

/*
==============
Trace_Cleanup

Destroys thread and event handle when trace is done, or when restarting a new trace
==============
*/
void Trace_Cleanup( void )
{
#ifdef _WIN32
	if ( tp.hThread )
	{
		TerminateThread( tp.hThread, 0 );
		CloseHandle( tp.hThread );
		tp.hThread = (HANDLE)0;
	}

	if ( tp.hEventDone )
	{
		CloseHandle( tp.hEventDone );
		tp.hEventDone = (HANDLE)0;
	}
#endif
}

#ifdef _WIN32

/*
==============
Trace_ThreadFunction

Performs a trace, sets finish event and exits
==============
*/
DWORD WINAPI Trace_ThreadFunction( LPVOID p )
{
	int *results;

	results = ( int * )p;

	*results = Trace_GetHopCount( tp.server, 30 );
	SetEvent( tp.hEventDone );

	return 0;
}
#endif

/*
==============
Trace_StartTrace

Create finish event, sets up data, and starts thread to do a traceroute.
==============
*/
void Trace_StartTrace( int *results, int *finished, const char *server )
{
#ifdef _WIN32
	tp.p_nresults = results;
	strcpy( tp.server, server );

	*results = -1;

	Trace_Cleanup();

	tp.hEventDone = CreateEvent( NULL, TRUE, FALSE, NULL );
	if ( !tp.hEventDone )
	{
		return;
	}

	tp.p_ndone = finished;
	*tp.p_ndone = 0;

	tp.hThread = CreateThread( NULL, 0, Trace_ThreadFunction, results, 0, &tp.hThreadId );
#endif
}

/*
==============
Trace_Think

Invoked by general frame loop on client to periodically check if the traceroute thread has completed.
==============
*/
void Trace_Think( void )
{
#ifdef _WIN32
	if ( !tp.hEventDone )
		return;

	if ( WaitForSingleObject( tp.hEventDone, 0 ) == WAIT_OBJECT_0 )
	{
		Trace_Cleanup();
		*tp.p_ndone = 1;
	}
#endif
}