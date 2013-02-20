/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/
// studio_render.cpp: routines for drawing Half-Life 3DStudio models
// updates:
// 1-4-99	fixed AdvanceFrame wraping bug

#include <windows.h>

#include <gl\gl.h>
#include <gl\glu.h>

#pragma warning( disable : 4244 ) // double to float


////////////////////////////////////////////////////////////////////////

#include "mathlib.h"
#include "..\..\engine\studio.h"
#include "mdlviewer.h"


////////////////////////////////////////////////////////////////////////

vec3_t			g_xformverts[MAXSTUDIOVERTS];	// transformed vertices
vec3_t			g_lightvalues[MAXSTUDIOVERTS];	// light surface normals
vec3_t			*g_pxformverts;
vec3_t			*g_pvlightvalues;

vec3_t			g_lightvec;						// light vector in model reference frame
vec3_t			g_blightvec[MAXSTUDIOBONES];	// light vectors in bone reference frames
int				g_ambientlight;					// ambient world light
float			g_shadelight;					// direct world light
vec3_t			g_lightcolor;

int				g_smodels_total;				// cookie

float			g_bonetransform[MAXSTUDIOBONES][3][4];	// bone transformation matrix

int				g_chrome[MAXSTUDIOVERTS][2];	// texture coords for surface normals
int				g_chromeage[MAXSTUDIOBONES];	// last time chrome vectors were updated
vec3_t			g_chromeup[MAXSTUDIOBONES];		// chrome vector "up" in bone reference frames
vec3_t			g_chromeright[MAXSTUDIOBONES];	// chrome vector "right" in bone reference frames

////////////////////////////////////////////////////////////////////////

void StudioModel::CalcBoneAdj( )
{
	int					i, j;
	float				value;
	mstudiobonecontroller_t *pbonecontroller;
	
	pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	for (j = 0; j < m_pstudiohdr->numbonecontrollers; j++)
	{
		i = pbonecontroller[j].index;
		if (i <= 3)
		{
			// check for 360% wrapping
			if (pbonecontroller[j].type & STUDIO_RLOOP)
			{
				value = m_controller[i] * (360.0/256.0) + pbonecontroller[j].start;
			}
			else 
			{
				value = m_controller[i] / 255.0;
				if (value < 0) value = 0;
				if (value > 1.0) value = 1.0;
				value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			}
			// Con_DPrintf( "%d %d %f : %f\n", m_controller[j], m_prevcontroller[j], value, dadt );
		}
		else
		{
			value = m_mouth / 64.0;
			if (value > 1.0) value = 1.0;
			value = (1.0 - value) * pbonecontroller[j].start + value * pbonecontroller[j].end;
			// Con_DPrintf("%d %f\n", mouthopen, value );
		}
		switch(pbonecontroller[j].type & STUDIO_TYPES)
		{
		case STUDIO_XR:
		case STUDIO_YR:
		case STUDIO_ZR:
			m_adj[j] = value * (Q_PI / 180.0);
			break;
		case STUDIO_X:
		case STUDIO_Y:
		case STUDIO_Z:
			m_adj[j] = value;
			break;
		}
	}
}


void StudioModel::CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *q )
{
	int					j, k;
	vec4_t				q1, q2;
	vec3_t				angle1, angle2;
	mstudioanimvalue_t	*panimvalue;

	for (j = 0; j < 3; j++)
	{
		if (panim->offset[j+3] == 0)
		{
			angle2[j] = angle1[j] = pbone->value[j+3]; // default;
		}
		else
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j+3]);
			k = frame;
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// Bah, missing blend!
			if (panimvalue->num.valid > k)
			{
				angle1[j] = panimvalue[k+1].value;

				if (panimvalue->num.valid > k + 1)
				{
					angle2[j] = panimvalue[k+2].value;
				}
				else
				{
					if (panimvalue->num.total > k + 1)
						angle2[j] = angle1[j];
					else
						angle2[j] = panimvalue[panimvalue->num.valid+2].value;
				}
			}
			else
			{
				angle1[j] = panimvalue[panimvalue->num.valid].value;
				if (panimvalue->num.total > k + 1)
				{
					angle2[j] = angle1[j];
				}
				else
				{
					angle2[j] = panimvalue[panimvalue->num.valid + 2].value;
				}
			}
			angle1[j] = pbone->value[j+3] + angle1[j] * pbone->scale[j+3];
			angle2[j] = pbone->value[j+3] + angle2[j] * pbone->scale[j+3];
		}

		if (pbone->bonecontroller[j+3] != -1)
		{
			angle1[j] += m_adj[pbone->bonecontroller[j+3]];
			angle2[j] += m_adj[pbone->bonecontroller[j+3]];
		}
	}

	if (!VectorCompare( angle1, angle2 ))
	{
		AngleQuaternion( angle1, q1 );
		AngleQuaternion( angle2, q2 );
		QuaternionSlerp( q1, q2, s, q );
	}
	else
	{
		AngleQuaternion( angle1, q );
	}
}


void StudioModel::CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *pos )
{
	int					j, k;
	mstudioanimvalue_t	*panimvalue;

	for (j = 0; j < 3; j++)
	{
		pos[j] = pbone->value[j]; // default;
		if (panim->offset[j] != 0)
		{
			panimvalue = (mstudioanimvalue_t *)((byte *)panim + panim->offset[j]);
			
			k = frame;
			// find span of values that includes the frame we want
			while (panimvalue->num.total <= k)
			{
				k -= panimvalue->num.total;
				panimvalue += panimvalue->num.valid + 1;
			}
			// if we're inside the span
			if (panimvalue->num.valid > k)
			{
				// and there's more data in the span
				if (panimvalue->num.valid > k + 1)
				{
					pos[j] += (panimvalue[k+1].value * (1.0 - s) + s * panimvalue[k+2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[k+1].value * pbone->scale[j];
				}
			}
			else
			{
				// are we at the end of the repeating values section and there's another section with data?
				if (panimvalue->num.total <= k + 1)
				{
					pos[j] += (panimvalue[panimvalue->num.valid].value * (1.0 - s) + s * panimvalue[panimvalue->num.valid + 2].value) * pbone->scale[j];
				}
				else
				{
					pos[j] += panimvalue[panimvalue->num.valid].value * pbone->scale[j];
				}
			}
		}
		if (pbone->bonecontroller[j] != -1)
		{
			pos[j] += m_adj[pbone->bonecontroller[j]];
		}
	}
}


void StudioModel::CalcRotations ( vec3_t *pos, vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f )
{
	int					i;
	int					frame;
	mstudiobone_t		*pbone;
	float				s;

	frame = (int)f;
	s = (f - frame);

	// add in programatic controllers
	CalcBoneAdj( );

	pbone		= (mstudiobone_t *)((byte *)m_pstudiohdr + m_pstudiohdr->boneindex);
	for (i = 0; i < m_pstudiohdr->numbones; i++, pbone++, panim++) 
	{
		CalcBoneQuaternion( frame, s, pbone, panim, q[i] );
		CalcBonePosition( frame, s, pbone, panim, pos[i] );
	}

	if (pseqdesc->motiontype & STUDIO_X)
		pos[pseqdesc->motionbone][0] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Y)
		pos[pseqdesc->motionbone][1] = 0.0;
	if (pseqdesc->motiontype & STUDIO_Z)
		pos[pseqdesc->motionbone][2] = 0.0;
}


mstudioanim_t * StudioModel::GetAnim( mstudioseqdesc_t *pseqdesc )
{
	mstudioseqgroup_t	*pseqgroup;
	pseqgroup = (mstudioseqgroup_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqgroupindex) + pseqdesc->seqgroup;

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t *)((byte *)m_pstudiohdr + pseqgroup->data + pseqdesc->animindex);
	}

	return (mstudioanim_t *)((byte *)m_panimhdr[pseqdesc->seqgroup] + pseqdesc->animindex);
}


void StudioModel::SlerpBones( vec4_t q1[], vec3_t pos1[], vec4_t q2[], vec3_t pos2[], float s )
{
	int			i;
	vec4_t		q3;
	float		s1;

	if (s < 0) s = 0;
	else if (s > 1.0) s = 1.0;

	s1 = 1.0 - s;

	for (i = 0; i < m_pstudiohdr->numbones; i++)
	{
		QuaternionSlerp( q1[i], q2[i], s, q3 );
		q1[i][0] = q3[0];
		q1[i][1] = q3[1];
		q1[i][2] = q3[2];
		q1[i][3] = q3[3];
		pos1[i][0] = pos1[i][0] * s1 + pos2[i][0] * s;
		pos1[i][1] = pos1[i][1] * s1 + pos2[i][1] * s;
		pos1[i][2] = pos1[i][2] * s1 + pos2[i][2] * s;
	}
}


void StudioModel::AdvanceFrame( float dt )
{
	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + m_sequence;

	if (dt > 0.1)
		dt = (float)0.1;
	m_frame += dt * pseqdesc->fps;

	if (pseqdesc->numframes <= 1)
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= (int)(m_frame / (pseqdesc->numframes - 1)) * (pseqdesc->numframes - 1);
	}
}


void StudioModel::SetUpBones ( void )
{
	int					i;

	mstudiobone_t		*pbones;
	mstudioseqdesc_t	*pseqdesc;
	mstudioanim_t		*panim;

	static vec3_t		pos[MAXSTUDIOBONES];
	float				bonematrix[3][4];
	static vec4_t		q[MAXSTUDIOBONES];

	static vec3_t		pos2[MAXSTUDIOBONES];
	static vec4_t		q2[MAXSTUDIOBONES];
	static vec3_t		pos3[MAXSTUDIOBONES];
	static vec4_t		q3[MAXSTUDIOBONES];
	static vec3_t		pos4[MAXSTUDIOBONES];
	static vec4_t		q4[MAXSTUDIOBONES];


	if (m_sequence >=  m_pstudiohdr->numseq) {
		m_sequence = 0;
	}

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + m_sequence;

	panim = GetAnim( pseqdesc );
	CalcRotations( pos, q, pseqdesc, panim, m_frame );

	if (pseqdesc->numblends > 1)
	{
		float				s;

		panim += m_pstudiohdr->numbones;
		CalcRotations( pos2, q2, pseqdesc, panim, m_frame );
		s = m_blending[0] / 255.0;

		SlerpBones( q, pos, q2, pos2, s );

		if (pseqdesc->numblends == 4)
		{
			panim += m_pstudiohdr->numbones;
			CalcRotations( pos3, q3, pseqdesc, panim, m_frame );

			panim += m_pstudiohdr->numbones;
			CalcRotations( pos4, q4, pseqdesc, panim, m_frame );

			s = m_blending[0] / 255.0;
			SlerpBones( q3, pos3, q4, pos4, s );

			s = m_blending[1] / 255.0;
			SlerpBones( q, pos, q3, pos3, s );
		}
	}

	pbones = (mstudiobone_t *)((byte *)m_pstudiohdr + m_pstudiohdr->boneindex);

	for (i = 0; i < m_pstudiohdr->numbones; i++) {
		QuaternionMatrix( q[i], bonematrix );

		bonematrix[0][3] = pos[i][0];
		bonematrix[1][3] = pos[i][1];
		bonematrix[2][3] = pos[i][2];

		if (pbones[i].parent == -1) {
			memcpy(g_bonetransform[i], bonematrix, sizeof(float) * 12);
		} 
		else {
			R_ConcatTransforms (g_bonetransform[pbones[i].parent], bonematrix, g_bonetransform[i]);
		}
	}
}



/*
================
StudioModel::TransformFinalVert
================
*/
void StudioModel::Lighting (float *lv, int bone, int flags, vec3_t normal)
{
	float 	illum;
	float	lightcos;

	illum = g_ambientlight;

	if (flags & STUDIO_NF_FLATSHADE)
	{
		illum += g_shadelight * 0.8;
	} 
	else 
	{
		float r;
		lightcos = DotProduct (normal, g_blightvec[bone]); // -1 colinear, 1 opposite

		if (lightcos > 1.0)
			lightcos = 1;

		illum += g_shadelight;

		r = g_lambert;
		if (r <= 1.0) r = 1.0;

		lightcos = (lightcos + (r - 1.0)) / r; 		// do modified hemispherical lighting
		if (lightcos > 0.0) 
		{
			illum -= g_shadelight * lightcos; 
		}
		if (illum <= 0)
			illum = 0;
	}

	if (illum > 255) 
		illum = 255;
	*lv = illum / 255.0;	// Light from 0 to 1.0
}


void StudioModel::Chrome (int *pchrome, int bone, vec3_t normal)
{
	float n;

	if (g_chromeage[bone] != g_smodels_total)
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		vec3_t chromeupvec;		// g_chrome t vector in world reference frame
		vec3_t chromerightvec;	// g_chrome s vector in world reference frame
		vec3_t tmp;				// vector pointing at bone in world reference frame
		VectorScale( m_origin, -1, tmp );
		tmp[0] += g_bonetransform[bone][0][3];
		tmp[1] += g_bonetransform[bone][1][3];
		tmp[2] += g_bonetransform[bone][2][3];
		VectorNormalize( tmp );
		CrossProduct( tmp, g_vright, chromeupvec );
		VectorNormalize( chromeupvec );
		CrossProduct( tmp, chromeupvec, chromerightvec );
		VectorNormalize( chromerightvec );

		VectorIRotate( chromeupvec, g_bonetransform[bone], g_chromeup[bone] );
		VectorIRotate( chromerightvec, g_bonetransform[bone], g_chromeright[bone] );

		g_chromeage[bone] = g_smodels_total;
	}

	// calc s coord
	n = DotProduct( normal, g_chromeright[bone] );
	pchrome[0] = (n + 1.0) * 32; // FIX: make this a float

	// calc t coord
	n = DotProduct( normal, g_chromeup[bone] );
	pchrome[1] = (n + 1.0) * 32; // FIX: make this a float
}


/*
================
StudioModel::SetupLighting
	set some global variables based on entity position
inputs:
outputs:
	g_ambientlight
	g_shadelight
================
*/
void StudioModel::SetupLighting ( )
{
	int i;
	g_ambientlight = 32;
	g_shadelight = 192;

	g_lightvec[0] = 0;
	g_lightvec[1] = 0;
	g_lightvec[2] = -1.0;

	g_lightcolor[0] = 1.0;
	g_lightcolor[1] = 1.0;
	g_lightcolor[2] = 1.0;

	// TODO: only do it for bones that actually have textures
	for (i = 0; i < m_pstudiohdr->numbones; i++)
	{
		VectorIRotate( g_lightvec, g_bonetransform[i], g_blightvec[i] );
	}
}


/*
=================
StudioModel::SetupModel
	based on the body part, figure out which mesh it should be using.
inputs:
	currententity
outputs:
	pstudiomesh
	pmdl
=================
*/

void StudioModel::SetupModel ( int bodypart )
{
	int index;

	if (bodypart > m_pstudiohdr->numbodyparts)
	{
		// Con_DPrintf ("StudioModel::SetupModel: no such bodypart %d\n", bodypart);
		bodypart = 0;
	}

	mstudiobodyparts_t   *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + bodypart;

	index = m_bodynum / pbodypart->base;
	index = index % pbodypart->nummodels;

	m_pmodel = (mstudiomodel_t *)((byte *)m_pstudiohdr + pbodypart->modelindex) + index;
}


/*
================
StudioModel::DrawModel
inputs:
	currententity
	r_entorigin
================
*/
void StudioModel::DrawModel( )
{
	int i;

	g_smodels_total++; // render data cache cookie

	g_pxformverts = &g_xformverts[0];
	g_pvlightvalues = &g_lightvalues[0];

	if (m_pstudiohdr->numbodyparts == 0)
		return;

	glPushMatrix ();

    glTranslatef (m_origin[0],  m_origin[1],  m_origin[2]);

    glRotatef (m_angles[1],  0, 0, 1);
    glRotatef (m_angles[0],  0, 1, 0);
    glRotatef (m_angles[2],  1, 0, 0);

	// glShadeModel (GL_SMOOTH);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);

	SetUpBones ( );

	SetupLighting( );
	
	for (i=0 ; i < m_pstudiohdr->numbodyparts ; i++) 
	{
		SetupModel( i );
		DrawPoints( );
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	// glShadeModel (GL_FLAT);

	// glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glPopMatrix ();
}

/*
================

================
*/
void StudioModel::DrawPoints ( )
{
	int					i, j;
	mstudiomesh_t		*pmesh;
	byte				*pvertbone;
	byte				*pnormbone;
	vec3_t				*pstudioverts;
	vec3_t				*pstudionorms;
	mstudiotexture_t	*ptexture;
	float 				*av;
	float				*lv;
	float				lv_tmp;
	short				*pskinref;

	pvertbone = ((byte *)m_pstudiohdr + m_pmodel->vertinfoindex);
	pnormbone = ((byte *)m_pstudiohdr + m_pmodel->norminfoindex);
	ptexture = (mstudiotexture_t *)((byte *)m_ptexturehdr + m_ptexturehdr->textureindex);

	pmesh = (mstudiomesh_t *)((byte *)m_pstudiohdr + m_pmodel->meshindex);

	pstudioverts = (vec3_t *)((byte *)m_pstudiohdr + m_pmodel->vertindex);
	pstudionorms = (vec3_t *)((byte *)m_pstudiohdr + m_pmodel->normindex);

	pskinref = (short *)((byte *)m_ptexturehdr + m_ptexturehdr->skinindex);
	if (m_skinnum != 0 && m_skinnum < m_ptexturehdr->numskinfamilies)
		pskinref += (m_skinnum * m_ptexturehdr->numskinref);

	for (i = 0; i < m_pmodel->numverts; i++)
	{
		VectorTransform (pstudioverts[i], g_bonetransform[pvertbone[i]], g_pxformverts[i]);
	}

//
// clip and draw all triangles
//

	lv = (float *)g_pvlightvalues;
	for (j = 0; j < m_pmodel->nummesh; j++) 
	{
		int flags;
		flags = ptexture[pskinref[pmesh[j].skinref]].flags;
		for (i = 0; i < pmesh[j].numnorms; i++, lv += 3, pstudionorms++, pnormbone++)
		{
			Lighting (&lv_tmp, *pnormbone, flags, (float *)pstudionorms);

			// FIX: move this check out of the inner loop
			if (flags & STUDIO_NF_CHROME)
				Chrome( g_chrome[(float (*)[3])lv - g_pvlightvalues], *pnormbone, (float *)pstudionorms );

			lv[0] = lv_tmp * g_lightcolor[0];
			lv[1] = lv_tmp * g_lightcolor[1];
			lv[2] = lv_tmp * g_lightcolor[2];
		}
	}

	glCullFace(GL_FRONT);

	for (j = 0; j < m_pmodel->nummesh; j++) 
	{
		float s, t;
		short		*ptricmds;

		pmesh = (mstudiomesh_t *)((byte *)m_pstudiohdr + m_pmodel->meshindex) + j;
		ptricmds = (short *)((byte *)m_pstudiohdr + pmesh->triindex);

		s = 1.0/(float)ptexture[pskinref[pmesh->skinref]].width;
		t = 1.0/(float)ptexture[pskinref[pmesh->skinref]].height;

		glBindTexture( GL_TEXTURE_2D, ptexture[pskinref[pmesh->skinref]].index );

		if (ptexture[pskinref[pmesh->skinref]].flags & STUDIO_NF_CHROME)
		{
			while (i = *(ptricmds++))
			{
				if (i < 0)
				{
					glBegin( GL_TRIANGLE_FAN );
					i = -i;
				}
				else
				{
					glBegin( GL_TRIANGLE_STRIP );
				}


				for( ; i > 0; i--, ptricmds += 4)
				{
					// FIX: put these in as integer coords, not floats
					glTexCoord2f(g_chrome[ptricmds[1]][0]*s, g_chrome[ptricmds[1]][1]*t);
					
					lv = g_pvlightvalues[ptricmds[1]];
					glColor4f( lv[0], lv[1], lv[2], 1.0 );

					av = g_pxformverts[ptricmds[0]];
					glVertex3f(av[0], av[1], av[2]);
				}
				glEnd( );
			}	
		} 
		else 
		{
			while (i = *(ptricmds++))
			{
				if (i < 0)
				{
					glBegin( GL_TRIANGLE_FAN );
					i = -i;
				}
				else
				{
					glBegin( GL_TRIANGLE_STRIP );
				}


				for( ; i > 0; i--, ptricmds += 4)
				{
					// FIX: put these in as integer coords, not floats
					glTexCoord2f(ptricmds[2]*s, ptricmds[3]*t);
					
					lv = g_pvlightvalues[ptricmds[1]];
					glColor4f( lv[0], lv[1], lv[2], 1.0 );

					av = g_pxformverts[ptricmds[0]];
					glVertex3f(av[0], av[1], av[2]);
				}
				glEnd( );
			}	
		}
	}
}

