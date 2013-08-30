// CL_DLLEXPORT is the client version of dllexport.  It's turned off for secure clients.
#ifdef _WIN32
#define CL_DLLEXPORT __declspec(dllexport)
#else
#define CL_DLLEXPORT __attribute__ ((visibility("default")))
#endif

extern "C" 
{
	// From hl_weapons
	void CL_DLLEXPORT HUD_PostRunCmd( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );

	// From cdll_int
	int CL_DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
	int CL_DLLEXPORT HUD_VidInit( void );
	void CL_DLLEXPORT HUD_Init( void );
	int CL_DLLEXPORT HUD_Redraw( float flTime, int intermission );
	int CL_DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
	void CL_DLLEXPORT HUD_Reset ( void );
	void CL_DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
	void CL_DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
	char CL_DLLEXPORT HUD_PlayerMoveTexture( char *name );
	int CL_DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
	int CL_DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
	void CL_DLLEXPORT HUD_Frame( double time );
	void CL_DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
	void CL_DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
	void CL_DLLEXPORT HUD_ChatInputPosition( int *x, int *y );
	int CL_DLLEXPORT HUD_GetPlayerTeam(int iplayer);

	// From demo
	void CL_DLLEXPORT Demo_ReadBuffer( int size, unsigned char *buffer );

	// From entity
	int CL_DLLEXPORT HUD_AddEntity( int type, struct cl_entity_s *ent, const char *modelname );
	void CL_DLLEXPORT HUD_CreateEntities( void );
	void CL_DLLEXPORT HUD_StudioEvent( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
	void CL_DLLEXPORT HUD_TxferLocalOverrides( struct entity_state_s *state, const struct clientdata_s *client );
	void CL_DLLEXPORT HUD_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
	void CL_DLLEXPORT HUD_TxferPredictionData ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
	void CL_DLLEXPORT HUD_TempEntUpdate( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ), void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
	struct cl_entity_s CL_DLLEXPORT *HUD_GetUserEntity( int index );

	// From in_camera
	void CL_DLLEXPORT CAM_Think( void );
	int CL_DLLEXPORT CL_IsThirdPerson( void );
	void CL_DLLEXPORT CL_CameraOffset( float *ofs );

	// From input
	struct kbutton_s CL_DLLEXPORT *KB_Find( const char *name );
	void CL_DLLEXPORT CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active );
	void CL_DLLEXPORT HUD_Shutdown( void );
	int CL_DLLEXPORT HUD_Key_Event( int eventcode, int keynum, const char *pszCurrentBinding );

	// From inputw32
	void CL_DLLEXPORT IN_ActivateMouse( void );
	void CL_DLLEXPORT IN_DeactivateMouse( void );
	void CL_DLLEXPORT IN_MouseEvent (int mstate);
	void CL_DLLEXPORT IN_Accumulate (void);
	void CL_DLLEXPORT IN_ClearStates (void);

	// From tri
	void CL_DLLEXPORT HUD_DrawNormalTriangles( void );
	void CL_DLLEXPORT HUD_DrawTransparentTriangles( void );

	// From view
	void	CL_DLLEXPORT V_CalcRefdef( struct ref_params_s *pparams );

	// From GameStudioModelRenderer
	int CL_DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
}

extern modfuncs_t g_modfuncs;
//extern cldll_func_dst_t *g_pcldstAddrs;

/*
// Macros for the client receiving calls from the engine
#define RecClInitialize(a, b)			(g_pcldstAddrs->pInitFunc(&a, &b))
#define RecClHudInit()					(g_pcldstAddrs->pHudInitFunc())
#define RecClHudVidInit()				(g_pcldstAddrs->pHudVidInitFunc())
#define RecClHudRedraw(a, b)			(g_pcldstAddrs->pHudRedrawFunc(&a, &b))
#define RecClHudUpdateClientData(a, b)	(g_pcldstAddrs->pHudUpdateClientDataFunc(&a, &b))
#define RecClHudReset()					(g_pcldstAddrs->pHudResetFunc())
#define RecClClientMove(a, b)			(g_pcldstAddrs->pClientMove(&a, &b))
#define RecClClientMoveInit(a)			(g_pcldstAddrs->pClientMoveInit(&a))
#define RecClClientTextureType(a)		(g_pcldstAddrs->pClientTextureType(&a))
#define RecClIN_ActivateMouse()			(g_pcldstAddrs->pIN_ActivateMouse())
#define RecClIN_DeactivateMouse()		(g_pcldstAddrs->pIN_DeactivateMouse())
#define RecClIN_MouseEvent(a)			(g_pcldstAddrs->pIN_MouseEvent(&a))
#define RecClIN_ClearStates()			(g_pcldstAddrs->pIN_ClearStates())
#define RecClIN_Accumulate()			(g_pcldstAddrs->pIN_Accumulate())
#define RecClCL_CreateMove(a, b, c)		(g_pcldstAddrs->pCL_CreateMove(&a, &b, &c))
#define RecClCL_IsThirdPerson()			(g_pcldstAddrs->pCL_IsThirdPerson())
#define RecClCL_GetCameraOffsets(a)		(g_pcldstAddrs->pCL_GetCameraOffsets(&a))
#define RecClFindKey(a)					(g_pcldstAddrs->pFindKey(&a))
#define RecClCamThink()					(g_pcldstAddrs->pCamThink())
#define RecClCalcRefdef(a)				(g_pcldstAddrs->pCalcRefdef(&a))
#define RecClAddEntity(a, b, c)			(g_pcldstAddrs->pAddEntity(&a, &b, &c))
#define RecClCreateEntities()			(g_pcldstAddrs->pCreateEntities())
#define RecClDrawNormalTriangles()		(g_pcldstAddrs->pDrawNormalTriangles())
#define RecClDrawTransparentTriangles()	(g_pcldstAddrs->pDrawTransparentTriangles())
#define RecClStudioEvent(a, b)			(g_pcldstAddrs->pStudioEvent(&a, &b))
#define RecClPostRunCmd(a, b, c, d, e, f)		(g_pcldstAddrs->pPostRunCmd(&a, &b, &c, &d, &e, &f))
#define RecClShutdown()					(g_pcldstAddrs->pShutdown())
#define RecClTxferLocalOverrides(a, b)	(g_pcldstAddrs->pTxferLocalOverrides(&a, &b))
#define RecClProcessPlayerState(a, b)	(g_pcldstAddrs->pProcessPlayerState(&a, &b))
#define RecClTxferPredictionData(a, b, c, d, e, f)		(g_pcldstAddrs->pTxferPredictionData(&a, &b, &c, &d, &e, &f))
#define RecClReadDemoBuffer(a, b)		(g_pcldstAddrs->pReadDemoBuffer(&a, &b))
#define RecClConnectionlessPacket(a, b, c, d)		(g_pcldstAddrs->pConnectionlessPacket(&a, &b, &c, &d))
#define RecClGetHullBounds(a, b, c)		(g_pcldstAddrs->pGetHullBounds(&a, &b, &c))
#define RecClHudFrame(a)				(g_pcldstAddrs->pHudFrame(&a))
#define RecClKeyEvent(a, b, c)			(g_pcldstAddrs->pKeyEvent(&a, &b, &c))
#define RecClTempEntUpdate(a, b, c, d, e, f, g)	(g_pcldstAddrs->pTempEntUpdate(&a, &b, &c, &d, &e, &f, &g))
#define RecClGetUserEntity(a)			(g_pcldstAddrs->pGetUserEntity(&a))
#define RecClVoiceStatus(a, b)			(g_pcldstAddrs->pVoiceStatus(&a, &b))
#define RecClDirectorMessage(a, b)		(g_pcldstAddrs->pDirectorMessage(&a, &b))
#define RecClStudioInterface(a, b, c)	(g_pcldstAddrs->pStudioInterface(&a, &b, &c))
#define RecClChatInputPosition(a, b)	(g_pcldstAddrs->pChatInputPosition(&a, &b))

*/
// Macros for calling into the engine
#define CallEngSPR_Load(a)						(gEngfuncs.pfnSPR_Load(a))
#define CallEngSPR_Frames(a)					(gEngfuncs.pfnSPR_Frames(a))
#define CallEngSPR_Height(a, b)					(gEngfuncs.pfnSPR_Height(a, b))
#define CallEngSPR_Width(a, b)					(gEngfuncs.pfnSPR_Width(a, b))
#define CallEngSPR_Set(a, b, c, d)				(gEngfuncs.pfnSPR_Set(a, b, c, d))
#define CallEngSPR_Draw(a, b, c, d)				(gEngfuncs.pfnSPR_Draw(a, b, c, d))
#define CallEngSPR_DrawHoles(a, b, c, d)		(gEngfuncs.pfnSPR_DrawHoles(a, b, c, d))
#define CallEngSPR_DrawAdditive(a, b, c, d)		(gEngfuncs.pfnSPR_DrawAdditive(a, b, c, d))
#define CallEngSPR_EnableScissor(a, b, c, d)	(gEngfuncs.pfnSPR_EnableScissor(a, b, c, d))
#define CallEngSPR_DisableScissor()				(gEngfuncs.pfnSPR_DisableScissor())
#define CallEngSPR_GetList(a, b)				(gEngfuncs.pfnSPR_GetList(a, b))
#define CallEngDraw_FillRGBA(a, b, c, d, e, f, g, h)		(gEngfuncs.pfnFillRGBA(a, b, c, d, e, f, g, h))
#define CallEngDraw_FillRGBABlend(a, b, c, d, e, f, g, h)		(gEngfuncs.pfnFillRGBABlend(a, b, c, d, e, f, g, h))
#define CallEnghudGetScreenInfo(a)				(gEngfuncs.pfnGetScreenInfo(a))
#define CallEngSetCrosshair(a, b, c, d, e)		(gEngfuncs.pfnSetCrosshair(a, b, c, d, e))
#define CallEnghudRegisterVariable(a, b, c)		(gEngfuncs.pfnRegisterVariable(a, b, c))
#define CallEnghudGetCvarFloat(a)				(gEngfuncs.pfnGetCvarFloat(a))
#define CallEnghudGetCvarString(a)				(gEngfuncs.pfnGetCvarString(a))
#define CallEnghudAddCommand(a, b)				(gEngfuncs.pfnAddCommand(a, b))
#define CallEnghudHookUserMsg(a, b)				(gEngfuncs.pfnHookUserMsg(a, b))
#define CallEnghudServerCmd(a)					(gEngfuncs.pfnServerCmd(a))
#define CallEnghudClientCmd(a)					(gEngfuncs.pfnClientCmd(a))
#define CallEngPrimeMusicStream(a, b)		(gEngfuncs.pfnPrimeMusicStream(a, b))
#define CallEnghudGetPlayerInfo(a, b)			(gEngfuncs.pfnGetPlayerInfo(a, b))
#define CallEnghudPlaySoundByName(a, b)			(gEngfuncs.pfnPlaySoundByName(a, b))
#define CallEnghudPlaySoundByNameAtPitch(a, b, c)	(gEngfuncs.pfnPlaySoundByNameAtPitch(a, b, c))
#define CallEnghudPlaySoundVoiceByName(a, b, c)	(gEngfuncs.pfnPlaySoundVoiceByName(a, b, c))
#define CallEnghudPlaySoundByIndex(a, b)		(gEngfuncs.pfnPlaySoundByIndex(a, b))
#define CallEngAngleVectors(a, b, c, d)			(gEngfuncs.pfnAngleVectors(a, b, c, d))
#define CallEngTextMessageGet(a)				(gEngfuncs.pfnTextMessageGet(a))
#define CallEngTextMessageDrawCharacter(a, b, c, d, e, f)	(gEngfuncs.pfnDrawCharacter(a, b, c, d, e, f))
#define CallEngDraw_String(a, b, c)				(gEngfuncs.pfnDrawConsoleString(a, b, c))
#define CallEngDraw_SetTextColor(a, b, c)		(gEngfuncs.pfnDrawSetTextColor(a, b, c))
#define CallEnghudDrawConsoleStringLen(a, b, c)	(gEngfuncs.pfnDrawConsoleStringLen(a, b, c))
#define CallEnghudConsolePrint(a)				(gEngfuncs.pfnConsolePrint(a))
#define CallEnghudCenterPrint(a)				(gEngfuncs.pfnCenterPrint(a))
#define CallEnghudCenterX()						(gEngfuncs.GetWindowCenterX())
#define CallEnghudCenterY()						(gEngfuncs.GetWindowCenterY())
#define CallEnghudGetViewAngles(a)				(gEngfuncs.GetViewAngles(a))
#define CallEnghudSetViewAngles(a)				(gEngfuncs.SetViewAngles(a))
#define CallEnghudGetMaxClients()				(gEngfuncs.GetMaxClients())
#define CallEngCvar_SetValue(a, b)				(gEngfuncs.Cvar_SetValue(a, b))
#define CallEngCmd_Argc()						(gEngfuncs.Cmd_Argc())
#define CallEngCmd_Argv(a)						(gEngfuncs.Cmd_Argv(a))
#define CallEnghudPhysInfo_ValueForKey(a)		(gEngfuncs.PhysInfo_ValueForKey(a))
#define CallEnghudServerInfo_ValueForKey(a)		(gEngfuncs.ServerInfo_ValueForKey(a))
#define CallEnghudGetClientMaxspeed()			(gEngfuncs.GetClientMaxspeed())
#define CallEnghudCheckParm(a, b)				(gEngfuncs.CheckParm(a, b))
#define CallEngKey_Event(a, b)					(gEngfuncs.Key_Event(a, b))
#define CallEnghudGetMousePosition(a, b)		(gEngfuncs.GetMousePosition(a, b))
#define CallEnghudIsNoClipping()				(gEngfuncs.IsNoClipping())
#define CallEnghudGetLocalPlayer()				(gEngfuncs.GetLocalPlayer())
#define CallEnghudGetViewModel()				(gEngfuncs.GetViewModel())
#define CallEnghudGetEntityByIndex(a)			(gEngfuncs.GetEntityByIndex(a))
#define CallEnghudGetClientTime()				(gEngfuncs.GetClientTime())
#define CallEngV_CalcShake()					(gEngfuncs.V_CalcShake())
#define CallEngV_ApplyShake(a, b, c)			(gEngfuncs.V_ApplyShake(a, b, c))
#define CallEngPM_PointContents(a, b)			(gEngfuncs.PM_PointContents(a, b))
#define CallEngPM_WaterEntity(a)				(gEngfuncs.PM_WaterEntity(a))
#define CallEngPM_TraceLine(a, b, c, d, e)		(gEngfuncs.PM_TraceLine(a, b, c, d, e))
#define CallEngCL_LoadModel(a, b)				(gEngfuncs.CL_LoadModel(a, b))
#define CallEngCL_CreateVisibleEntity(a, b)		(gEngfuncs.CL_CreateVisibleEntity(a, b))
#define CallEnghudGetSpritePointer(a)			(gEngfuncs.GetSpritePointer(a))
#define CallEnghudPlaySoundByNameAtLocation(a, b, c)		(gEngfuncs.pfnPlaySoundByNameAtLocation(a, b, c))
#define CallEnghudPrecacheEvent(a, b)			(gEngfuncs.pfnPrecacheEvent(a, b))
#define CallEnghudPlaybackEvent(a, b, c, d, e, f, g, h, i, j, k, l)	(gEngfuncs.pfnPlaybackEvent(a, b, c, d, e, f, g, h, i, j, k, l))
#define CallEnghudWeaponAnim(a, b)				(gEngfuncs.pfnWeaponAnim(a, b))
#define CallEngRandomFloat(a, b)				(gEngfuncs.pfnRandomFloat(a, b))
#define CallEngRandomLong(a, b)					(gEngfuncs.pfnRandomLong(a, b))
#define CallEngCL_HookEvent(a, b)				(gEngfuncs.pfnHookEvent(a, b))
#define CallEngCon_IsVisible()					(gEngfuncs.Con_IsVisible())
#define CallEnghudGetGameDir()					(gEngfuncs.pfnGetGameDirectory())
#define CallEngCvar_FindVar(a)					(gEngfuncs.pfnGetCvarPointer(a))
#define CallEngKey_NameForBinding(a)			(gEngfuncs.Key_LookupBinding(a))
#define CallEnghudGetLevelName()				(gEngfuncs.pfnGetLevelName())
#define CallEnghudGetScreenFade(a)				(gEngfuncs.pfnGetScreenFade(a))
#define CallEnghudSetScreenFade(a)				(gEngfuncs.pfnSetScreenFade(a))
#define CallEngVGuiWrap_GetPanel()				(gEngfuncs.VGui_GetPanel())
#define CallEngVGui_ViewportPaintBackground(a)	(gEngfuncs.VGui_ViewportPaintBackground(a))
#define CallEngCOM_LoadFile(a, b, c)			(gEngfuncs.COM_LoadFile(a, b, c))
#define CallEngCOM_ParseFile(a, b)				(gEngfuncs.COM_ParseFile(a, b))
#define CallEngCOM_FreeFile(a)					(gEngfuncs.COM_FreeFile(a))
#define CallEngCL_IsSpectateOnly()				(gEngfuncs.IsSpectateOnly())
#define CallEngR_LoadMapSprite(a)				(gEngfuncs.LoadMapSprite(a))
#define CallEngCOM_AddAppDirectoryToSearchPath(a, b)		(gEngfuncs.COM_AddAppDirectoryToSearchPath(a, b))
#define CallEngClientDLL_ExpandFileName(a, b, c)(gEngfuncs.COM_ExpandFilename(a, b, c))
#define CallEngPlayerInfo_ValueForKey(a, b)		(gEngfuncs.PlayerInfo_ValueForKey(a, b))
#define CallEngPlayerInfo_SetValueForKey(a, b)	(gEngfuncs.PlayerInfo_SetValueForKey(a, b))
#define CallEngGetPlayerUniqueID(a, b)			(gEngfuncs.GetPlayerUniqueID(a, b))
#define CallEngGetTrackerIDForPlayer(a)			(gEngfuncs.GetTrackerIDForPlayer(a))
#define CallEngGetPlayerForTrackerID(a)			(gEngfuncs.GetPlayerForTrackerID(a))
#define CallEnghudServerCmdUnreliable(a)		(gEngfuncs.pfnServerCmdUnreliable(a))
#define CallEngGetMousePos(a)					(gEngfuncs.pfnGetMousePos(a))
#define CallEngSetMousePos(a, b)				(gEngfuncs.pfnSetMousePos(a, b))
#define CallEngSetMouseEnable(a)				(gEngfuncs.pfnSetMouseEnable(a))
#define CallEngLocalPlayerInfo_ValueForKey(a)		(gEngfuncs.LocalPlayerInfo_ValueForKey(a))

#if 0
inline float CVAR_GET_FLOAT( const char *x ) {	return CallEnghudGetCvarFloat( (char*)x ); }
inline char* CVAR_GET_STRING( const char *x ) {	return CallEnghudGetCvarString( (char*)x ); }
inline struct cvar_s *CVAR_CREATE( const char *cv, const char *val, const int flags ) {	return CallEnghudRegisterVariable( (char*)cv, (char*)val, flags ); }
#endif

