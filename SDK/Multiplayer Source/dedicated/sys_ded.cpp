//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifdef _WIN32
#include <windows.h> 
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#else
#include <string.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#endif

//#define FRAMERATE // define me to have hlds print out what it thinks the framerate is

#include <stdio.h>
#include <stdlib.h>

#include "sys_ded.h"
#include "conproc.h"
#include "dedicated.h"
#include "exefuncs.h"

#include "dll_state.h"
#include "enginecallback.h"
#if defined( _WIN32 )
#include "../utils/procinfo/procinfo.h"
#endif

#ifdef _WIN32
static	HANDLE	hinput;
static  HANDLE	houtput;

static const char *g_pszengine = "swds.dll";
#else
static const char *g_pszengine = "engine_i386.so";
static char g_szEXEName[ 256 ];
#endif

// System Memory & Size
static unsigned char	*gpMemBase = NULL;
static int				giMemSize = 0x2000000;  // 32 Mb default heapsize

exefuncs_t ef;

static char	console_text[256];
static int	console_textlen;

static long hDLLThirdParty = 0L;

char			*gpszCmdLine = NULL;

SleepType Sys_Sleep;


void Sys_Sleep_Old( int msec )
{
#ifdef _WIN32
	Sleep( msec );
#else
    usleep(msec * 1000);
#endif
}
#if !defined ( _WIN32 )

typedef int (*NET_Sleep_t)( void );
NET_Sleep_t NET_Sleep =NULL;
extern long ghMod; // from engine.cpp

void Sys_Sleep_Net( int msec )
{
	if ( NET_Sleep != NULL) 
	{
		NET_Sleep();
	} 
	else
	{
		Sys_Sleep_Old(msec); // NET_Sleep isn't hooked yet, fallback to the old method
	}
}

volatile char paused=0; // this checks if pause has run yet, tell the compiler it can change at any time
// for Sys_Sleep_Timer()
void alarmFunc(int num) {
	signal( SIGALRM, alarmFunc ); // reset the signal handler
	if(!paused) { // paused is 0, the timer has fired before the pause was called... Lets queue it again
		struct itimerval itim;  
		itim.it_interval.tv_sec = 0;   
		itim.it_interval.tv_usec = 0;
		itim.it_value.tv_sec = 0;
		itim.it_value.tv_usec = 1000; // get it to run again real soon
		setitimer( ITIMER_REAL, &itim, 0 );
	}	

}
#endif

void Sys_Sleep_Timer( int msec )
{
#ifdef _WIN32
	Sleep( msec );
#else
// linux runs on a 100Hz scheduling clock, so the minimum latency from
// usleep is 10msec. However, people want lower latency than this.. 
//
// There are a few solutions, one is to use the realtime scheduler in the 
// kernel BUT this needs root privelleges to start. It also can play
// unfriendly with other programs.

// Another solution is to use software timers, they use the RTC of the 
// system and are accurate to microseconds (or so).

// timers, via setitimer() are used here

	struct itimerval tm;

	tm.it_value.tv_sec=msec/1000; // convert msec to seconds
	tm.it_value.tv_usec=(msec%1000)*1E3; // get the number of msecs and change to micros
	tm.it_interval.tv_sec  = 0;
        tm.it_interval.tv_usec = 0;

	paused=0;
	if( setitimer(ITIMER_REAL,&tm,NULL)==0) 
	{ // set the timer to trigger
		pause();	 // wait for the signal
	}
	paused=1;

#endif
}


void Sys_Sleep_Select( int msec )
{
#ifdef _WIN32
	Sleep( msec );
#else	// _WIN32
	struct timeval tv;

	// Assumes msec < 1000
	tv.tv_sec	= 0;
	tv.tv_usec	= 1000 * msec;

	select( 1, NULL, NULL, NULL, &tv );
#endif	// _WIN32
}


void *Sys_GetProcAddress( long library, const char *name )
{
#ifdef _WIN32
	return ( void * )GetProcAddress( (HMODULE)library, name );
#else // LINUX
	return dlsym( (void *)library, name );
#endif
}

long Sys_LoadLibrary( char *lib )
{
	void *hDll = NULL;

#ifdef _WIN32
	hDll = ::LoadLibrary( lib );
#else
    char    cwd[1024];
    char    absolute_lib[1024];
    
    if (!getcwd(cwd, sizeof(cwd)))
        Sys_ErrorMessage(1, "Sys_LoadLibrary: Couldn't determine current directory.");
        
    if (cwd[strlen(cwd)-1] == '/')
        cwd[strlen(cwd)-1] = 0;
        
    snprintf(absolute_lib, sizeof(absolute_lib), "%s/%s", cwd, lib);
    
    hDll = dlopen( absolute_lib, RTLD_NOW );
    if ( !hDll )
    {
        Sys_ErrorMessage( 1, dlerror() );
    }   
#endif
	return (long)hDll;
}

void Sys_FreeLibrary( long library )
{
	if ( !library )
		return;

#ifdef _WIN32
	::FreeLibrary( (HMODULE)library );
#else
	dlclose( (void *)library );
#endif
}

int Sys_GetExecutableName( char *out )
{
#ifdef _WIN32
	if ( !::GetModuleFileName( ( HINSTANCE )GetModuleHandle( NULL ), out, 256 ) )
	{
		return 0;
	}
#else
	strcpy( out, g_szEXEName );
#endif
	return 1;
}

/*
==============
Sys_ErrorMessage

Engine is erroring out, display error in message box
==============
*/
void Sys_ErrorMessage( int level, const char *msg )
{
#ifdef _WIN32
	MessageBox( NULL, msg, "Half-Life", MB_OK );
	PostQuitMessage(0);	
#else
	printf( "%s\n", msg );
	exit( -1 );
#endif
}

#ifdef _WIN32
/*
==============
UpdateStatus

Update status line at top of console if engine is running
==============
*/
void UpdateStatus( int force )
{
	static double tLast = 0.0;
	double	tCurrent;
	char	szPrompt[256];
	int		n, spec, nMax;
	char	szMap[32];
	float	fps;

	if ( !engineapi.Host_GetHostInfo )
		return;

	tCurrent = (float)( timeGetTime() / 1000.0f );

	engineapi.Host_GetHostInfo( &fps, &n, &spec, &nMax, szMap );

	if ( !force )
	{
		if ( ( tCurrent - tLast ) < 0.5f )
			return;
	}

	tLast = tCurrent;

	snprintf( szPrompt, sizeof(szPrompt), "%.1f fps %2i(%2i spec)/%2i on %16s", (float)fps, n, spec, nMax, szMap);

	WriteStatusText( szPrompt );
}
#endif

/*
================
Sys_ConsoleOutput

Print text to the dedicated console
================
*/
void Sys_ConsoleOutput (char *string)
{
#ifdef _WIN32
	unsigned long dummy;
	char	text[256];

	if (console_textlen)
	{
		text[0] = '\r';
		memset(&text[1], ' ', console_textlen);
		text[console_textlen+1] = '\r';
		text[console_textlen+2] = 0;
		WriteFile(houtput, text, console_textlen+2, &dummy, NULL);
	}

	WriteFile(houtput, string, strlen(string), &dummy, NULL);

	if (console_textlen)
	{
		WriteFile(houtput, console_text, console_textlen, &dummy, NULL);
	}
	UpdateStatus( 1 /* force */ );
#else
	printf( "%s", string );
	fflush( stdout );
#endif
}

/*
==============
Sys_Printf

Engine is printing to console
==============
*/
void Sys_Printf(char *fmt, ...)
{
	// Dump text to debugging console.
	va_list argptr;
	char szText[1024];

	va_start (argptr, fmt);
	vsnprintf (szText, sizeof(szText), fmt, argptr);
	va_end (argptr);

	// Get Current text and append it.
	Sys_ConsoleOutput( szText );
}

/*
==============
Load3rdParty

Load support for third party .dlls ( gamehost )
==============
*/
void Load3rdParty( void )
{
	// Only do this if the server operator wants the support.
	// ( In case of malicious code, too )
	if ( CheckParm( "-usegh" ) )   
	{
		hDLLThirdParty = Sys_LoadLibrary( "ghostinj.dll" );
	}
}

/*
==============
EF_VID_ForceUnlockedAndReturnState

Dummy funcion called by engine
==============
*/
int  EF_VID_ForceUnlockedAndReturnState(void)
{
	return 0;
}

/*
==============
EF_VID_ForceLockState

Dummy funcion called by engine
==============
*/
void EF_VID_ForceLockState(int)
{
}

/*
==============
CheckParm

Search for psz in command line to .exe, if **ppszValue is set, then the pointer is
 directed at the NEXT argument in the command line
==============
*/
char *CheckParm(const char *psz, char **ppszValue)
{
	int i;
	static char sz[128];
	char *pret;

	if (!gpszCmdLine)
		return NULL;

	pret = strstr( gpszCmdLine, psz );

	// should we return a pointer to the value?
	if (pret && ppszValue)
	{
		char *p1 = pret;
		*ppszValue = NULL;

		while ( *p1 && (*p1 != 32))
			p1++;

		if (p1 != 0)
		{
			char *p2 = ++p1;

			for ( i = 0; i < 128; i++ )
			{
				if ( !*p2 || (*p2 == 32))
					break;
				sz[i] = *p2++;
			}

			sz[i] = 0;
			*ppszValue = &sz[0];		
		}	
	}

	return pret;
}

/*
==============
InitInstance

==============
*/
int InitInstance( void )
{
	Load3rdParty();

	Eng_SetState( DLL_INACTIVE );

	memset( &ef, 0, sizeof( ef ) );
	
	// Function pointers used by dedicated server
	ef.Sys_Printf							= Sys_Printf;
	ef.ErrorMessage							= Sys_ErrorMessage;

	ef.VID_ForceLockState					= EF_VID_ForceLockState;
	ef.VID_ForceUnlockedAndReturnState		= EF_VID_ForceUnlockedAndReturnState;

#ifdef _WIN32
	// Data
	ef.fMMX									= PROC_IsMMX();
	ef.iCPUMhz								= PROC_GetSpeed();	// in MHz
#endif

	return 1;
}

/*
================
Sys_ConsoleInput

================
*/
#ifdef _WIN32
char *Sys_ConsoleInput (void)
{
	INPUT_RECORD	recs[1024];
	unsigned long	dummy;
	int				ch;
	unsigned long	numread, numevents;

	while ( 1 )
	{
		if (!GetNumberOfConsoleInputEvents (hinput, &numevents))
		{
			exit( -1 );
		}

		if (numevents <= 0)
			break;

		if ( !ReadConsoleInput(hinput, recs, 1, &numread) )
		{
			exit( -1 );
		}

		if (numread != 1)
		{
			exit( -1 );
		}

		if ( recs[0].EventType == KEY_EVENT )
		{
			if ( !recs[0].Event.KeyEvent.bKeyDown )
			{
				ch = recs[0].Event.KeyEvent.uChar.AsciiChar;
				switch (ch)
				{
					case '\r':
						WriteFile(houtput, "\r\n", 2, &dummy, NULL);	
						if (console_textlen)
						{
							console_text[console_textlen] = 0;
							console_textlen = 0;
							return console_text;
						}
						break;

					case '\b':
						if (console_textlen)
						{
							console_textlen--;
							WriteFile(houtput, "\b \b", 3, &dummy, NULL);	
						}
						break;

					default:
						if (ch >= ' ')
						{
							if (console_textlen < sizeof(console_text)-2)
							{
								WriteFile(houtput, &ch, 1, &dummy, NULL);	
								console_text[console_textlen] = ch;
								console_textlen++;
							}
						}

						break;

				}
			}
		}
	}

	return NULL;
}
#else
char *Sys_ConsoleInput(void)
{
	struct timeval	tvTimeout;
	fd_set			fdSet;
	unsigned char	ch;
 
	FD_ZERO( &fdSet );
	FD_SET( STDIN_FILENO, &fdSet );

	tvTimeout.tv_sec        = 0;
	tvTimeout.tv_usec       = 0;

	if ( select( 1, &fdSet, NULL, NULL, &tvTimeout ) == -1 || !FD_ISSET( STDIN_FILENO, &fdSet ) )
		return NULL;

	console_textlen = 0;

	// We're going to shove a newline onto the end of the input later in
	// ProcessConsoleInput(), so only accept 254 chars instead of 255.    -LH

	while ( read( STDIN_FILENO, &ch, 1 ) )
	{
		if ( ( ch == 10 ) || ( console_textlen == 254 ) )
		{
			// For commands longer than we can accept, consume the remainder of the input buffer
			if ( ( console_textlen == 254 ) && ( ch != 10 ) )
			{
				while ( read( STDIN_FILENO, &ch, 1 ) )
				{
					if ( ch == 10 )
						break;
				}
			}

			//Null terminate string and return
			console_text[ console_textlen ] = 0;
			console_textlen = 0;
			return console_text;
		}

		console_text[ console_textlen++ ] = ch;
	}

	return NULL;
}
#endif

#ifdef _WIN32
/*
==============
WriteStatusText

==============
*/
void WriteStatusText( char *szText )
{
	char szFullLine[81];
	COORD coord;
	DWORD dwWritten = 0;
	WORD wAttrib[80];
	
	int i;
	
	for ( i = 0; i < 80; i++ )
	{
		wAttrib[i] = FOREGROUND_RED | FOREGROUND_INTENSITY;
	}

	memset( szFullLine, 0, sizeof(szFullLine) );
	snprintf( szFullLine, sizeof( szFullLine ), "%s", szText );

	coord.X = 0;
	coord.Y = 0;

	WriteConsoleOutputAttribute( houtput, wAttrib, 80, coord, &dwWritten );
	WriteConsoleOutputCharacter( houtput, szFullLine, 80, coord, &dwWritten );	
}
#endif

/*
==============
CreateConsoleWindow

Create console window ( overridable? )
==============
*/
int CreateConsoleWindow( void )
{
#ifdef _WIN32
	if ( !AllocConsole () )
	{
		return 0;
	}

	hinput	= GetStdHandle (STD_INPUT_HANDLE);
	houtput = GetStdHandle (STD_OUTPUT_HANDLE);
	
	InitConProc();
#endif

	return 1;
}

/*
==============
DestroyConsoleWindow

==============
*/
void DestroyConsoleWindow( void )
{
#ifdef _WIN32
	FreeConsole ();

	// shut down QHOST hooks if necessary
	DeinitConProc ();
#endif
}

/*
==============
ProcessConsoleInput

==============
*/
void ProcessConsoleInput( void )
{
	char *s;

	if ( !engineapi.Cbuf_AddText )
		return;

	do
	{
		s = Sys_ConsoleInput ();
		if (s)
		{
			char szBuf[ 256 ];
			snprintf( szBuf, sizeof(szBuf), "%s\n", s );
			engineapi.Cbuf_AddText ( szBuf );
		}
	} while (s);
}

/*
================
GameInit
================
*/
int GameInit(void)
{
	char *p;

	// Command line override
	if ( (CheckParm ("-heapsize", &p ) ) && p )
	{
		giMemSize = atoi( p ) * 1024;
	}

	Sys_Sleep=Sys_Sleep_Old;

#if !defined ( _WIN32 )
	char *pPingType;
	int type;
	if ( (CheckParm ("-pingboost", &pPingType)) && pPingType )
	{
		type=atoi( pPingType );
		switch( type ) {
			case 1:
				//printf("Using timer method\n");
				signal(SIGALRM,alarmFunc);
				Sys_Sleep=Sys_Sleep_Timer;
				break;
			case 2:
				Sys_Sleep = Sys_Sleep_Select;
				break;
			case 3:
				Sys_Sleep = Sys_Sleep_Net;
				// we Sys_GetProcAddress NET_Sleep() from 
				//engine_i386.so later in this function
				break;
			default: // just in case
				Sys_Sleep=Sys_Sleep_Old;
				break;
		}

	}
#endif
	

	// Try and allocated it
#ifdef _WIN32
	gpMemBase = (unsigned char *)::GlobalAlloc( GMEM_FIXED, giMemSize );
#else
	gpMemBase = (unsigned char *)malloc( giMemSize );
#endif
	if (!gpMemBase)
	{
		return 0;
	}

#ifdef _WIN32
	// Check that we are running on Win32
	OSVERSIONINFO	vinfo;
	vinfo.dwOSVersionInfoSize = sizeof(vinfo);

	if ( !GetVersionEx ( &vinfo ) )
	{
		return 0;
	}

	if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32s )
	{
		return 0;
	}
	
#endif

	if ( !Eng_Load( gpszCmdLine, &ef, giMemSize, gpMemBase, g_pszengine, DLL_NORMAL ) )
	{
		return 0;
	}

#if !defined ( _WIN32 )
	if ( type == 3 ) 
	{
		NET_Sleep=(NET_Sleep_t)Sys_GetProcAddress(ghMod,"NET_Sleep_Timeout");
		//printf("Net_Sleep:%p\n",NET_Sleep);
	}
#endif

	Eng_SetState( DLL_ACTIVE );

	return 1;
}

/*
==============
GameShutdown

==============
*/
void GameShutdown( void )
{
	Eng_Unload();

	if ( gpMemBase )
	{
#ifdef _WIN32
		::GlobalFree( gpMemBase );
#else
		free( gpMemBase );
#endif
		gpMemBase = NULL;
	}
}

#ifdef _WIN32

BOOL gbAppHasBeenTerminated = FALSE;
BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch ( fdwCtrlType )
	{
		case CTRL_C_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			gbAppHasBeenTerminated = TRUE;
			return TRUE;

		default:
			return FALSE;
	}
}

/*
==============
WinMain

EXE entry point
==============
*/
int PASCAL WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow )
{
	int		iret = -1;

	// Store off command line for argument searching
	gpszCmdLine = strdup( GetCommandLine() );

	Sys_Sleep=Sys_Sleep_Old; // win32 doesn't have pingbooster options :)

	if ( !InitInstance() )
	{
		goto cleanup;
	}

	if ( !CreateConsoleWindow() )
	{
		goto cleanup;
	}

	if ( !GameInit() )
	{
		goto cleanup;
	}

	if ( engineapi.SetStartupMode )
	{
		engineapi.SetStartupMode( 1 );
	}

	while ( 1 )
	{
		int bDone = 0;

		static double oldtime = 0.0;

		MSG msg;
		double newtime;
		double dtime;

		if ( gbAppHasBeenTerminated )
			break;

		// Try to allow other apps to get some CPU
		Sys_Sleep( 1 );

		if ( !engineapi.Sys_FloatTime )
			break;

		while ( 1 )
		{
			newtime = engineapi.Sys_FloatTime();
			if ( newtime < oldtime )
			{
				oldtime = newtime - 0.05;
			}
			
			dtime = newtime - oldtime;

			if ( gbAppHasBeenTerminated )
				break;

			if ( dtime > 0.001 )
				break;

			// Running really fast, yield some time to other apps
			Sys_Sleep( 1 );
		}
		

		while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if ( gbAppHasBeenTerminated )
				break;

			if (!::GetMessage( &msg, NULL, 0, 0))
			{
				bDone = 1;
				break;
			}

			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}

		if ( bDone )
			break;

		ProcessConsoleInput();

		if ( engineapi.Host_Frame )
		{
			Eng_Frame( 0, dtime );
		}

		UpdateStatus( 0  /* don't force */ );

		oldtime = newtime;
	}

	GameShutdown();

	DestroyConsoleWindow();

	iret = 1;

cleanup:

	if ( gpszCmdLine )
	{
		free( gpszCmdLine );
	}

	return iret;
}

#else

#define MAX_LINUX_CMDLINE 512 

static char cmdline[ MAX_LINUX_CMDLINE ];

void BuildCmdLine( int argc, char **argv )
{
	int len;
	int i;

	for (len = 0, i = 1; i < argc; i++)
	{
		len += strlen(argv[i]) + 1;
	}

	if ( len > MAX_LINUX_CMDLINE )
	{
		printf( "command line too long, %i max\n", MAX_LINUX_CMDLINE );
		exit(-1);
		return;
	}

	cmdline[0] = '\0';
	for ( i = 1; i < argc; i++ )
	{
		if ( i > 1 )
		{
			strcat( cmdline, " " );
		}
		strcat( cmdline, argv[ i ] );
	}
}

char *GetCommandLine( void )
{
	return cmdline;
}

int main(int argc, char **argv)
{
	int		iret = -1;

#ifdef _DEBUG
	snprintf(g_szEXEName, sizeof( g_szEXEName ), "hlds_run.dbg" );
#else
	snprintf(g_szEXEName, sizeof( g_szEXEName ), "%s", *argv );
#endif

	// Store off command line for argument searching
	BuildCmdLine(argc, argv);
	gpszCmdLine = strdup( GetCommandLine() );

	if ( !InitInstance() )
	{
		goto cleanup;
	}

	if ( !CreateConsoleWindow() )
	{
		goto cleanup;
	}

	if ( !GameInit() )
	{
		goto cleanup;
	}

	if ( engineapi.SetStartupMode )
	{
		engineapi.SetStartupMode( 1 );
	}

   while ( 1 )
   {
		char *p;
		static double oldtime = 0.0;

		double newtime;
		double dtime;

#ifdef FRAMERATE
		static int frameNumber=0,frameRate=0;
		static struct timeval beforeTime;		
		if(frameRate==0) {
			gettimeofday(&beforeTime,NULL);
		}

		frameRate++;

		if(frameRate%1000==0) {
			struct timeval afterTime;
			float timediff;
			
			gettimeofday(&afterTime,NULL);
			timediff= (float)(afterTime.tv_sec-beforeTime.tv_sec)+
        		      ((float)(afterTime.tv_usec-beforeTime.tv_usec))/1E6;

			printf("Frame Rate:%f\n",(float)frameRate/timediff);
			frameRate=0;

		}
#endif

		Sys_Sleep( 1 );

		if ( !engineapi.Sys_FloatTime )
			break;

		while ( 1 )
		{
			newtime = engineapi.Sys_FloatTime();
			if ( newtime < oldtime )
			{
				oldtime = newtime - 0.05;
			}
			
			dtime = newtime - oldtime;

			if ( dtime > 0.001 )
				break;

			// Running really fast, yield some time to other apps
			Sys_Sleep( 1 );
		}
		
		ProcessConsoleInput();

		Eng_Frame( 0, dtime );

		oldtime = newtime;
	}
	
	GameShutdown();

	DestroyConsoleWindow();

	iret = 1;

cleanup:

	if ( gpszCmdLine )
	{
		free( gpszCmdLine );
	}

	if ( hDLLThirdParty )
	{
		Sys_FreeLibrary( hDLLThirdParty );
		hDLLThirdParty = 0L;
	}

	return iret;
}

#endif
