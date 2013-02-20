//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// engine/launcher interface
#if !defined( ENGINE_LAUNCHER_APIH )
#define ENGINE_LAUNCHER_APIH
#ifdef _WIN32
#pragma once
#endif

//typedef void ( *xcommand_t ) ( void );

#define RENDERTYPE_UNDEFINED	0
#define RENDERTYPE_SOFTWARE		1
#define RENDERTYPE_HARDWARE		2

#define ENGINE_LAUNCHER_API_VERSION 1

typedef struct engine_api_s
{
	int		version;
	int		rendertype;
	int		size;

	// Functions
	int		( *GetEngineState )				( void );
	void	( *Cbuf_AddText )				( char *text ); // append cmd at end of buf
	void	( *Cbuf_InsertText )			( char *text ); // insert cmd at start of buf
	void	( *Cmd_AddCommand )				( char *cmd_name, void ( *funcname )( void ) );
	int		( *Cmd_Argc )					( void );
	char	*( *Cmd_Args )					( void );
	char	*( *Cmd_Argv )					( int arg );
	void	( *Con_Printf )					( char *, ... );
	void	( *Con_SafePrintf )				( char *, ... );
	void	( *Cvar_Set )					( char *var_name, char *value );
	void	( *Cvar_SetValue )				( char *var_name, float value );
	int		( *Cvar_VariableInt )			( char *var_name );
	char	*( *Cvar_VariableString )		( char *var_name );
	float	( *Cvar_VariableValue )			( char *var_name );
	void	( *ForceReloadProfile )			( void );
	int		( *GetGameInfo )				( struct GameInfo_s *pGI, char *pszChannel );
	void	( *GameSetBackground )			( int bBack );
	void	( *GameSetState )				( int iState );
	void	( *GameSetSubState )			( int iState );
	int		( *GetPauseState )				( void );
	int		( *Host_Frame )					( float time, int iState, int *stateInfo );
	void	( *Host_GetHostInfo )			( float *fps, int *nActive, int *nSpectators, int *nMaxPlayers, char *pszMap );
	void	( *Host_Shutdown )				( void );
	int		( *Game_Init )					( char *lpCmdLine, unsigned char *pMem, int iSize, struct exefuncs_s *pef, void *, int );
	void	( *IN_ActivateMouse )			( void );
	void	( *IN_ClearStates )				( void );
	void	( *IN_DeactivateMouse )			( void );
	void	( *IN_MouseEvent )				( int mstate );
	void	( *Keyboard_ReturnToGame )		( void );
	void	( *Key_ClearStates )			( void );
	void	( *Key_Event )					( int key, int down );
	int		( *LoadGame )					( const char *pszSlot );
	void	( *S_BlockSound )				( void );
	void	( *S_ClearBuffer )				( void );
	void	( *S_GetDSPointer )				( struct IDirectSound **lpDS, struct IDirectSoundBuffer **lpDSBuf );
	void 	*( *S_GetWAVPointer )			( void );
	void	( *S_UnblockSound )				( void );
	int		( *SaveGame )					( const char *pszSlot, const char *pszComment );
	void	( *SetAuth )					( void *pobj );
	void	( *SetMessagePumpDisableMode )	( int bMode );
	void	( *SetPauseState )				( int bPause );
	void	( *SetStartupMode )				( int bMode );
	void	( *SNDDMA_Shutdown )			( void );
	void	( *Snd_AcquireBuffer )			( void );
	void	( *Snd_ReleaseBuffer )			( void );
	void	( *StoreProfile )				( void );
	double	( *Sys_FloatTime )				( void );
	void	( *VID_UpdateWindowVars )		( void *prc, int x, int y );
	void	( *VID_UpdateVID )				( struct viddef_s *pvid );

	// VGUI interfaces
	void	( *VGui_CallEngineSurfaceProc )	( void* hwnd, unsigned int msg, unsigned int wparam, long lparam );

	// notifications that the launcher is taking/giving focus to the engine
	void    ( *EngineTakingFocus )			( void );
	void    ( *LauncherTakingFocus )		( void );

#ifdef _WIN32
	// Only filled in by rendertype RENDERTYPE_HARDWARE
	void	( *GL_Init )					( void );
	int		( *GL_SetMode )					( HWND hwndGame, HDC *pmaindc, HGLRC *pbaseRC, int fD3D, const char *p, const char *pszCmdLine );
	void	( *GL_Shutdown )				( HWND hwnd, HDC hdc, HGLRC hglrc );

	void	( *QGL_D3DShared )				( struct tagD3DGlobals *d3dGShared );

	int		( WINAPI *glSwapBuffers )		( HDC dc );
	void	( *DirectorProc ) ( unsigned int cmd, void * params );
#else
	// NOT USED IN LINUX!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	void	( *GL_Init )					( void );
	void	( *GL_SetMode )					( void );
	void	( *GL_Shutdown )				( void );
	void	( *QGL_D3DShared )				( void );
	void	( *glSwapBuffers )				( void );
	void	( *DirectorProc )				( void );
	// LINUX
#endif

} engine_api_t;

#endif // ENGINE_LAUNCHER_APIH
