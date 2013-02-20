//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"

// Predicted values saved off in hl_weapons.cpp
void Game_GetSequence( int *seq, int *gaitseq );
void Game_GetOrientation( float *o, float *a );

float g_flStartScaleTime;
int iPrevRenderState;
int iRenderStateChanged;

// Global engine <-> studio model rendering code interface
extern engine_studio_api_t IEngineStudio;

typedef struct
{
	vec3_t		origin;
	vec3_t		angles;

	vec3_t		realangles;

	float		animtime;
	float		frame;
	int			sequence;
	int			gaitsequence;
	float		framerate;

	int			m_fSequenceLoops;
	int			m_fSequenceFinished;

	byte		controller[ 4 ];
	byte		blending[ 2 ];

	latchedvars_t	lv;
} client_anim_state_t;

static client_anim_state_t g_state;
static client_anim_state_t g_clientstate;

// The renderer object, created on the stack.
CGameStudioModelRenderer g_StudioRenderer;
/*
====================
CGameStudioModelRenderer

====================
*/
CGameStudioModelRenderer::CGameStudioModelRenderer( void )
{
	// If you want to predict animations locally, set this to TRUE
	// NOTE:  The animation code is somewhat broken, but gives you a sense for how
	//  to do client side animation of the predicted player in a third person game.
	m_bLocal = false;
}

/*
====================
StudioSetupBones

====================
*/
void CGameStudioModelRenderer::StudioSetupBones ( void )
{
	int					i;
	double				f;

	mstudiobone_t		*pbones;
	mstudioseqdesc_t	*pseqdesc;
	mstudioanim_t		*panim;

	static float		pos[MAXSTUDIOBONES][3];
	static vec4_t		q[MAXSTUDIOBONES];
	float				bonematrix[3][4];

	static float		pos2[MAXSTUDIOBONES][3];
	static vec4_t		q2[MAXSTUDIOBONES];
	static float		pos3[MAXSTUDIOBONES][3];
	static vec4_t		q3[MAXSTUDIOBONES];
	static float		pos4[MAXSTUDIOBONES][3];
	static vec4_t		q4[MAXSTUDIOBONES];

	// Use default bone setup for nonplayers
	if ( !m_pCurrentEntity->player )
	{
		CStudioModelRenderer::StudioSetupBones();
		return;
	}

	// Bound sequence number.
	if ( m_pCurrentEntity->curstate.sequence >= m_pStudioHeader->numseq ) 
	{
		m_pCurrentEntity->curstate.sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	if ( m_pPlayerInfo && m_pPlayerInfo->gaitsequence != 0 )
	{
		f = m_pPlayerInfo->gaitframe;
	}
	else 
	{
		f = StudioEstimateFrame( pseqdesc );
	}

	// This game knows how to do three way blending
	if ( pseqdesc->numblends == 3 )
	{
		float				s;

		// Get left anim
		panim = StudioGetAnim( m_pRenderModel, pseqdesc );

		// Blending is 0-127 == Left to Middle, 128 to 255 == Middle to right
		if ( m_pCurrentEntity->curstate.blending[0] <= 127 )
		{
			StudioCalcRotations( pos, q, pseqdesc, panim, f );
			
			// Scale 0-127 blending up to 0-255
			s = m_pCurrentEntity->curstate.blending[0];
			s = ( s * 2.0 );
		}
		else
		{
			
			// Skip ahead to middle
			panim += m_pStudioHeader->numbones;

			StudioCalcRotations( pos, q, pseqdesc, panim, f );

			// Scale 127-255 blending up to 0-255
			s = m_pCurrentEntity->curstate.blending[0];
			s = 2.0 * ( s - 127.0 );
		}

		// Normalize interpolant
		s /= 255.0;

		// Go to middle or right
		panim += m_pStudioHeader->numbones;

		StudioCalcRotations( pos2, q2, pseqdesc, panim, f );

		// Spherically interpolate the bones
		StudioSlerpBones( q, pos, q2, pos2, s );
	}
	else
	{
		panim = StudioGetAnim( m_pRenderModel, pseqdesc );
		StudioCalcRotations( pos, q, pseqdesc, panim, f );
	}

	// Are we in the process of transitioning from one sequence to another.
	if ( m_fDoInterp &&
		m_pCurrentEntity->latched.sequencetime &&
		( m_pCurrentEntity->latched.sequencetime + 0.2 > m_clTime ) && 
		( m_pCurrentEntity->latched.prevsequence < m_pStudioHeader->numseq ))
	{
		// blend from last sequence
		static float		pos1b[MAXSTUDIOBONES][3];
		static vec4_t		q1b[MAXSTUDIOBONES];
		float				s;

		// Blending value into last sequence
		unsigned char prevseqblending = m_pCurrentEntity->latched.prevseqblending[ 0 ];

		// Point at previous sequenece
		pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->latched.prevsequence;
		
		// Know how to do three way blends
		if ( pseqdesc->numblends == 3 )
		{
			float				s;

			// Get left animation
			panim = StudioGetAnim( m_pRenderModel, pseqdesc );

			if ( prevseqblending <= 127 )
			{
				// Set up bones based on final frame of previous sequence
				StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
				
				s = prevseqblending;
				s = ( s * 2.0 );
			}
			else
			{
				// Skip to middle blend
				panim += m_pStudioHeader->numbones;

				StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

				s = prevseqblending;
				s = 2.0 * ( s - 127.0 );
			}

			// Normalize
			s /= 255.0;

			panim += m_pStudioHeader->numbones;
			StudioCalcRotations( pos2, q2, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );

			// Interpolate bones
			StudioSlerpBones( q1b, pos1b, q2, pos2, s );
		}
		else
		{
			panim = StudioGetAnim( m_pRenderModel, pseqdesc );
			// clip prevframe
			StudioCalcRotations( pos1b, q1b, pseqdesc, panim, m_pCurrentEntity->latched.prevframe );
		}

		// Now blend last frame of previous sequence with current sequence.
		s = 1.0 - (m_clTime - m_pCurrentEntity->latched.sequencetime) / 0.2;
		StudioSlerpBones( q, pos, q1b, pos1b, s );
	}
	else
	{
		m_pCurrentEntity->latched.prevframe = f;
	}

	// Now convert quaternions and bone positions into matrices
	pbones = (mstudiobone_t *)((byte *)m_pStudioHeader + m_pStudioHeader->boneindex);

	for (i = 0; i < m_pStudioHeader->numbones; i++) 
	{
		QuaternionMatrix( q[i], bonematrix );

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1) 
		{
			if ( IEngineStudio.IsHardware() )
			{
				ConcatTransforms ((*m_protationmatrix), bonematrix, (*m_pbonetransform)[i]);
				ConcatTransforms ((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
			}
			else
			{
				ConcatTransforms ((*m_paliastransform), bonematrix, (*m_pbonetransform)[i]);
				ConcatTransforms ((*m_protationmatrix), bonematrix, (*m_plighttransform)[i]);
			}

			// Apply client-side effects to the transformation matrix
			StudioFxTransform( m_pCurrentEntity, (*m_pbonetransform)[i] );
		} 
		else 
		{
			ConcatTransforms ((*m_pbonetransform)[pbones[i].parent], bonematrix, (*m_pbonetransform)[i]);
			ConcatTransforms ((*m_plighttransform)[pbones[i].parent], bonematrix, (*m_plighttransform)[i]);
		}
	}
}

/*
====================
StudioEstimateGait

====================
*/
void CGameStudioModelRenderer::StudioEstimateGait( entity_state_t *pplayer )
{
	float dt;
	vec3_t est_velocity;

	dt = (m_clTime - m_clOldTime);
	dt = max( 0.0, dt );
	dt = min( 1.0, dt );

	if (dt == 0 || m_pPlayerInfo->renderframe == m_nFrameCount)
	{
		m_flGaitMovement = 0;
		return;
	}

	// VectorAdd( pplayer->velocity, pplayer->prediction_error, est_velocity );
	if ( m_fGaitEstimation )
	{
		VectorSubtract( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin, est_velocity );
		VectorCopy( m_pCurrentEntity->origin, m_pPlayerInfo->prevgaitorigin );
		m_flGaitMovement = Length( est_velocity );
		if (dt <= 0 || m_flGaitMovement / dt < 5)
		{
			m_flGaitMovement = 0;
			est_velocity[0] = 0;
			est_velocity[1] = 0;
		}
	}
	else
	{
		VectorCopy( pplayer->velocity, est_velocity );
		m_flGaitMovement = Length( est_velocity ) * dt;
	}

	if (est_velocity[1] == 0 && est_velocity[0] == 0)
	{
		float flYawDiff = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;
		if (flYawDiff > 180)
			flYawDiff -= 360;
		if (flYawDiff < -180)
			flYawDiff += 360;

		if (dt < 0.25)
			flYawDiff *= dt * 4;
		else
			flYawDiff *= dt;

		m_pPlayerInfo->gaityaw += flYawDiff;
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - (int)(m_pPlayerInfo->gaityaw / 360) * 360;

		m_flGaitMovement = 0;
	}
	else
	{
		m_pPlayerInfo->gaityaw = (atan2(est_velocity[1], est_velocity[0]) * 180 / M_PI);
		if (m_pPlayerInfo->gaityaw > 180)
			m_pPlayerInfo->gaityaw = 180;
		if (m_pPlayerInfo->gaityaw < -180)
			m_pPlayerInfo->gaityaw = -180;
	}

}

/*
====================
StudioProcessGait

====================
*/
void CGameStudioModelRenderer::StudioProcessGait( entity_state_t *pplayer )
{
	mstudioseqdesc_t	*pseqdesc;
	float dt;
	float flYaw;	 // view direction relative to movement

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + m_pCurrentEntity->curstate.sequence;

	m_pCurrentEntity->angles[PITCH] = 0;
	m_pCurrentEntity->latched.prevangles[PITCH] = m_pCurrentEntity->angles[PITCH];

	dt = (m_clTime - m_clOldTime);
	dt = max( 0.0, dt );
	dt = min( 1.0, dt );

	StudioEstimateGait( pplayer );

	// calc side to side turning
	flYaw = m_pCurrentEntity->angles[YAW] - m_pPlayerInfo->gaityaw;

	flYaw = fmod( flYaw, 360.0 );

	if (flYaw < -180)
	{
		flYaw = flYaw + 360;
	}
	else if (flYaw > 180)
	{
		flYaw = flYaw - 360;
	}

	float maxyaw = 120.0;

	if (flYaw > maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw - 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw - 180;
	}
	else if (flYaw < -maxyaw)
	{
		m_pPlayerInfo->gaityaw = m_pPlayerInfo->gaityaw + 180;
		m_flGaitMovement = -m_flGaitMovement;
		flYaw = flYaw + 180;
	}

	float blend_yaw = ( flYaw / 90.0 ) * 128.0 + 127.0;
	blend_yaw = min( 255.0, blend_yaw );
	blend_yaw = max( 0.0, blend_yaw );
	
	blend_yaw = 255.0 - blend_yaw;

	m_pCurrentEntity->curstate.blending[0] = (int)(blend_yaw);
	m_pCurrentEntity->latched.prevblending[0] = m_pCurrentEntity->curstate.blending[0];
	m_pCurrentEntity->latched.prevseqblending[0] = m_pCurrentEntity->curstate.blending[0];

	m_pCurrentEntity->angles[YAW] = m_pPlayerInfo->gaityaw;
	if (m_pCurrentEntity->angles[YAW] < -0)
	{
		m_pCurrentEntity->angles[YAW] += 360;
	}
	m_pCurrentEntity->latched.prevangles[YAW] = m_pCurrentEntity->angles[YAW];

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pStudioHeader + m_pStudioHeader->seqindex) + pplayer->gaitsequence;
	
	// Calc gait frame
	if (pseqdesc->linearmovement[0] > 0)
	{
		m_pPlayerInfo->gaitframe += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	}
	else
	{
		m_pPlayerInfo->gaitframe += pseqdesc->fps * dt * m_pCurrentEntity->curstate.framerate;
	}

	// Do modulo
	m_pPlayerInfo->gaitframe = m_pPlayerInfo->gaitframe - (int)(m_pPlayerInfo->gaitframe / pseqdesc->numframes) * pseqdesc->numframes;
	if (m_pPlayerInfo->gaitframe < 0)
	{
		m_pPlayerInfo->gaitframe += pseqdesc->numframes;
	}
}

/*
==============================
SavePlayerState

For local player, in third person, we need to store real render data and then
  setup for with fake/client side animation data
==============================
*/
void CGameStudioModelRenderer::SavePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();
	assert( ent );
	if ( !ent )
		return;

	st = &g_state;

	st->angles		= ent->curstate.angles;
	st->origin		= ent->curstate.origin;

	st->realangles	= ent->angles;

	st->sequence	= ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime	= ent->curstate.animtime;
	st->frame		= ent->curstate.frame;
	st->framerate	= ent->curstate.framerate;
	memcpy( st->blending, ent->curstate.blending, 2 );
	memcpy( st->controller, ent->curstate.controller, 4 );

	st->lv = ent->latched;
}

void GetSequenceInfo( void *pmodel, client_anim_state_t *pev, float *pflFrameRate, float *pflGroundSpeed )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if (! pstudiohdr)
		return;

	mstudioseqdesc_t	*pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

int GetSequenceFlags( void *pmodel, client_anim_state_t *pev )
{
	studiohdr_t *pstudiohdr;
	
	pstudiohdr = (studiohdr_t *)pmodel;
	if ( !pstudiohdr || pev->sequence >= pstudiohdr->numseq )
		return 0;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}

float StudioFrameAdvance ( client_anim_state_t *st, float framerate, float flInterval )
{
	if (flInterval == 0.0)
	{
		flInterval = (gEngfuncs.GetClientTime() - st->animtime);
		if (flInterval <= 0.001)
		{
			st->animtime = gEngfuncs.GetClientTime();
			return 0.0;
		}
	}
	if (!st->animtime)
		flInterval = 0.0;
	
	st->frame += flInterval * framerate * st->framerate;
	st->animtime = gEngfuncs.GetClientTime();

	if (st->frame < 0.0 || st->frame >= 256.0) 
	{
		if ( st->m_fSequenceLoops )
			st->frame -= (int)(st->frame / 256.0) * 256.0;
		else
			st->frame = (st->frame < 0.0) ? 0 : 255;
		st->m_fSequenceFinished = TRUE;	// just in case it wasn't caught in GetEvents
	}

	return flInterval;
}

/*
==============================
SetupClientAnimation

Called to set up local player's animation values
==============================
*/
void CGameStudioModelRenderer::SetupClientAnimation( entity_state_t *pplayer )
{
	static double oldtime;
	double curtime, dt;

	client_anim_state_t *st;
	float fr, gs;

	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();
	assert( ent );
	if ( !ent )
		return;

	curtime = gEngfuncs.GetClientTime();
	dt = curtime - oldtime;
	dt = min( 1.0, max( 0.0, dt ) );

	oldtime = curtime;
	st = &g_clientstate;
	
	st->framerate = 1.0;

	int oldseq = st->sequence;
	Game_GetSequence( &st->sequence, &st->gaitsequence ); //CVAR_GET_FLOAT( "sequence" );
	Game_GetOrientation( (float *)&st->origin, (float *)&st->angles );
	st->realangles = st->angles;

	if ( st->sequence != oldseq )
	{
		st->frame = 0.0;
		st->lv.prevsequence = oldseq;
		st->lv.sequencetime = st->animtime;

		memcpy( st->lv.prevseqblending, st->blending, 2 );
		memcpy( st->lv.prevcontroller, st->controller, 4 );
	}

	void *pmodel = (studiohdr_t *)IEngineStudio.Mod_Extradata( ent->model );

	GetSequenceInfo( pmodel, st, &fr, &gs );
	st->m_fSequenceLoops = ((GetSequenceFlags( pmodel, st ) & STUDIO_LOOPING) != 0);
	StudioFrameAdvance( st, fr, dt );
	
//	gEngfuncs.Con_Printf( "gs %i frame %f\n", st->gaitsequence, st->frame );

	ent->angles				= st->realangles;
	ent->curstate.angles	= st->angles;
	ent->curstate.origin	= st->origin;

	ent->curstate.sequence	= st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime	= st->animtime;
	ent->curstate.frame		= st->frame;
	ent->curstate.framerate	= st->framerate;
	memcpy( ent->curstate.blending, st->blending, 2 );
	memcpy( ent->curstate.controller, st->controller, 4 );

	ent->latched = st->lv;
}

/*
==============================
RestorePlayerState

Called to restore original player state information
==============================
*/
void CGameStudioModelRenderer::RestorePlayerState( entity_state_t *pplayer )
{
	client_anim_state_t *st;
	cl_entity_t *ent = IEngineStudio.GetCurrentEntity();
	assert( ent );
	if ( !ent )
		return;

	st = &g_clientstate;

	st->angles		= ent->curstate.angles;
	st->origin		= ent->curstate.origin;
	st->realangles  = ent->angles;

	st->sequence	= ent->curstate.sequence;
	st->gaitsequence = pplayer->gaitsequence;
	st->animtime	= ent->curstate.animtime;
	st->frame		= ent->curstate.frame;
	st->framerate	= ent->curstate.framerate;
	memcpy( st->blending, ent->curstate.blending, 2 );
	memcpy( st->controller, ent->curstate.controller, 4 );

	st->lv = ent->latched;

	st = &g_state;

	ent->curstate.angles	= st->angles;
	ent->curstate.origin	= st->origin;
	ent->angles				= st->realangles;

	ent->curstate.sequence	= st->sequence;
	pplayer->gaitsequence = st->gaitsequence;
	ent->curstate.animtime	= st->animtime;
	ent->curstate.frame		= st->frame;
	ent->curstate.framerate	= st->framerate;
	memcpy( ent->curstate.blending, st->blending, 2 );
	memcpy( ent->curstate.controller, st->controller, 4 );

	ent->latched = st->lv;
}

/*
==============================
StudioDrawPlayer

==============================
*/
int CGameStudioModelRenderer::StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	int iret = 0;

	bool isLocalPlayer = false;
		
	// Set up for client?
	if ( m_bLocal && IEngineStudio.GetCurrentEntity() == gEngfuncs.GetLocalPlayer() )
	{
		isLocalPlayer = true;
	}

	if ( isLocalPlayer )
	{
		// Store original data
		SavePlayerState( pplayer );

		// Copy in client side animation data
		SetupClientAnimation( pplayer );
	}

	// Call real draw function
	iret = _StudioDrawPlayer( flags, pplayer );

	// Restore for client?
	if ( isLocalPlayer )
	{
		// Restore the original data for the player
		RestorePlayerState( pplayer );
	}

	return iret;
}

/*
====================
_StudioDrawPlayer

====================
*/
int CGameStudioModelRenderer::_StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	alight_t lighting;
	vec3_t dir;

	m_pCurrentEntity = IEngineStudio.GetCurrentEntity();
	IEngineStudio.GetTimes( &m_nFrameCount, &m_clTime, &m_clOldTime );
	IEngineStudio.GetViewInfo( m_vRenderOrigin, m_vUp, m_vRight, m_vNormal );
	IEngineStudio.GetAliasScale( &m_fSoftwareXScale, &m_fSoftwareYScale );

	m_nPlayerIndex = pplayer->number - 1;

	if (m_nPlayerIndex < 0 || m_nPlayerIndex >= gEngfuncs.GetMaxClients())
		return 0;

	m_pRenderModel = IEngineStudio.SetupPlayerModel( m_nPlayerIndex );
	if (m_pRenderModel == NULL)
		return 0;

	m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata (m_pRenderModel);
	IEngineStudio.StudioSetHeader( m_pStudioHeader );
	IEngineStudio.SetRenderModel( m_pRenderModel );

	if (pplayer->gaitsequence)
	{
		vec3_t orig_angles;
		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		VectorCopy( m_pCurrentEntity->angles, orig_angles );
	
		StudioProcessGait( pplayer );

		m_pPlayerInfo->gaitsequence = pplayer->gaitsequence;
		m_pPlayerInfo = NULL;

		StudioSetUpTransform( 0 );
		VectorCopy( orig_angles, m_pCurrentEntity->angles );
	}
	else
	{
		m_pCurrentEntity->curstate.controller[0] = 127;
		m_pCurrentEntity->curstate.controller[1] = 127;
		m_pCurrentEntity->curstate.controller[2] = 127;
		m_pCurrentEntity->curstate.controller[3] = 127;
		m_pCurrentEntity->latched.prevcontroller[0] = m_pCurrentEntity->curstate.controller[0];
		m_pCurrentEntity->latched.prevcontroller[1] = m_pCurrentEntity->curstate.controller[1];
		m_pCurrentEntity->latched.prevcontroller[2] = m_pCurrentEntity->curstate.controller[2];
		m_pCurrentEntity->latched.prevcontroller[3] = m_pCurrentEntity->curstate.controller[3];
		
		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );
		m_pPlayerInfo->gaitsequence = 0;

		StudioSetUpTransform( 0 );
	}

	if (flags & STUDIO_RENDER)
	{
		// see if the bounding box lets us trivially reject, also sets
		if (!IEngineStudio.StudioCheckBBox ())
			return 0;

		(*m_pModelsDrawn)++;
		(*m_pStudioModelCount)++; // render data cache cookie

		if (m_pStudioHeader->numbodyparts == 0)
			return 1;
	}

	m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );
	StudioSetupBones( );
	StudioSaveBones( );
	m_pPlayerInfo->renderframe = m_nFrameCount;

	m_pPlayerInfo = NULL;

	if (flags & STUDIO_EVENTS)
	{
		StudioCalcAttachments( );
		IEngineStudio.StudioClientEvents( );
		// copy attachments into global entity array
		if ( m_pCurrentEntity->index > 0 )
		{
			cl_entity_t *ent = gEngfuncs.GetEntityByIndex( m_pCurrentEntity->index );

			memcpy( ent->attachment, m_pCurrentEntity->attachment, sizeof( vec3_t ) * 4 );
		}
	}

	if (flags & STUDIO_RENDER)
	{
		/*
		if (m_pCvarHiModels->value && m_pRenderModel != m_pCurrentEntity->model  )
		{
			// show highest resolution multiplayer model
			m_pCurrentEntity->curstate.body = 255;
		}

		if (!(m_pCvarDeveloper->value == 0 && gEngfuncs.GetMaxClients() == 1 ) && ( m_pRenderModel == m_pCurrentEntity->model ) )
		{
			m_pCurrentEntity->curstate.body = 1; // force helmet
		}
		*/

		lighting.plightvec = dir;
		IEngineStudio.StudioDynamicLight(m_pCurrentEntity, &lighting );

		IEngineStudio.StudioEntityLight( &lighting );

		// model and frame independant
		IEngineStudio.StudioSetupLighting (&lighting);

		m_pPlayerInfo = IEngineStudio.PlayerInfo( m_nPlayerIndex );

		// get remap colors
		m_nTopColor = m_pPlayerInfo->topcolor;
		if (m_nTopColor < 0)
			m_nTopColor = 0;
		if (m_nTopColor > 360)
			m_nTopColor = 360;
		m_nBottomColor = m_pPlayerInfo->bottomcolor;
		if (m_nBottomColor < 0)
			m_nBottomColor = 0;
		if (m_nBottomColor > 360)
			m_nBottomColor = 360;

		IEngineStudio.StudioSetRemapColors( m_nTopColor, m_nBottomColor );

		StudioRenderModel( );
		m_pPlayerInfo = NULL;

		if (pplayer->weaponmodel)
		{
			cl_entity_t saveent = *m_pCurrentEntity;

			model_t *pweaponmodel = IEngineStudio.GetModelByIndex( pplayer->weaponmodel );

			m_pStudioHeader = (studiohdr_t *)IEngineStudio.Mod_Extradata (pweaponmodel);
			IEngineStudio.StudioSetHeader( m_pStudioHeader );

			StudioMergeBones( pweaponmodel);

			IEngineStudio.StudioSetupLighting (&lighting);

			StudioRenderModel( );

			StudioCalcAttachments( );

			*m_pCurrentEntity = saveent;
		}
	}

	return 1;
}

/*
====================
Studio_FxTransform

====================
*/
void CGameStudioModelRenderer::StudioFxTransform( cl_entity_t *ent, float transform[3][4] )
{
	switch( ent->curstate.renderfx )
	{
	case kRenderFxDistort:
	case kRenderFxHologram:
		if ( gEngfuncs.pfnRandomLong(0,49) == 0 )
		{
			int axis = gEngfuncs.pfnRandomLong(0,1);
			if ( axis == 1 ) // Choose between x & z
				axis = 2;
			VectorScale( transform[axis], gEngfuncs.pfnRandomFloat(1,1.484), transform[axis] );
		}
		else if ( gEngfuncs.pfnRandomLong(0,49) == 0 )
		{
			float offset;
			int axis = gEngfuncs.pfnRandomLong(0,1);
			if ( axis == 1 ) // Choose between x & z
				axis = 2;
			offset = gEngfuncs.pfnRandomFloat(-10,10);
			transform[gEngfuncs.pfnRandomLong(0,2)][3] += offset;
		}
		break;
	case kRenderFxExplode:
		{
			if ( iRenderStateChanged )
			{
				g_flStartScaleTime = m_clTime;
				iRenderStateChanged = FALSE;
			}

			// Make the Model continue to shrink
			float flTimeDelta = m_clTime - g_flStartScaleTime;
			if ( flTimeDelta > 0 )
			{
				float flScale = 0.001;
				// Goes almost all away
				if ( flTimeDelta <= 2.0 )
					flScale = 1.0 - (flTimeDelta / 2.0);

				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 3; j++)
						transform[i][j] *= flScale;
				}
			}
		}
		break;
	}
}

////////////////////////////////////
// Hooks to class implementation
////////////////////////////////////

/*
====================
R_StudioDrawPlayer

====================
*/
int R_StudioDrawPlayer( int flags, entity_state_t *pplayer )
{
	return g_StudioRenderer.StudioDrawPlayer( flags, pplayer );
}

/*
====================
R_StudioDrawModel

====================
*/
int R_StudioDrawModel( int flags )
{
	return g_StudioRenderer.StudioDrawModel( flags );
}

/*
====================
R_StudioInit

====================
*/
void R_StudioInit( void )
{
	g_StudioRenderer.Init();
}

// The simple drawing interface we'll pass back to the engine
r_studio_interface_t studio =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

/*
====================
HUD_GetStudioModelInterface

Export this function for the engine to use the studio renderer class to render objects.
====================
*/
#define DLLEXPORT __declspec( dllexport )
extern "C" int DLLEXPORT HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	if ( version != STUDIO_INTERFACE_VERSION )
		return 0;

	// Point the engine to our callbacks
	*ppinterface = &studio;

	// Copy in engine helper functions
	memcpy( &IEngineStudio, pstudio, sizeof( IEngineStudio ) );

	// Initialize local variables, etc.
	R_StudioInit();

	// Success
	return 1;
}
