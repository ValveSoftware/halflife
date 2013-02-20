//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( STUDIOMODELRENDERER_H )
#define STUDIOMODELRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

/*
====================
CStudioModelRenderer

====================
*/
class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer( void );
	virtual ~CStudioModelRenderer( void );

	// Initialization
	virtual void Init( void );

public:  
	// Public Interfaces
	virtual int StudioDrawModel ( int flags );
	virtual int StudioDrawPlayer ( int flags, struct entity_state_s *pplayer );

public:
	// Local interfaces
	//

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim ( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc );

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform (int trivial_accept);

	// Set up model bone positions
	virtual void StudioSetupBones ( void );	

	// Find final attachment points
	virtual void StudioCalcAttachments ( void );
	
	// Save bone matrices and names
	virtual void StudioSaveBones( void );

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones ( model_t *m_pSubModel );

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant( void );

	// Determine current frame for rendering
	virtual float StudioEstimateFrame ( mstudioseqdesc_t *pseqdesc );

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

	// Spherical interpolation of bones
	virtual void StudioSlerpBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s );

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj ( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen );

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q );

	// Get bone positions
	virtual void StudioCalcBonePosition ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos );

	// Compute rotations
	virtual void StudioCalcRotations ( float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	// Send bones and verts to renderer
	virtual void StudioRenderModel ( void );

	// Finalize rendering
	virtual void StudioRenderFinal (void);
	
	// GL&D3D vs. Software renderer finishing functions
	virtual void StudioRenderFinal_Software ( void );
	virtual void StudioRenderFinal_Hardware ( void );

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend ( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );

	// Estimate gait frame for player
	virtual void StudioEstimateGait ( entity_state_t *pplayer );

	// Process movement of player
	virtual void StudioProcessGait ( entity_state_t *pplayer );

public:

	// Client clock
	double			m_clTime;				
	// Old Client clock
	double			m_clOldTime;			

	// Do interpolation?
	int				m_fDoInterp;			
	// Do gait estimation?
	int				m_fGaitEstimation;		

	// Current render frame #
	int				m_nFrameCount;

	// Cvars that studio model code needs to reference
	//
	// Use high quality models?
	cvar_t			*m_pCvarHiModels;	
	// Developer debug output desired?
	cvar_t			*m_pCvarDeveloper;
	// Draw entities bone hit boxes, etc?
	cvar_t			*m_pCvarDrawEntities;

	// The entity which we are currently rendering.
	cl_entity_t		*m_pCurrentEntity;		

	// The model for the entity being rendered
	model_t			*m_pRenderModel;

	// Player info for current player, if drawing a player
	player_info_t	*m_pPlayerInfo;

	// The index of the player being drawn
	int				m_nPlayerIndex;

	// The player's gait movement
	float			m_flGaitMovement;

	// Pointer to header block for studio model data
	studiohdr_t		*m_pStudioHeader;
	
	// Pointers to current body part and submodel
	mstudiobodyparts_t *m_pBodyPart;
	mstudiomodel_t	*m_pSubModel;

	// Palette substition for top and bottom of model
	int				m_nTopColor;			
	int				m_nBottomColor;

	//
	// Sprite model used for drawing studio model chrome
	model_t			*m_pChromeSprite;

	// Caching
	// Number of bones in bone cache
	int				m_nCachedBones; 
	// Names of cached bones
	char			m_nCachedBoneNames[ MAXSTUDIOBONES ][ 32 ];
	// Cached bone & light transformation matrices
	float			m_rgCachedBoneTransform [ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			m_rgCachedLightTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];

	// Software renderer scale factors
	float			m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	float			m_vUp[ 3 ];
	float			m_vRight[ 3 ];
	float			m_vNormal[ 3 ];

	float			m_vRenderOrigin[ 3 ];
	
	// Model render counters ( from engine )
	int				*m_pStudioModelCount;
	int				*m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float			(*m_protationmatrix)[ 3 ][ 4 ];	
	// Model to view transformation
	float			(*m_paliastransform)[ 3 ][ 4 ];	

	// Concatenated bone and light transforms
	float			(*m_pbonetransform) [ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			(*m_plighttransform)[ MAXSTUDIOBONES ][ 3 ][ 4 ];
};

#endif // STUDIOMODELRENDERER_H