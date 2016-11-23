//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( R_STUDIOINT_H )
#define R_STUDIOINT_H
#if defined( _WIN32 )
#pragma once
#endif

#define STUDIO_INTERFACE_VERSION 1

typedef struct engine_studio_api_s
{
	// Allocate number*size bytes and zero it
	void			*( *Mem_Calloc )				( int number, size_t size );
	// Check to see if pointer is in the cache
	void			*( *Cache_Check )				( struct cache_user_s *c );
	// Load file into cache ( can be swapped out on demand )
	void			( *LoadCacheFile )				( char *path, struct cache_user_s *cu );
	// Retrieve model pointer for the named model
	struct model_s	*( *Mod_ForName )				( const char *name, int crash_if_missing );
	// Retrieve pointer to studio model data block from a model
	void			*( *Mod_Extradata )				( struct model_s *mod );
	// Retrieve indexed model from client side model precache list
	struct model_s	*( *GetModelByIndex )			( int index );
	// Get entity that is set for rendering
	struct cl_entity_s * ( *GetCurrentEntity )		( void );
	// Get referenced player_info_t
	struct player_info_s *( *PlayerInfo )			( int index );
	// Get most recently received player state data from network system
	struct entity_state_s *( *GetPlayerState )		( int index );
	// Get viewentity
	struct cl_entity_s * ( *GetViewEntity )			( void );
	// Get current frame count, and last two timestampes on client
	void			( *GetTimes )					( int *framecount, double *current, double *old );
	// Get a pointer to a cvar by name
	struct cvar_s	*( *GetCvar )					( const char *name );
	// Get current render origin and view vectors ( up, right and vpn )
	void			( *GetViewInfo )				( float *origin, float *upv, float *rightv, float *vpnv );
	// Get sprite model used for applying chrome effect
	struct model_s	*( *GetChromeSprite )			( void );
	// Get model counters so we can incement instrumentation
	void			( *GetModelCounters )			( int **s, int **a );
	// Get software scaling coefficients
	void			( *GetAliasScale )				( float *x, float *y );

	// Get bone, light, alias, and rotation matrices
	float			****( *StudioGetBoneTransform ) ( void );
	float			****( *StudioGetLightTransform )( void );
	float			***( *StudioGetAliasTransform ) ( void );
	float			***( *StudioGetRotationMatrix ) ( void );

	// Set up body part, and get submodel pointers
	void			( *StudioSetupModel )			( int bodypart, void **ppbodypart, void **ppsubmodel );
	// Check if entity's bbox is in the view frustum
	int				( *StudioCheckBBox )			( void );
	// Apply lighting effects to model
	void			( *StudioDynamicLight )			( struct cl_entity_s *ent, struct alight_s *plight );
	void			( *StudioEntityLight )			( struct alight_s *plight );
	void			( *StudioSetupLighting )		( struct alight_s *plighting );

	// Draw mesh vertices
	void			( *StudioDrawPoints )			( void );

	// Draw hulls around bones
	void			( *StudioDrawHulls )			( void );
	// Draw bbox around studio models
	void			( *StudioDrawAbsBBox )			( void );
	// Draws bones
	void			( *StudioDrawBones )			( void );
	// Loads in appropriate texture for model
	void			( *StudioSetupSkin )			( void *ptexturehdr, int index );
	// Sets up for remapped colors
	void			( *StudioSetRemapColors )		( int top, int bottom );
	// Set's player model and returns model pointer
	struct model_s	*( *SetupPlayerModel )			( int index );
	// Fires any events embedded in animation
	void			( *StudioClientEvents )			( void );
	// Retrieve/set forced render effects flags
	int				( *GetForceFaceFlags )			( void );
	void			( *SetForceFaceFlags )			( int flags );
	// Tell engine the value of the studio model header
	void			( *StudioSetHeader )			( void *header );
	// Tell engine which model_t * is being renderered
	void			( *SetRenderModel )				( struct model_s *model );

	// Final state setup and restore for rendering
	void			( *SetupRenderer )				( int rendermode );
	void			( *RestoreRenderer )			( void );

	// Set render origin for applying chrome effect
	void			( *SetChromeOrigin )			( void );

	// True if using D3D/OpenGL
	int				( *IsHardware )					( void );
	
	// Only called by hardware interface
	void			( *GL_StudioDrawShadow )		( void );
	void			( *GL_SetRenderMode )			( int mode );
} engine_studio_api_t;

typedef struct server_studio_api_s
{
	// Allocate number*size bytes and zero it
	void			*( *Mem_Calloc )				( int number, size_t size );
	// Check to see if pointer is in the cache
	void			*( *Cache_Check )				( struct cache_user_s *c );
	// Load file into cache ( can be swapped out on demand )
	void			( *LoadCacheFile )				( char *path, struct cache_user_s *cu );
	// Retrieve pointer to studio model data block from a model
	void			*( *Mod_Extradata )				( struct model_s *mod );
} server_studio_api_t;


// client blending
typedef struct r_studio_interface_s
{
	int				version;
	int				( *StudioDrawModel	)			( int flags );
	int				( *StudioDrawPlayer	)			( int flags, struct entity_state_s *pplayer );
} r_studio_interface_t;

extern r_studio_interface_t *pStudioAPI;

// server blending
#define SV_BLENDING_INTERFACE_VERSION 1

typedef struct sv_blending_interface_s
{
	int	version;

	void	( *SV_StudioSetupBones )( struct model_s *pModel, 
					float frame,
					int sequence,
					const vec3_t angles,
					const vec3_t origin,
					const byte *pcontroller,
					const byte *pblending,
					int iBone,
					const edict_t *pEdict );
} sv_blending_interface_t;

#endif // R_STUDIOINT_H