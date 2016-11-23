//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( GAMESTUDIOMODELRENDERER_H )
#define GAMESTUDIOMODELRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

/*
====================
CGameStudioModelRenderer

====================
*/
class CGameStudioModelRenderer : public CStudioModelRenderer
{
public:
	CGameStudioModelRenderer( void );

	// Set up model bone positions
	virtual void StudioSetupBones ( void );	

	// Estimate gait frame for player
	virtual void StudioEstimateGait ( entity_state_t *pplayer );

	// Process movement of player
	virtual void StudioProcessGait ( entity_state_t *pplayer );

	// Player drawing code
	virtual int StudioDrawPlayer( int flags, entity_state_t *pplayer );
	virtual int _StudioDrawPlayer( int flags, entity_state_t *pplayer );

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

private:
	// For local player, in third person, we need to store real render data and then
	//  setup for with fake/client side animation data
	void SavePlayerState( entity_state_t *pplayer );
	// Called to set up local player's animation values
	void SetupClientAnimation( entity_state_t *pplayer );
	// Called to restore original player state information
	void RestorePlayerState( entity_state_t *pplayer );

private: 
	// Private data
	bool m_bLocal;
};

#endif // GAMESTUDIOMODELRENDERER_H