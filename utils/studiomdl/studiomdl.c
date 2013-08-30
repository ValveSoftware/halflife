/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

//
// studiomdl.c: generates a studio .mdl file from a .qc script
// models/<scriptname>.mdl.
//


#pragma warning( disable : 4244 )
#pragma warning( disable : 4237 )
#pragma warning( disable : 4305 )


#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>

#include "archtypes.h"
#include "cmdlib.h"
#include "lbmlib.h"
#include "scriplib.h"
#include "mathlib.h"
#define EXTERN
#include "../../engine/studio.h"
#include "studiomdl.h"
#include "../../dlls/activity.h"
#include "../../dlls/activitymap.h"


static int force_powerof2_textures = 0;

void Sys_Error (char *error, ...) {};

void clip_rotations( vec3_t rot );

#define strcpyn( a, b ) strncpy( a, b, sizeof( a ) )

/*
=================
=================
*/

int k_memtotal;
void *kalloc( int num, int size )
{
	// printf( "calloc( %d, %d )\n", num, size );
	// printf( "%d ", num * size );
	k_memtotal += num * size;
	return calloc( num, size );
}

void kmemset( void *ptr, int value, int size )
{
	// printf( "kmemset( %x, %d, %d )\n", ptr, value, size );
	memset( ptr, value, size );
	return;
}


/*
=================
=================
*/


void ClearModel (void)
{

}

void ExtractMotion( )
{ 
	int i, j, k;
	int	q;

	// extract linear motion
	for (i = 0; i < numseq; i++)
	{
		if (sequence[i].numframes > 1)
		{
			// assume 0 for now.
			int					type;
			vec3_t				*ppos;
			vec3_t				motion = { 0,0,0};
			type = sequence[i].motiontype;
			ppos = sequence[i].panim[0]->pos[0];

			k = sequence[i].numframes - 1;

			if (type & STUDIO_LX)
				motion[0] = ppos[k][0] - ppos[0][0];
			if (type & STUDIO_LY)
				motion[1] = ppos[k][1] - ppos[0][1];
			if (type & STUDIO_LZ)
				motion[2] = ppos[k][2] - ppos[0][2];

			// printf("%f %f %f\n", motion[0], motion[1], motion[2] );
			for (j = 0; j < sequence[i].numframes; j++)
			{	
				vec3_t	adj;
				for (k = 0; k < sequence[i].panim[0]->numbones; k++)
				{
					if (sequence[i].panim[0]->node[k].parent == -1)
					{
						ppos = sequence[i].panim[0]->pos[k];

						VectorScale( motion, j * 1.0 / (sequence[i].numframes - 1), adj );
						// printf("  %f %f %f\n", adj[0], adj[1], adj[2] );
						for (q = 0; q < sequence[i].numblends; q++)
						{
							VectorSubtract( sequence[i].panim[q]->pos[k][j], adj, sequence[i].panim[q]->pos[k][j] );
						}
					}
				}
			}

			VectorCopy( motion, sequence[i].linearmovement );
		}
		else
		{
			VectorSubtract( sequence[i].linearmovement, sequence[i].linearmovement, sequence[i].linearmovement );
		}
	}


	// extract unused motion
	for (i = 0; i < numseq; i++)
	{
		int					type;
		type = sequence[i].motiontype;
		for (k = 0; k < sequence[i].panim[0]->numbones; k++)
		{
			if (sequence[i].panim[0]->node[k].parent == -1)
			{
				for (q = 0; q < sequence[i].numblends; q++)
				{
					float	motion[6];
					motion[0] = sequence[i].panim[q]->pos[k][0][0];
					motion[1] = sequence[i].panim[q]->pos[k][0][1];
					motion[2] = sequence[i].panim[q]->pos[k][0][2];
					motion[3] = sequence[i].panim[q]->rot[k][0][0];
					motion[4] = sequence[i].panim[q]->rot[k][0][1];
					motion[5] = sequence[i].panim[q]->rot[k][0][2];

					for (j = 0; j < sequence[i].numframes; j++)
					{	
						/*
						if (type & STUDIO_X)
							sequence[i].panim[q]->pos[k][j][0] = motion[0];
						if (type & STUDIO_Y)
							sequence[i].panim[q]->pos[k][j][1] = motion[1];
						if (type & STUDIO_Z)
							sequence[i].panim[q]->pos[k][j][2] = motion[2];
						*/
						if (type & STUDIO_XR)
							sequence[i].panim[q]->rot[k][j][0] = motion[3];
						if (type & STUDIO_YR)
							sequence[i].panim[q]->rot[k][j][1] = motion[4];
						if (type & STUDIO_ZR)
							sequence[i].panim[q]->rot[k][j][2] = motion[5];
					}
				}
			}
		}
	}
	

	// extract auto motion
	for (i = 0; i < numseq; i++)
	{
		// assume 0 for now.
		int					type;
		vec3_t				*ppos;
		vec3_t				*prot;
		vec3_t				motion = { 0,0,0};
		vec3_t				angles = { 0,0,0};

		type = sequence[i].motiontype;
		// printf("%f %f %f\n", motion[0], motion[1], motion[2] );
		for (j = 0; j < sequence[i].numframes; j++)
		{	
			ppos = sequence[i].panim[0]->pos[0];
			prot = sequence[i].panim[0]->rot[0];

			if (type & STUDIO_AX)
				motion[0] = ppos[j][0] - ppos[0][0];
			if (type & STUDIO_AY)
				motion[1] = ppos[j][1] - ppos[0][1];
			if (type & STUDIO_AZ)
				motion[2] = ppos[j][2] - ppos[0][2];
			if (type & STUDIO_AXR)
				angles[0] = prot[j][0] - prot[0][0];
			if (type & STUDIO_AYR)
				angles[1] = prot[j][1] - prot[0][1];
			if (type & STUDIO_AZR)
				angles[2] = prot[j][2] - prot[0][2];

			VectorCopy( motion, sequence[i].automovepos[j] );
			VectorCopy( angles, sequence[i].automoveangle[j] );

			for (k = 0; k < sequence[i].panim[0]->numbones; k++)
			{
				if (sequence[i].panim[0]->node[k].parent == -1)
				{
					// printf("  %f %f %f\n", adj[0], adj[1], adj[2] );
					for (q = 0; q < sequence[i].numblends; q++)
					{
						// VectorSubtract( sequence[i].panim[q]->pos[k][j], motion, sequence[i].panim[q]->pos[k][j] );
						// VectorSubtract( sequence[i].panim[q]->rot[k][j], angles, sequence[i].panim[q]->pos[k][j] );
					}
				}
			}
		}
	}
}

void OptimizeAnimations(void)
{
	int i, j;
	int n, m;
	int type;
	int q;
	int	iError = 0;

	// optimize animations
	for (i = 0; i < numseq; i++)
	{
		sequence[i].numframes = sequence[i].panim[0]->endframe - sequence[i].panim[0]->startframe + 1;

		// force looping animations to be looping
		if (sequence[i].flags & STUDIO_LOOPING)
		{
			for (j = 0; j < sequence[i].panim[0]->numbones; j++)
			{
				for (q = 0; q < sequence[i].numblends; q++)
				{
					vec3_t				*ppos = sequence[i].panim[q]->pos[j];
					vec3_t				*prot = sequence[i].panim[q]->rot[j];

					n = 0; // sequence[i].panim[q]->startframe;
					m = sequence[i].numframes - 1; // sequence[i].panim[q]->endframe;
					
					type = sequence[i].motiontype;
					if (!(type & STUDIO_LX))
						ppos[m][0] = ppos[n][0];
					if (!(type & STUDIO_LY))
						ppos[m][1] = ppos[n][1];
					if (!(type & STUDIO_LZ))
						ppos[m][2] = ppos[n][2];

					prot[m][0] = prot[n][0];
					prot[m][1] = prot[n][1];
					prot[m][2] = prot[n][2];
				}
			}
		}

		for (j = 0; j < sequence[i].numevents; j++)
		{
			if (sequence[i].event[j].frame < sequence[i].panim[0]->startframe)
			{
				printf( "sequence %s has event (%d) before first frame (%d)\n", sequence[i].name, sequence[i].event[j].frame, sequence[i].panim[0]->startframe );
				sequence[i].event[j].frame = sequence[i].panim[0]->startframe;
				iError++;
			}
			if (sequence[i].event[j].frame > sequence[i].panim[0]->endframe)
			{
				printf( "sequence %s has event (%d) after last frame (%d)\n", sequence[i].name, sequence[i].event[j].frame, sequence[i].panim[0]->endframe );
				sequence[i].event[j].frame = sequence[i].panim[0]->endframe;
				iError++;
			}
		}

		sequence[i].frameoffset = sequence[i].panim[0]->startframe;
		// printf("\n");
	}
	/*
	if (iError)
		exit(1);
	*/
}

int findNode( char *name )
{
	int k;

	for (k = 0; k < numbones; k++)
	{
		if (strcmp( bonetable[k].name, name ) == 0)
		{
			return k;
		}
	}
	return -1;
}


void MatrixCopy (float in[3][4], float out[3][4])
{
	int i, j;

	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 4; j++)
		{
			out[i][j] = in[i][j];
		}
	}
}



void MakeTransitions( )
{
	int i, j, k;
	int iHit;

	// add in direct node transitions
	for (i = 0; i < numseq; i++)
	{
		if (sequence[i].entrynode != sequence[i].exitnode)
		{
			xnode[sequence[i].entrynode-1][sequence[i].exitnode-1] = sequence[i].exitnode;
			if (sequence[i].nodeflags)
			{
				xnode[sequence[i].exitnode-1][sequence[i].entrynode-1] = sequence[i].entrynode;
			}
		}
		if (sequence[i].entrynode > numxnodes)
			numxnodes = sequence[i].entrynode;
	}

	// add multi-stage transitions 
	do 
	{
		iHit = 0;
		for (i = 1; i <= numxnodes; i++)
		{
			for (j = 1; j <= numxnodes; j++)
			{
				// if I can't go there directly
				if (i != j && xnode[i-1][j-1] == 0)
				{
					for (k = 1; k < numxnodes; k++)
					{
						// but I found someone who knows how that I can get to
						if (xnode[k-1][j-1] > 0 && xnode[i-1][k-1] > 0)
						{
							// then go to them
							xnode[i-1][j-1] = -xnode[i-1][k-1];
							iHit = 1;
							break;
						}
					}
				}
			}
		}
		// reset previous pass so the links can be used in the next pass
		for (i = 1; i <= numxnodes; i++)
		{
			for (j = 1; j <= numxnodes; j++)
			{
				xnode[i-1][j-1] = abs( xnode[i-1][j-1] );
			}
		}
	}
	while (iHit);
}



void SimplifyModel (void)
{
	int i, j, k;
	int n, m, q;
	vec3_t			*defaultpos[MAXSTUDIOSRCBONES];
	vec3_t			*defaultrot[MAXSTUDIOSRCBONES];
	int				iError = 0;

	OptimizeAnimations( );
	ExtractMotion( );
	MakeTransitions( );

	// find used bones
	for (i = 0; i < nummodels; i++)
	{
		for (k = 0; k < MAXSTUDIOSRCBONES; k++)
		{
			model[i]->boneref[k] = 0;
		}
		for (j = 0; j < model[i]->numverts; j++)
		{
			model[i]->boneref[model[i]->vert[j].bone] = 1;
		}
		for (k = 0; k < MAXSTUDIOSRCBONES; k++)
		{
			// tag parent bones as used if child has been used
			if (model[i]->boneref[k])
			{
				n = model[i]->node[k].parent;
				while (n != -1 && !model[i]->boneref[n])
				{
					model[i]->boneref[n] = 1;
					n = model[i]->node[n].parent;
				}
			}
		}
	}

	// rename model bones if needed
	for (i = 0; i < nummodels; i++)
	{
		for (j = 0; j < model[i]->numbones; j++)
		{
			for (k = 0; k < numrenamedbones; k++)
			{
				if (!strcmp( model[i]->node[j].name, renamedbone[k].from))
				{
					strcpy( model[i]->node[j].name, renamedbone[k].to );
					break;
				}
			}
		}
	}

	// union of all used bones
	numbones = 0;
	for (i = 0; i < nummodels; i++)
	{
		for (k = 0; k < MAXSTUDIOSRCBONES; k++)
		{
			model[i]->boneimap[k] = -1;
		}
		for (j = 0; j < model[i]->numbones; j++)
		{
			if (model[i]->boneref[j])
			{
				k = findNode( model[i]->node[j].name );
				if (k == -1)
				{
					// create new bone
					// printf("%d : %s\n", numbones, model[i]->node[j].name );
					k = numbones;
					strcpyn( bonetable[k].name, model[i]->node[j].name );
					if ((n = model[i]->node[j].parent) != -1)
						bonetable[k].parent		= findNode( model[i]->node[n].name );
					else
						bonetable[k].parent		= -1;
					bonetable[k].bonecontroller	= 0;
					bonetable[k].flags		= 0;
					// set defaults
					defaultpos[k] = kalloc( MAXSTUDIOANIMATIONS, sizeof( vec3_t ) );
					defaultrot[k] = kalloc( MAXSTUDIOANIMATIONS, sizeof( vec3_t ) );
					for (n = 0; n < MAXSTUDIOANIMATIONS; n++)
					{
						VectorCopy( model[i]->skeleton[j].pos, defaultpos[k][n] );
						VectorCopy( model[i]->skeleton[j].rot, defaultrot[k][n] );
					}
					VectorCopy( model[i]->skeleton[j].pos, bonetable[k].pos );
					VectorCopy( model[i]->skeleton[j].rot, bonetable[k].rot );
					numbones++;
				}
				else
				{
					// double check parent assignments
					n = model[i]->node[j].parent;
					if (n != -1)
						n = findNode( model[i]->node[n].name );
	 				m = bonetable[k].parent;

					if (n != m)
					{
						printf("illegal parent bone replacement in model \"%s\"\n\t\"%s\" has \"%s\", previously was \"%s\"\n", 
							model[i]->name, 
							model[i]->node[j].name, 
							(n != -1) ? bonetable[n].name : "ROOT",
							(m != -1) ? bonetable[m].name : "ROOT" );
						iError++;
					}
				}
				model[i]->bonemap[j] = k;
				model[i]->boneimap[k] = j;
			}
		}
	}

	if (iError && !(ignore_warnings))
	{
		exit( 1 );
	}

	if (numbones >= MAXSTUDIOBONES)
	{
		Error( "Too many bones used in model, used %d, max %d\n", numbones, MAXSTUDIOBONES );
	}

	// rename sequence bones if needed
	for (i = 0; i < numseq; i++)
	{
		for (j = 0; j < sequence[i].panim[0]->numbones; j++)
		{
			for (k = 0; k < numrenamedbones; k++)
			{
				if (!strcmp( sequence[i].panim[0]->node[j].name, renamedbone[k].from))
				{
					strcpy( sequence[i].panim[0]->node[j].name, renamedbone[k].to );
					break;
				}
			}
		}
	}

	// map each sequences bone list to master list
	for (i = 0; i < numseq; i++)
	{
		for (k = 0; k < MAXSTUDIOSRCBONES; k++)
		{
			sequence[i].panim[0]->boneimap[k] = -1;
		}
		for (j = 0; j < sequence[i].panim[0]->numbones; j++)
		{
			k = findNode( sequence[i].panim[0]->node[j].name );
			
			if (k == -1)
			{
				// printf("unknown bone \"%s\" in sequence \"%s\"\n", sequence[i].panim[0]->node[j].name, sequence[i].name );
				sequence[i].panim[0]->bonemap[j] = -1;
			}
			else
			{
				char *szAnim = "ROOT";
				char *szNode = "ROOT";

				// whoa, check parent connections!
				if (sequence[i].panim[0]->node[j].parent != -1)
					szAnim = sequence[i].panim[0]->node[sequence[i].panim[0]->node[j].parent].name;
				
				if (bonetable[k].parent != -1)
					szNode = bonetable[bonetable[k].parent].name;

				if (strcmp(szAnim, szNode))
				{
					printf("illegal parent bone replacement in sequence \"%s\"\n\t\"%s\" has \"%s\", reference has \"%s\"\n", 
						sequence[i].name, 
						sequence[i].panim[0]->node[j].name, 
						szAnim,
						szNode );
					iError++;
				}
				sequence[i].panim[0]->bonemap[j] = k;
				sequence[i].panim[0]->boneimap[k] = j;
				// VectorCopy( sequence[i].panim[0]->pos[j][0].org, bonetable[k].pos );
				// VectorCopy( sequence[i].panim[0]->rot[j][0].org, bonetable[k].rot );
			}
		}
	}
	if (iError && !(ignore_warnings))
	{
		exit( 1 );
	}

	// link bonecontrollers
	for (i = 0; i < numbonecontrollers; i++)
	{
		for (j = 0; j < numbones; j++)
		{
			if (stricmp( bonecontroller[i].name, bonetable[j].name) == 0)
				break;
		}
		if (j >= numbones)
		{
			Error("unknown bonecontroller link '%s'\n", bonecontroller[i].name );
		}
		bonecontroller[i].bone = j;
	}

	// link attachments
	for (i = 0; i < numattachments; i++)
	{
		for (j = 0; j < numbones; j++)
		{
			if (stricmp( attachment[i].bonename, bonetable[j].name) == 0)
				break;
		}
		if (j >= numbones)
		{
			Error("unknown attachment link '%s'\n", attachment[i].bonename );
		}
		attachment[i].bone = j;
	}

	// relink model
	for (i = 0; i < nummodels; i++)
	{
		for (j = 0; j < model[i]->numverts; j++)
		{
			model[i]->vert[j].bone = model[i]->bonemap[model[i]->vert[j].bone];
		}
		for (j = 0; j < model[i]->numnorms; j++)
		{
			model[i]->normal[j].bone = model[i]->bonemap[model[i]->normal[j].bone];
		}
	}

	// set hitgroups
	for (k = 0; k < numbones; k++)
	{
		bonetable[k].group = -9999;
	}
	for (j = 0; j < numhitgroups; j++)
	{
		for (k = 0; k < numbones; k++)
		{
			if (strcmpi( bonetable[k].name, hitgroup[j].name) == 0)
			{
				bonetable[k].group = hitgroup[j].group;
				break;
			}
		}
		if (k >= numbones)
			Error( "cannot find bone %s for hitgroup %d\n", hitgroup[j].name, hitgroup[j].group );
	}
	for (k = 0; k < numbones; k++)
	{
		if (bonetable[k].group == -9999)
		{
			if (bonetable[k].parent != -1)
				bonetable[k].group = bonetable[bonetable[k].parent].group;
			else
				bonetable[k].group = 0;
		}
	}

	if (numhitboxes == 0)
	{
		// find intersection box volume for each bone
		for (k = 0; k < numbones; k++)
		{
			for (j = 0; j < 3; j++)
			{
				bonetable[k].bmin[j] = 0.0;
				bonetable[k].bmax[j] = 0.0;
			}
		}
		// try all the connect vertices
		for (i = 0; i < nummodels; i++)
		{
			vec3_t	p;
			for (j = 0; j < model[i]->numverts; j++)
			{
				VectorCopy( model[i]->vert[j].org, p );
				k = model[i]->vert[j].bone;

				if (p[0] < bonetable[k].bmin[0]) bonetable[k].bmin[0] = p[0];
				if (p[1] < bonetable[k].bmin[1]) bonetable[k].bmin[1] = p[1];
				if (p[2] < bonetable[k].bmin[2]) bonetable[k].bmin[2] = p[2];
				if (p[0] > bonetable[k].bmax[0]) bonetable[k].bmax[0] = p[0];
				if (p[1] > bonetable[k].bmax[1]) bonetable[k].bmax[1] = p[1];
				if (p[2] > bonetable[k].bmax[2]) bonetable[k].bmax[2] = p[2];
			}
		}
		// add in all your children as well
		for (k = 0; k < numbones; k++)
		{
			if ((j = bonetable[k].parent) != -1)
			{
				if (bonetable[k].pos[0] < bonetable[j].bmin[0]) bonetable[j].bmin[0] = bonetable[k].pos[0];
				if (bonetable[k].pos[1] < bonetable[j].bmin[1]) bonetable[j].bmin[1] = bonetable[k].pos[1];
				if (bonetable[k].pos[2] < bonetable[j].bmin[2]) bonetable[j].bmin[2] = bonetable[k].pos[2];
				if (bonetable[k].pos[0] > bonetable[j].bmax[0]) bonetable[j].bmax[0] = bonetable[k].pos[0];
				if (bonetable[k].pos[1] > bonetable[j].bmax[1]) bonetable[j].bmax[1] = bonetable[k].pos[1];
				if (bonetable[k].pos[2] > bonetable[j].bmax[2]) bonetable[j].bmax[2] = bonetable[k].pos[2];
			}
		}

		for (k = 0; k < numbones; k++)
		{
			if (bonetable[k].bmin[0] < bonetable[k].bmax[0] - 1
				&& bonetable[k].bmin[1] < bonetable[k].bmax[1] - 1
				&& bonetable[k].bmin[2] < bonetable[k].bmax[2] - 1)
			{
				hitbox[numhitboxes].bone = k;
				hitbox[numhitboxes].group = bonetable[k].group;
				VectorCopy( bonetable[k].bmin, hitbox[numhitboxes].bmin );
				VectorCopy( bonetable[k].bmax, hitbox[numhitboxes].bmax );

				if (dump_hboxes)
				{
					printf("$hbox %d \"%s\" %.2f %.2f %.2f  %.2f %.2f %.2f\n",
						hitbox[numhitboxes].group,
						bonetable[hitbox[numhitboxes].bone].name, 
						hitbox[numhitboxes].bmin[0], hitbox[numhitboxes].bmin[1], hitbox[numhitboxes].bmin[2],
						hitbox[numhitboxes].bmax[0], hitbox[numhitboxes].bmax[1], hitbox[numhitboxes].bmax[2] );

				}
				numhitboxes++;
			}
		}
	}
	else
	{
		for (j = 0; j < numhitboxes; j++)
		{
			for (k = 0; k < numbones; k++)
			{
				if (strcmpi( bonetable[k].name, hitbox[j].name) == 0)
				{
					hitbox[j].bone = k;
					break;
				}
			}
			if (k >= numbones)
				Error( "cannot find bone %s for bbox\n", hitbox[j].name );
		}
	}

	// relink animations
	for (i = 0; i < numseq; i++)
	{
		vec3_t			*origpos[MAXSTUDIOSRCBONES];
		vec3_t			*origrot[MAXSTUDIOSRCBONES];

		for (q = 0; q < sequence[i].numblends; q++)
		{
			// save pointers to original animations
			for (j = 0; j < sequence[i].panim[q]->numbones; j++)
			{
				origpos[j] = sequence[i].panim[q]->pos[j];
				origrot[j] = sequence[i].panim[q]->rot[j];
			}

			for (j = 0; j < numbones; j++)
			{
				if ((k = sequence[i].panim[0]->boneimap[j]) >= 0)
				{
					// link to original animations
					sequence[i].panim[q]->pos[j] = origpos[k];
					sequence[i].panim[q]->rot[j] = origrot[k];
				}
				else
				{
					// link to dummy animations
					sequence[i].panim[q]->pos[j] = defaultpos[j];
					sequence[i].panim[q]->rot[j] = defaultrot[j];
				}
			}
		}
		// printf("%s %f\n", sequence[i].name, sequence[i].panim[0]->pos[3][0][0] );
	}

	// find scales for all bones
	for (j = 0; j < numbones; j++)
	{
		for (k = 0; k < 6; k++)
		{
			float minv, maxv, scale;

			if (k < 3) 
			{
				minv = -128.0;
				maxv = 128.0;
			}
			else
			{
				minv = -Q_PI / 8.0;
				maxv = Q_PI / 8.0;
			}

			for (i = 0; i < numseq; i++)
			{
				for (q = 0; q < sequence[i].numblends; q++)
				{
					for (n = 0; n < sequence[i].numframes; n++)
					{
						float v;
						switch(k)
						{
						case 0: 
						case 1: 
						case 2: 
							v = ( sequence[i].panim[q]->pos[j][n][k] - bonetable[j].pos[k] ); 
							break;
						case 3:
						case 4:
						case 5:
							v = ( sequence[i].panim[q]->rot[j][n][k-3] - bonetable[j].rot[k-3] ); 
							if (v >= Q_PI)
								v -= Q_PI * 2;
							if (v < -Q_PI)
								v += Q_PI * 2;
							break;
						}
						if (v < minv)
							minv = v;
						if (v > maxv)
							maxv = v;
					}
				}
			}
			if (minv < maxv)
			{
				if (-minv> maxv)
				{
					scale = minv / -32768.0;
				}
				else
				{
					scale = maxv / 32767;
				}
			}
			else
			{
				scale = 1.0 / 32.0;
			}
			switch(k)
			{
			case 0: 
			case 1: 
			case 2: 
				bonetable[j].posscale[k] = scale;
				break;
			case 3:
			case 4:
			case 5:
				bonetable[j].rotscale[k-3] = scale;
				break;
			}
			// printf("%.0f ", 1.0 / scale );
		}
		// printf("\n" );
	}


	// find bounding box for each sequence
	for (i = 0; i < numseq; i++)
	{
		vec3_t bmin, bmax;

		// find intersection box volume for each bone
		for (j = 0; j < 3; j++)
		{
			bmin[j] = 9999.0;
			bmax[j] = -9999.0;
		}

		for (q = 0; q < sequence[i].numblends; q++)
		{
			for (n = 0; n < sequence[i].numframes; n++)
			{
				float bonetransform[MAXSTUDIOBONES][3][4];	// bone transformation matrix
				float bonematrix[3][4];						// local transformation matrix
				vec3_t pos;

				for (j = 0; j < numbones; j++)
				{
					vec3_t angle;

					// convert to degrees
					angle[0]	= sequence[i].panim[q]->rot[j][n][0] * (180.0 / Q_PI);
					angle[1]	= sequence[i].panim[q]->rot[j][n][1] * (180.0 / Q_PI);
					angle[2]	= sequence[i].panim[q]->rot[j][n][2] * (180.0 / Q_PI);

					AngleMatrix( angle, bonematrix );

					bonematrix[0][3] = sequence[i].panim[q]->pos[j][n][0];
					bonematrix[1][3] = sequence[i].panim[q]->pos[j][n][1];
					bonematrix[2][3] = sequence[i].panim[q]->pos[j][n][2];

					if (bonetable[j].parent == -1)
					{
						MatrixCopy( bonematrix, bonetransform[j] );
					}
					else
					{
						R_ConcatTransforms (bonetransform[bonetable[j].parent], bonematrix, bonetransform[j]);
					}
				}

				for (k = 0; k < nummodels; k++)
				{
					for (j = 0; j < model[k]->numverts; j++)
					{
						VectorTransform( model[k]->vert[j].org, bonetransform[model[k]->vert[j].bone], pos );

						if (pos[0] < bmin[0]) bmin[0] = pos[0];
						if (pos[1] < bmin[1]) bmin[1] = pos[1];
						if (pos[2] < bmin[2]) bmin[2] = pos[2];
						if (pos[0] > bmax[0]) bmax[0] = pos[0];
						if (pos[1] > bmax[1]) bmax[1] = pos[1];
						if (pos[2] > bmax[2]) bmax[2] = pos[2];
					}
				}
			}
		}

		VectorCopy( bmin, sequence[i].bmin );
		VectorCopy( bmax, sequence[i].bmax );

		/*
		printf("%s : %.0f %.0f %.0f %.0f %.0f %.0f\n", 
			sequence[i].name, bmin[0], bmax[0], bmin[1], bmax[1], bmin[2], bmax[2] );
		*/
		// printf("%s  %.2f\n", sequence[i].name, sequence[i].panim[0]->pos[9][0][0] / bonetable[9].pos[0] );
	}

	// reduce animations
	{
		int total = 0;
		int changes = 0;
		int p;
		
		for (i = 0; i < numseq; i++)
		{
			for (q = 0; q < sequence[i].numblends; q++)
			{
				for (j = 0; j < numbones; j++)
				{
					for (k = 0; k < 6; k++)
					{
						mstudioanimvalue_t	*pcount, *pvalue;
						float v;
						short value[MAXSTUDIOANIMATIONS];
						mstudioanimvalue_t data[MAXSTUDIOANIMATIONS];

						for (n = 0; n < sequence[i].numframes; n++)
						{
							switch(k)
							{
							case 0: 
							case 1: 
							case 2: 
								value[n] = ( sequence[i].panim[q]->pos[j][n][k] - bonetable[j].pos[k] ) / bonetable[j].posscale[k]; 
								break;
							case 3:
							case 4:
							case 5:
								v = ( sequence[i].panim[q]->rot[j][n][k-3] - bonetable[j].rot[k-3] ); 
								if (v >= Q_PI)
									v -= Q_PI * 2;
								if (v < -Q_PI)
									v += Q_PI * 2;

								value[n] = v / bonetable[j].rotscale[k-3]; 
								break;
							}
						}
						if (n == 0)
							Error("no animation frames: \"%s\"\n", sequence[i].name );


						sequence[i].panim[q]->numanim[j][k] = 0;

						memset( data, 0, sizeof( data ) ); 
						pcount = data; 
						pvalue = pcount + 1;

						pcount->num.valid = 1;
						pcount->num.total = 1;
						pvalue->value = value[0];
						pvalue++;

						for (m = 1, p = 0; m < n; m++)
						{
							if (abs(value[p] - value[m]) > 1600)
							{
								changes++;
								p = m;
							}
						}

						// this compression algorithm needs work

						for (m = 1; m < n; m++)
						{
							if (pcount->num.total == 255)
							{
								// too many, force a new entry
								pcount = pvalue;
								pvalue = pcount + 1;
								pcount->num.valid++;
								pvalue->value = value[m];
								pvalue++;
							} 
							// insert value if they're not equal, 
							// or if we're not on a run and the run is less than 3 units
							else if ((value[m] != value[m-1]) 
								|| ((pcount->num.total == pcount->num.valid) && ((m < n - 1) && value[m] != value[m+1])))
							{
								total++;
								if (pcount->num.total != pcount->num.valid)
								{
									//if (j == 0) printf("%d:%d   ", pcount->num.valid, pcount->num.total ); 
									pcount = pvalue;
									pvalue = pcount + 1;
								}
								pcount->num.valid++;
								pvalue->value = value[m];
								pvalue++;
							}
							pcount->num.total++;
						}
						//if (j == 0) printf("%d:%d\n", pcount->num.valid, pcount->num.total ); 

						sequence[i].panim[q]->numanim[j][k] = pvalue - data;
						if (sequence[i].panim[q]->numanim[j][k] == 2 && value[0] == 0)
						{
							sequence[i].panim[q]->numanim[j][k] = 0;
						}
						else
						{
							sequence[i].panim[q]->anim[j][k] = kalloc( pvalue - data, sizeof( mstudioanimvalue_t ) );
							memmove( sequence[i].panim[q]->anim[j][k], data, (pvalue - data) * sizeof( mstudioanimvalue_t ) );
						}
						// printf("%d(%d) ", sequence[i].panim[q]->numanim[j][k], n );
					}
					// printf("\n");
				}
			}
		}
		// printf("total %.0f changes %.0f\n", total, changes );
	}

	// auto groups
	if (numseqgroups == 1 && maxseqgroupsize < 1024 * 1024) 
	{	
		int current = 0;

		numseqgroups = 2;

		for (i = 0; i < numseq; i++)
		{
			int accum = 0;

			if (sequence[i].activity == 0)
			{
				for (q = 0; q < sequence[i].numblends; q++)
				{
					for (j = 0; j < numbones; j++)
					{
						for (k = 0; k < 6; k++)
						{
							accum += sequence[i].panim[q]->numanim[j][k] * sizeof( mstudioanimvalue_t );
						}
					}
				}
				accum += sequence[i].numblends * numbones * sizeof( mstudioanim_t );
			
				if (current && current + accum > maxseqgroupsize)
				{
					numseqgroups++;
					current = accum;
				}
				else
				{
					current += accum;
				}
				// printf("%d %d %d\n", numseqgroups, current, accum );
				sequence[i].seqgroup = numseqgroups - 1;
			}
			else
			{
				sequence[i].seqgroup = 0;
			}
		}
	}
}





/*
=================
=================
*/




int lookupControl( char *string )
{
	if (stricmp(string,"X")==0) return STUDIO_X;
	if (stricmp(string,"Y")==0) return STUDIO_Y;
	if (stricmp(string,"Z")==0) return STUDIO_Z;
	if (stricmp(string,"XR")==0) return STUDIO_XR;
	if (stricmp(string,"YR")==0) return STUDIO_YR;
	if (stricmp(string,"ZR")==0) return STUDIO_ZR;
	if (stricmp(string,"LX")==0) return STUDIO_LX;
	if (stricmp(string,"LY")==0) return STUDIO_LY;
	if (stricmp(string,"LZ")==0) return STUDIO_LZ;
	if (stricmp(string,"AX")==0) return STUDIO_AX;
	if (stricmp(string,"AY")==0) return STUDIO_AY;
	if (stricmp(string,"AZ")==0) return STUDIO_AZ;
	if (stricmp(string,"AXR")==0) return STUDIO_AXR;
	if (stricmp(string,"AYR")==0) return STUDIO_AYR;
	if (stricmp(string,"AZR")==0) return STUDIO_AZR;
	return -1;
}


// search case-insensitive for string2 in string
char *stristr( const char *string, const char *string2 )
{
	int c, len;
	c = tolower( *string2 );
	len = strlen( string2 );

	while (string) {
		for (; *string && tolower( *string ) != c; string++);
		if (*string) {
			if (strnicmp( string, string2, len ) == 0) {
				break;
			}
			string++;
		}
		else {
			return NULL;
		}
	}
	return (char *)string;
}

/*
=================
=================
*/


int lookup_texture( char *texturename )
{
	int i;

	for (i = 0; i < numtextures; i++) {
		if (stricmp( texture[i].name, texturename ) == 0) {
			return i;
		}
	}

	strcpyn( texture[i].name, texturename );

	if (stristr( texturename, "chrome" ) != NULL) {
		texture[i].flags = STUDIO_NF_FLATSHADE | STUDIO_NF_CHROME;
	}
	else {
		texture[i].flags = 0;
	}
	numtextures++;
	return i;
}


s_mesh_t *lookup_mesh( s_model_t *pmodel, char *texturename )
{
	int i, j;

	j = lookup_texture( texturename );

	for (i = 0; i < pmodel->nummesh; i++) {
		if (pmodel->pmesh[i]->skinref == j) {
			return pmodel->pmesh[i];
		}
	}
	
	if (i >= MAXSTUDIOMESHES) {
		Error( "too many textures in model: \"%s\"\n", pmodel->name );
	}

	pmodel->nummesh = i + 1;
	pmodel->pmesh[i] = kalloc( 1, sizeof( s_mesh_t ) );
	pmodel->pmesh[i]->skinref = j;
	

	return pmodel->pmesh[i];
}




s_trianglevert_t *lookup_triangle( s_mesh_t *pmesh, int index )
{
	if (index >= pmesh->alloctris) {
		int start = pmesh->alloctris;
		pmesh->alloctris = index + 256;
		if (pmesh->triangle) {
			pmesh->triangle = realloc( pmesh->triangle, pmesh->alloctris * sizeof( *pmesh->triangle ) );
			kmemset( &pmesh->triangle[start], 0, (pmesh->alloctris - start) * sizeof( *pmesh->triangle ) );
		} 
		else {
			pmesh->triangle = kalloc( pmesh->alloctris, sizeof( *pmesh->triangle ) );
		}
	}

	return pmesh->triangle[index];
}

int lookup_normal( s_model_t *pmodel, s_normal_t *pnormal )
{
	int i;
	for (i = 0; i < pmodel->numnorms; i++) {
		// if (VectorCompare( pmodel->normal[i].org, pnormal->org )
		if (DotProduct( pmodel->normal[i].org, pnormal->org ) > normal_blend
			&& pmodel->normal[i].bone == pnormal->bone
			&& pmodel->normal[i].skinref == pnormal->skinref) {
			return i;
		}
	}
	if (i >= MAXSTUDIOVERTS) {
		Error( "too many normals in model: \"%s\"\n", pmodel->name);
	}
	VectorCopy( pnormal->org, pmodel->normal[i].org );
	pmodel->normal[i].bone = pnormal->bone;
	pmodel->normal[i].skinref = pnormal->skinref;
	pmodel->numnorms = i + 1;
	return i;
}


int lookup_vertex( s_model_t *pmodel, s_vertex_t *pv )
{
	int i;

	// assume 2 digits of accuracy
	pv->org[0] = (int)(pv->org[0] * 100) / 100.0;
	pv->org[1] = (int)(pv->org[1] * 100) / 100.0;
	pv->org[2] = (int)(pv->org[2] * 100) / 100.0;

	for (i = 0; i < pmodel->numverts; i++) {
		if (VectorCompare( pmodel->vert[i].org, pv->org )
			&& pmodel->vert[i].bone == pv->bone) {
			return i;
		}
	}
	if (i >= MAXSTUDIOVERTS) {
		Error( "too many vertices in model: \"%s\"\n", pmodel->name);
	}
	VectorCopy( pv->org, pmodel->vert[i].org );
	pmodel->vert[i].bone = pv->bone;
	pmodel->numverts = i + 1;
	return i;
}


void adjust_vertex( float *org )
{
	org[0] = (org[0] - adjust[0]);
	org[1] = (org[1] - adjust[1]);
	org[2] = (org[2] - adjust[2]);
}

void scale_vertex( float *org )
{
	float tmp = org[0];
	org[0] = org[0] * scale_up;
	org[1] = org[1] * scale_up;
	org[2] = org[2] * scale_up;
}



/*
============
SetSkinValues

Called for the base frame
============
*/
void TextureCoordRanges( s_mesh_t *pmesh, s_texture_t *ptexture  )
{
	int			i, j;

	if (ptexture->flags & STUDIO_NF_CHROME) {
		ptexture->skintop = 0;
		ptexture->skinleft = 0;
		ptexture->skinwidth = (ptexture->srcwidth + 3) & ~3;
		ptexture->skinheight = ptexture->srcheight;

		for (i=0 ; i<pmesh->numtris ; i++) {
			for (j = 0; j < 3; j++) {
				pmesh->triangle[i][j].s = 0;
				pmesh->triangle[i][j].t = 0;
			}
			ptexture->max_s = 63;
			ptexture->min_s = 0;
			ptexture->max_t = 63;
			ptexture->min_t = 0;
		}
		return;
	}

	// clip texture coords.
	for (i=0 ; i<pmesh->numtris ; i++) {
		if (pmesh->triangle[i][0].u > 100.0 || pmesh->triangle[i][1].u > 100.0 || pmesh->triangle[i][2].u > 100.0)
		{
			// printf("%d : %f %f %f\n", i, pmesh->triangle[i][0].u, pmesh->triangle[i][1].u, pmesh->triangle[i][2].u );
		}
		if (pmesh->triangle[i][0].v > 100.0 || pmesh->triangle[i][1].v > 100.0 || pmesh->triangle[i][2].v > 100.0)
		{
			// printf("%d : %f %f %f\n", i, pmesh->triangle[i][0].v, pmesh->triangle[i][1].v, pmesh->triangle[i][2].v );
		}
	}
	for (i=0 ; i<pmesh->numtris ; i++) {
		for (j = 0; j < 3; j++) {
			if (pmesh->triangle[i][j].u > 2.0) pmesh->triangle[i][j].u = 2.0;
			if (pmesh->triangle[i][j].u < -1.0) pmesh->triangle[i][j].u = -1.0;
			if (pmesh->triangle[i][j].v > 2.0) pmesh->triangle[i][j].v = 2.0;
			if (pmesh->triangle[i][j].v < -1.0) pmesh->triangle[i][j].v = -1.0;
		}
	}
	// pack texture coords

	if (!clip_texcoords)
	{
		int k, n;
		do 
		{
			float min_u = 10;
			float max_u = -10;
			float k_max_u, n_min_u;
			k = -1;
			n = -1;
			for (i=0 ; i<pmesh->numtris ; i++) 
			{
				float local_min, local_max;
				local_min = min( pmesh->triangle[i][0].u, min( pmesh->triangle[i][1].u, pmesh->triangle[i][2].u ));
				local_max = max( pmesh->triangle[i][0].u, max( pmesh->triangle[i][1].u, pmesh->triangle[i][2].u ));
				if (local_min < min_u) { min_u = local_min; k = i; k_max_u = local_max; }
				if (local_max > max_u) { max_u = local_max; n = i; n_min_u = local_min; }
			}

			if (k_max_u + 1.0 < max_u)
			{
				//printf("%d %f %f\n", k, k_max_u, max_u ); 
				for (j = 0; j < 3; j++)
					pmesh->triangle[k][j].u += 1.0;
			}
			else if (n_min_u - 1.0 > min_u)
			{
				//printf("%d %f %f\n", n, n_min_u, min_u ); 
				for (j = 0; j < 3; j++)
					pmesh->triangle[n][j].u -= 1.0;
			}
			else
			{
				break;
			}
		} while (1);
		do 
		{
			float min_v = 10;
			float max_v = -10;
			float k_max_v, n_min_v;
			k = -1;
			n = -1;
			for (i=0 ; i<pmesh->numtris ; i++) 
			{
				float local_min, local_max;
				local_min = min( pmesh->triangle[i][0].v, min( pmesh->triangle[i][1].v, pmesh->triangle[i][2].v ));
				local_max = max( pmesh->triangle[i][0].v, max( pmesh->triangle[i][1].v, pmesh->triangle[i][2].v ));
				if (local_min < min_v) { min_v = local_min; k = i; k_max_v = local_max; }
				if (local_max > max_v) { max_v = local_max; n = i; n_min_v = local_min; }
			}

			if (k_max_v + 1.0 < max_v)
			{
				//printf("%d %f %f\n", k, k_max_v, max_v ); 
				for (j = 0; j < 3; j++)
					pmesh->triangle[k][j].v += 1.0;
			}
			else if (n_min_v - 1.0 > min_v)
			{
				//printf("%d %f %f\n", n, n_min_v, min_v ); 
				for (j = 0; j < 3; j++)
					pmesh->triangle[n][j].v -= 1.0;
			}
			else
			{
				break;
			}
		} while (1);
	}
	else
	{
		for (i=0 ; i<pmesh->numtris ; i++) 
		{
			for (j = 0; j < 3; j++) 
			{
				if (pmesh->triangle[i][j].u < 0) pmesh->triangle[i][j].u = 0;
				if (pmesh->triangle[i][j].u > 1) pmesh->triangle[i][j].u = 1;
				if (pmesh->triangle[i][j].v < 0) pmesh->triangle[i][j].v = 0;
				if (pmesh->triangle[i][j].v > 1) pmesh->triangle[i][j].v = 1;
			}
		}
	}
	// convert to pixel coordinates
	for (i=0 ; i<pmesh->numtris ; i++) {
		for (j = 0; j < 3; j++) {
			// FIXME losing texture coord resultion!
			pmesh->triangle[i][j].s = pmesh->triangle[i][j].u * (ptexture->srcwidth - 1);
			pmesh->triangle[i][j].t = pmesh->triangle[i][j].v * (ptexture->srcheight - 1);
		}
	}

	// find the range
	if (!clip_texcoords)
	{
		for (i=0 ; i<pmesh->numtris ; i++) {
			for (j = 0; j < 3; j++) {
				ptexture->max_s = max( pmesh->triangle[i][j].s, ptexture->max_s );
				ptexture->min_s = min( pmesh->triangle[i][j].s, ptexture->min_s );
				ptexture->max_t = max( pmesh->triangle[i][j].t, ptexture->max_t );
				ptexture->min_t = min( pmesh->triangle[i][j].t, ptexture->min_t );
			}
		}
	}
	else
	{
		ptexture->max_s = ptexture->srcwidth-1;
		ptexture->min_s = 0;
		ptexture->max_t = ptexture->srcheight-1;
		ptexture->min_t = 0;
	}
	//printf("%d %d : ", ptexture->srcwidth, ptexture->srcheight );
	//printf("%.0f %.0f %.0f %.0f\n", ptexture->min_s, ptexture->max_s, ptexture->min_t, ptexture->max_t );
}


void ResetTextureCoordRanges( s_mesh_t *pmesh, s_texture_t *ptexture  )
{
	int i, j;

	// adjust top, left edge
	for (i=0 ; i<pmesh->numtris ; i++) {
		for (j = 0; j < 3; j++) {
			pmesh->triangle[i][j].s -= ptexture->min_s;
			// quake wants t inverted
			pmesh->triangle[i][j].t = (ptexture->max_t - ptexture->min_t) - (pmesh->triangle[i][j].t - ptexture->min_t);
		}
	}
}



/*
===============
Grab_Skin
===============
*/
void Grab_BMP ( char *filename, s_texture_t *ptexture )
{
	int		result;

	if (result = LoadBMP(filename, &ptexture->ppicture, (byte **)&ptexture->ppal )) {
		Error("error %d reading BMP image \"%s\"\n", result, filename );
	}

	ptexture->srcwidth = bmhd.w;
	ptexture->srcheight = bmhd.h;

}

#define MIN_DIMENSION 8
#define MAX_DIMENSION 512

int GetBestPowerOf2( int value )
{
	int i;
	int power = MIN_DIMENSION;

	for(i=0;i<32;i++)
	{
		if ( (1<<i) < MIN_DIMENSION )
			continue;

		if ( (1<<i) > MAX_DIMENSION )
			continue;

		power=(1<<i);
		if(power>=value)
		{
			break;
		}
	}

	return power;
}

int GetSkinWidth( int rawsize )
{
	if ( !force_powerof2_textures )
	{
		return (int)( rawsize + 3) & ~3;
	}

	return GetBestPowerOf2( rawsize );	
}

int GetSkinHeight( int rawsize )
{
	if ( !force_powerof2_textures )
	{
		return ( rawsize );
	}

	return GetBestPowerOf2( rawsize );	
}

void ResizeTexture( s_texture_t *ptexture )
{
	int		i, j, s, t;
    byte	*pdest;
	int		srcadjwidth;

// make the width a multiple of 4; some hardware requires this, and it ensures
// dword alignment for each scan

	ptexture->skintop = ptexture->min_t;
	ptexture->skinleft = ptexture->min_s;

	ptexture->skinwidth = GetSkinWidth( ptexture->max_s - ptexture->min_s + 1);
	ptexture->skinheight = GetSkinHeight(ptexture->max_t - ptexture->min_t + 1);

	ptexture->size = ptexture->skinwidth * ptexture->skinheight + 256 * 3;

	printf ("BMP %s [%d %d] (%.0f%%)  %6d bytes\n", ptexture->name,  ptexture->skinwidth, ptexture->skinheight, 
		((ptexture->skinwidth * ptexture->skinheight) / (float)(ptexture->srcwidth * ptexture->srcheight)) * 100.0,
		ptexture->size );
	
	if (ptexture->size > 640 * 480)
	{
		printf("%.0f %.0f %.0f %.0f\n", ptexture->min_s, ptexture->max_s, ptexture->min_t, ptexture->max_t );
		Error("texture too large\n");
	}
	pdest = malloc( ptexture->size );
	ptexture->pdata = pdest;

	// data is saved as a multiple of 4
	srcadjwidth = (ptexture->srcwidth + 3) & ~3;

	// move the picture data to the model area, replicating missing data, deleting unused data.
	for (i = 0, t = ptexture->srcheight - ptexture->skinheight - ptexture->skintop + 10 * ptexture->srcheight; i < ptexture->skinheight; i++, t++) {
		while (t >= ptexture->srcheight) t -= ptexture->srcheight;
		while (t < 0) t += ptexture->srcheight;
		for (j = 0, s = ptexture->skinleft + 10 * ptexture->srcwidth; j < ptexture->skinwidth; j++, s++) {
			while (s >= ptexture->srcwidth) s -= ptexture->srcwidth;
			*(pdest++) = *(ptexture->ppicture + s + t * srcadjwidth);
		}
	}

	// TODO: process the texture and flag it if fullbright or transparent are used.
	// TODO: only save as many palette entries as are actually used.
	if (gamma != 1.8)
	{
		// gamma correct the monster textures to a gamma of 1.8
		float g;
		byte *psrc = (byte *)ptexture->ppal;
		g = gamma / 1.8;
		for (i = 0; i < 768; i++)
		{
			pdest[i] = pow( psrc[i] / 255.0, g ) * 255;
		}
	}
	else
	{
		memcpy( pdest, ptexture->ppal, 256 * sizeof( rgb_t ) );
	}

	free( ptexture->ppicture );
	free( ptexture->ppal );
}


void Grab_Skin ( s_texture_t *ptexture )
{
	char	file1[1024];
	int		time1;

	sprintf (file1, "%s/%s", cdpartial, ptexture->name);
	ExpandPathAndArchive (file1);

	if (cdtextureset)
	{
		int i;
		for (i = 0; i < cdtextureset; i++)
		{
			sprintf (file1, "%s/%s", cdtexture[i], ptexture->name);
			time1 = FileTime (file1);
			if (time1 != -1)
				break;
		}
		if (time1 == -1)
			Error( "%s not found", file1);
	}
	else
	{
		sprintf (file1, "%s/%s", cddir, ptexture->name);
	}
	
	if (stricmp( ".bmp", &file1[strlen(file1)-4]) == 0) {
		Grab_BMP( file1, ptexture );
	}
	else {
		Error("unknown graphics type: \"%s\"\n", file1 );
	}

}

void SetSkinValues( )
{
	int			i, j;
	int			index;

	for (i = 0; i < numtextures; i++)
	{
		Grab_Skin ( &texture[i] );

		texture[i].max_s = -9999999;
		texture[i].min_s = 9999999;
		texture[i].max_t = -9999999;
		texture[i].min_t = 9999999;
	}

	for (i = 0; i < nummodels; i++)
	{
		for (j = 0; j < model[i]->nummesh; j++) 
		{
			TextureCoordRanges( model[i]->pmesh[j], &texture[model[i]->pmesh[j]->skinref] );
		}
	}


	for (i = 0; i < numtextures; i++)
	{
		if (texture[i].max_s < texture[i].min_s )
		{
			// must be a replacement texture
			if (texture[i].flags & STUDIO_NF_CHROME)
			{
				texture[i].max_s = 63;
				texture[i].min_s = 0;
				texture[i].max_t = 63;
				texture[i].min_t = 0;
			}
			else
			{
				texture[i].max_s = texture[texture[i].parent].max_s;
				texture[i].min_s = texture[texture[i].parent].min_s;
				texture[i].max_t = texture[texture[i].parent].max_t;
				texture[i].min_t = texture[texture[i].parent].min_t;
			}
		}

		ResizeTexture( &texture[i] );
	}

	for (i = 0; i < nummodels; i++)
	{
		for (j = 0; j < model[i]->nummesh; j++) 
		{
			ResetTextureCoordRanges( model[i]->pmesh[j], &texture[model[i]->pmesh[j]->skinref] );
		}
	}

	// build texture groups
	for (i = 0; i < MAXSTUDIOSKINS; i++)
	{
		for (j = 0; j < MAXSTUDIOSKINS; j++)
		{
			skinref[i][j] = j;
		}
	}
	index = 0;
	for (i = 0; i < numtexturelayers[0]; i++)
	{
		for (j = 0; j < numtexturereps[0]; j++)
		{
			skinref[i][texturegroup[0][0][j]] = texturegroup[0][i][j];
		}
	}
	if (i != 0)
	{
		numskinfamilies = i;
	}
	else
	{
		numskinfamilies = 1;
		numskinref = numtextures;
	}


	// printf ("width: %i  height: %i\n",width, height);
	/*
	printf ("adjusted width: %i height: %i  top : %i  left: %i\n",
			pmesh->skinwidth, pmesh->skinheight, pmesh->skintop, pmesh->skinleft );
	*/
}



/*
=================
=================
*/

char	filename[1024];
FILE	*input;
char	line[1024];
int		linecount;


void Build_Reference( s_model_t *pmodel)
{
	int		i, parent;
	float	angle[3];

	for (i = 0; i < pmodel->numbones; i++)
	{
		float m[3][4];
		vec3_t p;

		// convert to degrees
		angle[0] = pmodel->skeleton[i].rot[0] * (180.0 / Q_PI);
		angle[1] = pmodel->skeleton[i].rot[1] * (180.0 / Q_PI);
		angle[2] = pmodel->skeleton[i].rot[2] * (180.0 / Q_PI);

		parent = pmodel->node[i].parent;
		if (parent == -1) {
			// scale the done pos.
			// calc rotational matrices
			AngleMatrix( angle, bonefixup[i].m );
			AngleIMatrix( angle, bonefixup[i].im );
			VectorCopy( pmodel->skeleton[i].pos, bonefixup[i].worldorg );
		}
		else {
			// calc compound rotational matrices
			// FIXME : Hey, it's orthogical so inv(A) == transpose(A)
			AngleMatrix( angle, m );
			R_ConcatTransforms( bonefixup[parent].m, m, bonefixup[i].m );
			AngleIMatrix( angle, m );
			R_ConcatTransforms( m, bonefixup[parent].im, bonefixup[i].im );

			// calc true world coord.
			VectorTransform(pmodel->skeleton[i].pos, bonefixup[parent].m, p );
			VectorAdd( p, bonefixup[parent].worldorg, bonefixup[i].worldorg );
		}
		// printf("%3d %f %f %f\n", i, bonefixup[i].worldorg[0], bonefixup[i].worldorg[1], bonefixup[i].worldorg[2] );
		/*
		AngleMatrix( angle, m );
		printf("%8.4f %8.4f %8.4f\n", m[0][0], m[1][0], m[2][0] );
		printf("%8.4f %8.4f %8.4f\n", m[0][1], m[1][1], m[2][1] );
		printf("%8.4f %8.4f %8.4f\n", m[0][2], m[1][2], m[2][2] );
		*/
	}
}


void Grab_Triangles( s_model_t *pmodel )
{
	int		i, j;
	int		tcount = 0;	
	int		ncount = 0;
	vec3_t	vmin, vmax;

	vmin[0] = vmin[1] = vmin[2] = 99999;
	vmax[0] = vmax[1] = vmax[2] = -99999;

	Build_Reference( pmodel );

//
// load the base triangles
//
	while (1) 
	{
		if (fgets( line, sizeof( line ), input ) != NULL) 
		{
			s_mesh_t *pmesh;
			char texturename[64];
			s_trianglevert_t	*ptriv;
			int bone;

			vec3_t vert[3];
			vec3_t norm[3];

			linecount++;

			// check for end
			if (strcmp( "end\n", line ) == 0) 
				return;

			// strip off trailing smag
			strcpy( texturename, line );
			for (i = strlen( texturename ) - 1; i >= 0 && ! isgraph( texturename[i] ); i--) ;
			texturename[i + 1] = '\0';

			// funky texture overrides
			for (i = 0; i < numrep; i++)  
			{
				if (sourcetexture[i][0] == '\0') 
				{
					strcpy( texturename, defaulttexture[i] );
					break;
				}
				if (stricmp( texturename, sourcetexture[i]) == 0) 
				{
					strcpy( texturename, defaulttexture[i] );
					break;
				}
			}

			if (texturename[0] == '\0')
			{
				// weird model problem, skip them
				fgets( line, sizeof( line ), input );
				fgets( line, sizeof( line ), input );
				fgets( line, sizeof( line ), input );
				linecount += 3;
				continue;
			}

			pmesh = lookup_mesh( pmodel, texturename );

			for (j = 0; j < 3; j++) 
			{
				if (flip_triangles)
					// quake wants them in the reverse order
					ptriv = lookup_triangle( pmesh, pmesh->numtris ) + 2 - j;
				else
					ptriv = lookup_triangle( pmesh, pmesh->numtris ) + j;

				if (fgets( line, sizeof( line ), input ) != NULL) 
				{
					s_vertex_t p;
					vec3_t tmp;
					s_normal_t normal;

					linecount++;
					if (sscanf( line, "%d %f %f %f %f %f %f %f %f",
						&bone, 
						&p.org[0], &p.org[1], &p.org[2], 
						&normal.org[0], &normal.org[1], &normal.org[2], 
						&ptriv->u, &ptriv->v ) == 9) 
					{
						if (bone < 0 || bone >= pmodel->numbones) 
						{
							fprintf( stderr, "bogus bone index\n" );
							fprintf(stderr, "%d %s :\n%s", linecount, filename, line );
							exit(1);
						}

						/*
						if (ptriv->u > 2.0)
						{
							printf("%d %f\n", linecount, ptriv->u );
						}
						*/

						VectorCopy( p.org, vert[j] );
						VectorCopy( normal.org, norm[j] );

						p.bone = bone;
						normal.bone = bone;
						normal.skinref = pmesh->skinref;

						if (p.org[2] < vmin[2]) vmin[2] = p.org[2];

						adjust_vertex( p.org );
						scale_vertex( p.org );

						// move vertex position to object space.
						VectorSubtract( p.org, bonefixup[p.bone].worldorg, tmp );
						VectorTransform(tmp, bonefixup[p.bone].im, p.org );

						// move normal to object space.
						VectorCopy( normal.org, tmp );
						VectorTransform(tmp, bonefixup[p.bone].im, normal.org );
						VectorNormalize( normal.org );

						ptriv->normindex = lookup_normal( pmodel, &normal );
						ptriv->vertindex = lookup_vertex( pmodel, &p );

						// tag bone as being used
						// pmodel->bone[bone].ref = 1;
					}
					else 
					{
						Error("%s: error on line %d: %s", filename, linecount, line );
					}
				}
			}

			if (tag_reversed || tag_normals)
			{
				// check triangle direction
				
				if (DotProduct( norm[0], norm[1] ) < 0.0 
					|| DotProduct( norm[1], norm[2] ) < 0.0
					|| DotProduct( norm[2], norm[0] ) < 0.0 ) {

					ncount++;

					if (tag_normals) 
					{
						// steal the triangle and make it white
						s_trianglevert_t *ptriv2;
						pmesh = lookup_mesh( pmodel, "..\\white.bmp" );
						ptriv2 = lookup_triangle( pmesh, pmesh->numtris );

						ptriv2[0] = ptriv[0];
						ptriv2[1] = ptriv[1];
						ptriv2[2] = ptriv[2];
					}
				} 
				else 
				{
					vec3_t a1, a2, sn;
					float x, y, z;

					VectorSubtract( vert[1], vert[0], a1 );
					VectorSubtract( vert[2], vert[0], a2 );
					CrossProduct( a1, a2, sn );
					VectorNormalize( sn );

					x = DotProduct( sn, norm[0] );
					y = DotProduct( sn, norm[1] );
					z = DotProduct( sn, norm[2] );
					if (x < 0.0 || y < 0.0 || z < 0.0) 
					{
						if (tag_reversed) 
						{
							// steal the triangle and make it white
							s_trianglevert_t *ptriv2;

							printf("triangle reversed (%f %f %f)\n",
								DotProduct( norm[0], norm[1] ),
								DotProduct( norm[1], norm[2] ),
								DotProduct( norm[2], norm[0] ) );

							pmesh = lookup_mesh( pmodel, "..\\white.bmp" );
							ptriv2 = lookup_triangle( pmesh, pmesh->numtris );

							ptriv2[0] = ptriv[0];
							ptriv2[1] = ptriv[1];
							ptriv2[2] = ptriv[2];
						}
					}
				}
			}

			pmodel->trimesh[tcount] = pmesh;
			pmodel->trimap[tcount] = pmesh->numtris++;
			tcount++;
		}
		else {
			break;
		}
	}

	
	if (ncount) printf("%d triangles with misdirected normals\n", ncount );

	if (vmin[2] != 0.0) 
	{
		printf("lowest vector at %f\n", vmin[2] );
	}
}


void Grab_Skeleton( s_node_t *pnodes, s_bone_t *pbones )
{
	float x, y, z, xr, yr, zr;
	char cmd[1024];
	int index;

	while (fgets( line, sizeof( line ), input ) != NULL) 
	{
		linecount++;
		if (sscanf( line, "%d %f %f %f %f %f %f", &index, &x, &y, &z, &xr, &yr, &zr ) == 7)
		{
			pbones[index].pos[0] = x;
			pbones[index].pos[1] = y;
			pbones[index].pos[2] = z;

			scale_vertex( pbones[index].pos );

			if (pnodes[index].mirrored)
				VectorScale( pbones[index].pos, -1.0, pbones[index].pos );
			
			pbones[index].rot[0] = xr;
			pbones[index].rot[1] = yr;
			pbones[index].rot[2] = zr;

			clip_rotations( pbones[index].rot ); 
		}
		else if (sscanf( line, "%s %d", cmd, &index ))
		{
			if (strcmp( cmd, "time" ) == 0) 
			{
				// pbones = pnode->bones[index] = kalloc(1, sizeof( s_bones_t ));
			}
			else if (strcmp( cmd, "end") == 0) 
			{
				return;
			}
		}
	}
}


int Grab_Nodes( s_node_t *pnodes )
{
	int index;
	char name[1024];
	int parent;
	int numbones = 0;
	int i;

	while (fgets( line, sizeof( line ), input ) != NULL) 
	{
		linecount++;
		if (sscanf( line, "%d \"%[^\"]\" %d", &index, name, &parent ) == 3)
		{
			// check for duplicated bones
			/*
			if (strlen(pnodes[index].name) != 0)
			{
				Error( "bone \"%s\" exists more than once\n", name );
			}
			*/
			
			strcpyn( pnodes[index].name, name );
			pnodes[index].parent = parent;
			numbones = index;
			// check for mirrored bones;
			for (i = 0; i < nummirrored; i++)
			{
				if (strcmp( name, mirrored[i] ) == 0)
					pnodes[index].mirrored = 1;
			}
			if ((! pnodes[index].mirrored) && parent != -1)
			{
				pnodes[index].mirrored = pnodes[pnodes[index].parent].mirrored;
			}
		}
		else 
		{
			return numbones + 1;
		}
	}
	Error( "Unexpected EOF at line %d\n", linecount );
	return 0;
}



void Grab_Studio ( s_model_t *pmodel )
{
	int		time1;
	char	cmd[1024];
	int		option;

	sprintf (filename, "%s/%s.smd", cddir, pmodel->name);
	time1 = FileTime (filename);
	if (time1 == -1)
		Error ("%s doesn't exist", filename);

	printf ("grabbing %s\n", filename);

	if ((input = fopen(filename, "r")) == 0) {
		fprintf(stderr,"reader: could not open file '%s'\n", filename);
	}
	linecount = 0;

	while (fgets( line, sizeof( line ), input ) != NULL) {
		linecount++;
		sscanf( line, "%s %d", cmd, &option );
		if (strcmp( cmd, "version" ) == 0) {
			if (option != 1) {
				Error("bad version\n");
			}
		}
		else if (strcmp( cmd, "nodes" ) == 0) {
			pmodel->numbones = Grab_Nodes( pmodel->node );
		}
		else if (strcmp( cmd, "skeleton" ) == 0) {
			Grab_Skeleton( pmodel->node, pmodel->skeleton );
		}
		else if (strcmp( cmd, "triangles" ) == 0) {
			Grab_Triangles( pmodel );
		}
		else 
		{
			printf("unknown studio command\n" );
		}
	}
	fclose( input );
}








void clip_rotations( vec3_t rot )
{
	int j;
	// clip everything to : -Q_PI <= x < Q_PI

	for (j = 0; j < 3; j++) {
		while (rot[j] >= Q_PI) 
			rot[j] -= Q_PI*2;
		while (rot[j] < -Q_PI) 
			rot[j] += Q_PI*2;
	}
}




/*
=================
Cmd_Eyeposition
=================
*/
void Cmd_Eyeposition (void)
{
// rotate points into frame of reference so model points down the positive x
// axis
	GetToken (false);
	eyeposition[1] = atof (token);

	GetToken (false);
	eyeposition[0] = -atof (token);

	GetToken (false);
	eyeposition[2] = atof (token);
}


/*
=================
Cmd_Flags
=================
*/
void Cmd_Flags (void)
{
	GetToken (false);
	gflags = atoi (token);
}


/*
=================
Cmd_Modelname
=================
*/
void Cmd_Modelname (void)
{
	GetToken (false);
	strcpyn (outname, token);
}


/*
===============
===============
*/


void Option_Studio( )
{
	if (!GetToken (false)) return;

	model[nummodels] = kalloc( 1, sizeof( s_model_t ) );
	bodypart[numbodyparts].pmodel[bodypart[numbodyparts].nummodels] = model[nummodels];

	strcpyn( model[nummodels]->name, token );

	flip_triangles = 1;

	scale_up = default_scale;

	while (TokenAvailable())
	{
		GetToken(false);
		if (stricmp( "reverse", token ) == 0)
		{
			flip_triangles = 0;
		}
		else if (stricmp( "scale", token ) == 0)
		{
			GetToken(false);
			scale_up = atof( token );
		}
	}

	Grab_Studio( model[nummodels] );

	bodypart[numbodyparts].nummodels++;
	nummodels++;

	scale_up = default_scale;
}


int Option_Blank( )
{
	model[nummodels] = kalloc( 1, sizeof( s_model_t ) );
	bodypart[numbodyparts].pmodel[bodypart[numbodyparts].nummodels] = model[nummodels];

	strcpyn( model[nummodels]->name, "blank" );

	bodypart[numbodyparts].nummodels++;
	nummodels++;
	return 0;
}


void Cmd_Bodygroup( )
{
	int is_started = 0;

	if (!GetToken(false)) return;

	if (numbodyparts == 0) {
		bodypart[numbodyparts].base = 1;
	}
	else {
		bodypart[numbodyparts].base = bodypart[numbodyparts-1].base * bodypart[numbodyparts-1].nummodels;
	}
	strcpyn( bodypart[numbodyparts].name, token );

	do
	{
		GetToken (true);
		if (endofscript)
			return;
		else if (token[0] == '{')
			is_started = 1;
		else if (token[0] == '}')
			break;
		else if (stricmp("studio", token ) == 0)
			Option_Studio( );
		else if (stricmp("blank", token ) == 0)
			Option_Blank( );
	} while (1);

	numbodyparts++;
	return;
}


void Cmd_Body( )
{
	int is_started = 0;

	if (!GetToken(false)) return;

	if (numbodyparts == 0) {
		bodypart[numbodyparts].base = 1;
	}
	else {
		bodypart[numbodyparts].base = bodypart[numbodyparts-1].base * bodypart[numbodyparts-1].nummodels;
	}
	strcpyn(bodypart[numbodyparts].name, token );

	Option_Studio( );

	numbodyparts++;
}



/*
===============
===============
*/

void Grab_Animation( s_animation_t *panim)
{
	vec3_t pos;
	vec3_t rot;
	char cmd[1024];
	int index;
	int	t = -99999999;
	float cz, sz;
	int start = 99999;
	int end = 0;

	for (index = 0; index < panim->numbones; index++) 
	{
		panim->pos[index] = kalloc( MAXSTUDIOANIMATIONS, sizeof( vec3_t ) );
		panim->rot[index] = kalloc( MAXSTUDIOANIMATIONS, sizeof( vec3_t ) );
	}

	cz = cos( zrotation );
	sz = sin( zrotation );

	while (fgets( line, sizeof( line ), input ) != NULL) 
	{
		linecount++;
		if (sscanf( line, "%d %f %f %f %f %f %f", &index, &pos[0], &pos[1], &pos[2], &rot[0], &rot[1], &rot[2] ) == 7)
		{
			if (t >= panim->startframe && t <= panim->endframe)
			{
				if (panim->node[index].parent == -1) {
					adjust_vertex( pos );
					panim->pos[index][t][0] = cz * pos[0] - sz * pos[1];
					panim->pos[index][t][1] = sz * pos[0] + cz * pos[1];
					panim->pos[index][t][2] = pos[2];
					// rotate model
					rot[2]			+= zrotation;
				}
				else
				{
					VectorCopy( pos, panim->pos[index][t] );
				}
				if (t > end)
					end = t;
				if (t < start)
					start = t;

				if (panim->node[index].mirrored)
					VectorScale( panim->pos[index][t], -1.0, panim->pos[index][t] );

				scale_vertex( panim->pos[index][t] );

				clip_rotations( rot );

				VectorCopy( rot, panim->rot[index][t] );
			}
		}
		else if (sscanf( line, "%s %d", cmd, &index ))
		{
			if (strcmp( cmd, "time" ) == 0) 
			{
				t = index;
			}
			else if (strcmp( cmd, "end") == 0) 
			{
				panim->startframe = start;
				panim->endframe = end;
				return;
			}
			else
			{
				Error( "Error(%d) : %s", linecount, line );
			}
		}
		else
		{
			Error( "Error(%d) : %s", linecount, line );
		}
	}
	Error( "unexpected EOF: %s\n", panim->name );
}


void Shift_Animation( s_animation_t *panim)
{
	int j;

	int size;

	size = (panim->endframe - panim->startframe + 1) * sizeof( vec3_t );
	// shift
	for (j = 0; j < panim->numbones; j++)
	{
		vec3_t *ppos;
		vec3_t *prot;

		k_memtotal -= MAXSTUDIOANIMATIONS * sizeof( vec3_t ) * 2;
		k_memtotal += size * 2;

		ppos = kalloc( 1, size );
		prot = kalloc( 1, size );

		memmove( ppos, &panim->pos[j][panim->startframe], size );
		memmove( prot, &panim->rot[j][panim->startframe], size );

		free( panim->pos[j] );
		free( panim->rot[j] );

		panim->pos[j] = ppos;
		panim->rot[j] = prot;
	}
}



void Option_Animation ( char *name, s_animation_t *panim )
{
	int		time1;
	char	cmd[1024];
	int		option;

	strcpyn( panim->name, name );

	sprintf (filename, "%s/%s.smd", cddir, panim->name);
	time1 = FileTime (filename);
	if (time1 == -1)
		Error ("%s doesn't exist", filename);

	printf ("grabbing %s\n", filename);

	if ((input = fopen(filename, "r")) == 0) {
		fprintf(stderr,"reader: could not open file '%s'\n", filename);
		Error(0);
	}
	linecount = 0;

	while (fgets( line, sizeof( line ), input ) != NULL) {
		linecount++;
		sscanf( line, "%s %d", cmd, &option );
		if (strcmp( cmd, "version" ) == 0) {
			if (option != 1) {
				Error("bad version\n");
			}
		}
		else if (strcmp( cmd, "nodes" ) == 0) {
			panim->numbones = Grab_Nodes( panim->node );
		}
		else if (strcmp( cmd, "skeleton" ) == 0) {
			Grab_Animation( panim );
			Shift_Animation( panim );
		}
		else 
		{
			printf("unknown studio command : %s\n", cmd );
			while (fgets( line, sizeof( line ), input ) != NULL) {
				linecount++;
				if (strncmp(line,"end",3)==0)
					break;
			}
		}
	}
	fclose( input );
}


int Option_Deform ( s_sequence_t *psequence )
{
	return 0;
}




/*
===============
===============
*/


int Option_Motion ( s_sequence_t *psequence )
{
	while (TokenAvailable())
	{
		GetToken (false);
		psequence->motiontype |= lookupControl( token );
	}
	return 0;
}


int Option_Event ( s_sequence_t *psequence )
{
	int event;

	if (psequence->numevents + 1 >= MAXSTUDIOEVENTS)
	{
		printf("too many events\n");
		exit(0);
	}

	GetToken (false);
	event = atoi( token );
	psequence->event[psequence->numevents].event = event;

	GetToken( false );
	psequence->event[psequence->numevents].frame = atoi( token );

	psequence->numevents++;

	// option token
	if (TokenAvailable())
	{
		GetToken( false );
		if (token[0] == '}') // opps, hit the end
			return 1;
		// found an option
		strcpy( psequence->event[psequence->numevents-1].options, token );
	}

	return 0;
}


int Option_Fps ( s_sequence_t *psequence )
{
	GetToken (false);

	psequence->fps = atof( token );

	return 0;
}

int Option_AddPivot ( s_sequence_t *psequence )
{
	if (psequence->numpivots + 1 >= MAXSTUDIOPIVOTS)
	{
		printf("too many pivot points\n");
		exit(0);
	}

	
	GetToken (false);
	psequence->pivot[psequence->numpivots].index = atoi( token );

	GetToken (false);
	psequence->pivot[psequence->numpivots].start = atoi( token );

	GetToken (false);
	psequence->pivot[psequence->numpivots].end = atoi( token );

	psequence->numpivots++;

	return 0;
}



/*
=================
Option_Origin
=================
*/
void Cmd_Origin (void)
{
	GetToken (false);
	defaultadjust[0] = atof (token);

	GetToken (false);
	defaultadjust[1] = atof (token);

	GetToken (false);
	defaultadjust[2] = atof (token);

	if (TokenAvailable()) {
		GetToken (false);
		defaultzrotation = (atof( token ) + 90) * (Q_PI / 180.0);
	}
}


void Option_Origin (void)
{
	GetToken (false);
	adjust[0] = atof (token);

	GetToken (false);
	adjust[1] = atof (token);

	GetToken (false);
	adjust[2] = atof (token);
}

void Option_Rotate(void )
{
	GetToken (false);
	zrotation = (atof( token ) + 90) * (Q_PI / 180.0);
}

/*
=================
=================
*/
void Cmd_ScaleUp (void)
{

	GetToken (false);
	default_scale = scale_up = atof (token);
}

void Option_ScaleUp (void)
{

	GetToken (false);
	scale_up = atof (token);
}


/*
=================
=================
*/

int Cmd_SequenceGroup( )
{
	GetToken (false);
	strcpyn( sequencegroup[numseqgroups].label, token );
	numseqgroups++;

	return 0;
}


int Cmd_SequenceGroupSize( )
{
	GetToken (false);
	maxseqgroupsize = 1024 * atoi( token );
	return 0;
}

int lookupActivity( char *szActivity )
{
	int i;

	for (i = 0; activity_map[i].name; i++)
	{
		if (stricmp( szActivity, activity_map[i].name ) == 0)
			return activity_map[i].type;
	}
	// match ACT_#
	if (strnicmp( szActivity, "ACT_", 4 ) == 0)
	{
		return atoi( &szActivity[4] );
	}
	return 0;
}


int Cmd_Sequence( )
{
	int depth = 0;
	char smdfilename[MAXSTUDIOGROUPS][1024];
	int i;
	int numblends = 0;
	int start = 0;
	int end = MAXSTUDIOANIMATIONS - 1;

	if (!GetToken(false)) return 0;

	strcpyn( sequence[numseq].name, token );

	VectorCopy( defaultadjust, adjust );
	scale_up = default_scale;

	zrotation = defaultzrotation;
	sequence[numseq].fps = 30.0;
	sequence[numseq].seqgroup = numseqgroups - 1;
	sequence[numseq].blendstart[0] = 0.0;
	sequence[numseq].blendend[0] = 1.0;

	while (1)
	{
		if (depth > 0)
		{
			if(!GetToken(true)) 
			{
				break;
			}
		}
		else
		{
			if (!TokenAvailable()) 
			{
				break;
			}
			else 
			{
				GetToken (false);
			}
		}

		if (endofscript)
		{
			if (depth != 0)
			{
				printf("missing }\n" );
				exit(1);
			}
			return 1;
		}
		if (stricmp("{", token ) == 0)
		{
			depth++;
		}
		else if (stricmp("}", token ) == 0)
		{
			depth--;
		}
		else if (stricmp("deform", token ) == 0)
		{
			Option_Deform( &sequence[numseq] );
		}
		else if (stricmp("event", token ) == 0)
		{
			depth -= Option_Event( &sequence[numseq] );
		}
		else if (stricmp("pivot", token ) == 0)
		{
			Option_AddPivot( &sequence[numseq] );
		}
		else if (stricmp("fps", token ) == 0)
		{
			Option_Fps( &sequence[numseq] );
		}
		else if (stricmp("origin", token ) == 0)
		{
			Option_Origin( );
		}
		else if (stricmp("rotate", token ) == 0)
		{
			Option_Rotate( );
		}
		else if (stricmp("scale", token ) == 0)
		{
			Option_ScaleUp( );
		}
		else if (strnicmp("loop", token, 4 ) == 0)
		{
			sequence[numseq].flags |= STUDIO_LOOPING;
		}
		else if (strnicmp("frame", token, 5 ) == 0)
		{
			GetToken( false );
			start = atoi( token );
			GetToken( false );
			end = atoi( token );
		}
		else if (strnicmp("blend", token, 5 ) == 0)
		{
			GetToken( false );
			sequence[numseq].blendtype[0] = lookupControl( token );
			GetToken( false );
			sequence[numseq].blendstart[0] = atof( token );
			GetToken( false );
			sequence[numseq].blendend[0] = atof( token );
		}
		else if (strnicmp("node", token, 4 ) == 0)
		{
			GetToken( false );
			sequence[numseq].entrynode = sequence[numseq].exitnode = atoi( token );
		}
		else if (strnicmp("transition", token, 4 ) == 0)
		{
			GetToken( false );
			sequence[numseq].entrynode = atoi( token );
			GetToken( false );
			sequence[numseq].exitnode = atoi( token );
		}
		else if (strnicmp("rtransition", token, 4 ) == 0)
		{
			GetToken( false );
			sequence[numseq].entrynode = atoi( token );
			GetToken( false );
			sequence[numseq].exitnode = atoi( token );
			sequence[numseq].nodeflags |= 1;
		}
		else if (lookupControl( token ) != -1)
		{
			sequence[numseq].motiontype |= lookupControl( token );
		}
		else if (stricmp("animation", token ) == 0)
		{
			GetToken(false);
			strcpyn( smdfilename[numblends++], token );
		}
		else if ((i = lookupActivity( token )) != 0)
		{
			sequence[numseq].activity = i;
			GetToken( false );
			sequence[numseq].actweight = atoi( token );
		}
		else
		{
			strcpyn( smdfilename[numblends++], token );
		}

		if (depth < 0)
		{
			printf("missing {\n");
			exit(1);
		}
	};

	if (numblends == 0)
	{
		printf("no animations found\n");
		exit(1);
	}
	for (i = 0; i < numblends; i++)
	{
		panimation[numani] = kalloc( 1, sizeof( s_animation_t ) );
		sequence[numseq].panim[i] = panimation[numani];
		sequence[numseq].panim[i]->startframe = start;
		sequence[numseq].panim[i]->endframe = end;
		sequence[numseq].panim[i]->flags = 0;
		Option_Animation( smdfilename[i], panimation[numani] );
		numani++;
	}
	sequence[numseq].numblends = numblends;

	numseq++;

	return 0;
}






/*
=================
=================
*/
int Cmd_Root (void)
{
	if (GetToken (false))
	{
		strcpyn( pivotname[0], token );
		return 0;
	}
	return 1;
}

int Cmd_Pivot (void)
{
	if (GetToken (false))
	{
		int index = atoi(token);
		if (GetToken(false))
		{
			strcpyn( pivotname[index], token );
			return 0;
		}
	}
	return 1;
}


int Cmd_Controller (void)
{
	if (GetToken (false))
	{
		if (!strcmpi("mouth",token))
		{
			bonecontroller[numbonecontrollers].index = 4;
		}
		else
		{
			bonecontroller[numbonecontrollers].index = atoi(token);
		}
		if (GetToken(false))
		{
			strcpyn( bonecontroller[numbonecontrollers].name, token );
			GetToken(false);
			if ((bonecontroller[numbonecontrollers].type = lookupControl(token)) == -1) 
			{
				printf("unknown bonecontroller type '%s'\n", token );
				return 0;
			}
			GetToken(false);
			bonecontroller[numbonecontrollers].start = atof( token );
			GetToken(false);
			bonecontroller[numbonecontrollers].end = atof( token );

			if (bonecontroller[numbonecontrollers].type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
			{
				if (((int)(bonecontroller[numbonecontrollers].start + 360) % 360) == ((int)(bonecontroller[numbonecontrollers].end + 360) % 360))
				{
					bonecontroller[numbonecontrollers].type |= STUDIO_RLOOP;
				}
			}
			numbonecontrollers++;
		}
	}
	return 1;
}


/*
=================
=================
*/
void Cmd_BBox (void)
{
	GetToken (false);
	bbox[0][0] = atof( token );

	GetToken (false);
	bbox[0][1] = atof( token );

	GetToken (false);
	bbox[0][2] = atof( token );

	GetToken (false);
	bbox[1][0] = atof( token );

	GetToken (false);
	bbox[1][1] = atof( token );

	GetToken (false);
	bbox[1][2] = atof( token );
}

/*
=================
=================
*/
void Cmd_CBox (void)
{
	GetToken (false);
	cbox[0][0] = atof( token );

	GetToken (false);
	cbox[0][1] = atof( token );

	GetToken (false);
	cbox[0][2] = atof( token );

	GetToken (false);
	cbox[1][0] = atof( token );

	GetToken (false);
	cbox[1][1] = atof( token );

	GetToken (false);
	cbox[1][2] = atof( token );
}


/*
=================
=================
*/
void Cmd_Mirror (void)
{
	GetToken (false);
	strcpyn( mirrored[nummirrored++], token );
}

/*
=================
=================
*/
void Cmd_Gamma (void)
{
	GetToken (false);
	gamma = atof( token );
}





/*
=================
=================
*/
int Cmd_TextureGroup( )
{
	int i;
	int depth = 0;
	int index = 0;
	int group = 0;

	if (numtextures == 0)
		Error( "texturegroups must follow model loading\n");

	if (!GetToken(false)) return 0;

	if (numskinref == 0)
		numskinref = numtextures;

	while (1)
	{
		if(!GetToken(true)) 
		{
			break;
		}

		if (endofscript)
		{
			if (depth != 0)
			{
				Error("missing }\n" );
			}
			return 1;
		}
		if (token[0] == '{')
		{
			depth++;
		}
		else if (token[0] == '}')
		{
			depth--;
			if (depth == 0)
				break;
			group++;
			index = 0;
		}
		else if (depth == 2)
		{
			i = lookup_texture( token );
			texturegroup[numtexturegroups][group][index] = i;
			if (group != 0)
				texture[i].parent = texturegroup[numtexturegroups][0][index];
			index++;
			numtexturereps[numtexturegroups] = index;
			numtexturelayers[numtexturegroups] = group + 1;
		}
	}

	numtexturegroups++;

	return 0;
}



/*
=================
=================
*/
int Cmd_Hitgroup( )
{
	GetToken (false);
	hitgroup[numhitgroups].group = atoi( token );
	GetToken (false);
	strcpyn( hitgroup[numhitgroups].name, token );
	numhitgroups++;

	return 0;
}


int Cmd_Hitbox( )
{
	GetToken (false);
	hitbox[numhitboxes].group = atoi( token );
	GetToken (false);
	strcpyn( hitbox[numhitboxes].name, token );
	GetToken (false);
	hitbox[numhitboxes].bmin[0] = atof( token );
	GetToken (false);
	hitbox[numhitboxes].bmin[1] = atof( token );
	GetToken (false);
	hitbox[numhitboxes].bmin[2] = atof( token );
	GetToken (false);
	hitbox[numhitboxes].bmax[0] = atof( token );
	GetToken (false);
	hitbox[numhitboxes].bmax[1] = atof( token );
	GetToken (false);
	hitbox[numhitboxes].bmax[2] = atof( token );

	numhitboxes++;

	return 0;
}


/*
=================
=================
*/
int Cmd_Attachment( )
{
	// index
	GetToken (false);
	attachment[numattachments].index = atoi( token );

	// bone name
	GetToken (false);
	strcpyn( attachment[numattachments].bonename, token );

	// position
	GetToken (false);
	attachment[numattachments].org[0] = atof( token );
	GetToken (false);
	attachment[numattachments].org[1] = atof( token );
	GetToken (false);
	attachment[numattachments].org[2] = atof( token );

	if (TokenAvailable())
		GetToken (false);

	if (TokenAvailable())
		GetToken (false);

	numattachments++;
	return 0;
}



/*
=================
=================
*/
void Cmd_Renamebone( )
{
	// from
	GetToken (false);
	strcpy( renamedbone[numrenamedbones].from, token );

	// to
	GetToken (false);
	strcpy( renamedbone[numrenamedbones].to, token );

	numrenamedbones++;
}

/*add transparent texture support to models
===================
Cmd_SetTextureRendermode

//paramaters:
  	 // "texturename" "rendermode" renderamt
  	// acceptable strings for rendermode are:
  	// "alpha"
  	// "additive"
  	// "masked"

===================
*/
void Cmd_SetTextureRendermode( void )
{
	int iTextureIndex;

	if(!TokenAvailable())
	{
  		printf("*********ERROR!!!*************");
  		printf("\nmissing texturename after $texrendermode\n");
  		exit(1);
	}

	GetToken(false);

	iTextureIndex = lookup_texture(token);

	if(!TokenAvailable())
	{
  		printf("\n*********ERROR!!!*************\n");
  		printf("\nmissing rendermode at $texrendermode\n");
  		exit(1);
	}

	GetToken(false);

	if(!strcmp(token, "additive"))
	{
		texture[iTextureIndex].flags |= STUDIO_NF_ADDITIVE;
		return;
	}
	else if(!strcmp(token, "masked"))
	{
  		texture[iTextureIndex].flags |= STUDIO_NF_MASKED;
  		return;
	}
	else
	{
  		printf("\n*********ERROR!!!*************\n");
  		printf("\ninvalid rendermode at $texrendermode, choices are :\nadditive\nmasked\n");
  		exit(1);
	}
}


/*
===============
ParseScript
===============
*/
void ParseScript (void)
{
	while (1)
	{
		do
		{	// look for a line starting with a $ command
			GetToken (true);
			if (endofscript)
				return;
			if (token[0] == '$')
				break;
			while (TokenAvailable())
				GetToken (false);
		} while (1);
	
		if (!strcmp (token, "$modelname"))
		{
			Cmd_Modelname ();
		}
		else if (!strcmp (token, "$cd"))
		{
			if (cdset)
				Error ("Two $cd in one model");
			cdset = true;
			GetToken (false);
			strcpy (cdpartial, token);
			strcpy (cddir, ExpandPath(token));
		}
		else if (!strcmp (token, "$cdtexture"))
		{
			while (TokenAvailable())
			{
				GetToken (false);
				strcpy (cdtexture[cdtextureset], ExpandPath(token));
				cdtextureset++;
			}
		}

		else if (!strcmp (token, "$scale"))
		{
			Cmd_ScaleUp ();
		}

		else if (!strcmp (token, "$root"))
		{
			Cmd_Root ();
		}
		else if (!strcmp (token, "$pivot"))
		{
			Cmd_Pivot ();
		}
		else if (!strcmp (token, "$controller"))
		{
			Cmd_Controller ();
		}


		else if (!strcmp (token, "$body"))
		{
			Cmd_Body();
		}

		else if (!strcmp (token, "$bodygroup"))
		{
			Cmd_Bodygroup();
		}

		else if (!strcmp (token, "$sequence"))
		{
			Cmd_Sequence ();
		}

		else if (!strcmp (token, "$sequencegroup"))
		{
			Cmd_SequenceGroup ();
		}

		else if (!strcmp (token, "$sequencegroupsize"))
		{
			Cmd_SequenceGroupSize ();
		}

		else if (!strcmp (token, "$eyeposition"))
		{
			Cmd_Eyeposition ();
		}

		else if (!strcmp (token, "$origin"))
		{
			Cmd_Origin ();
		}

		else if (!strcmp (token, "$bbox"))
		{
			Cmd_BBox ();
		}
		else if (!strcmp (token, "$cbox"))
		{
			Cmd_CBox ();
		}
		else if (!strcmp (token, "$mirrorbone"))
		{
			Cmd_Mirror ();
		}
		else if (!strcmp (token, "$gamma"))
		{
			Cmd_Gamma ();
		}
		else if (!strcmp (token, "$flags"))
		{
			Cmd_Flags ();
		}
		else if (!strcmp (token, "$texturegroup"))
		{
			Cmd_TextureGroup ();
		}

		else if (!strcmp (token, "$hgroup"))
		{
			Cmd_Hitgroup ();
		}
		else if (!strcmp (token, "$hbox"))
		{
			Cmd_Hitbox ();
		}
		else if (!strcmp (token, "$attachment"))
		{
			Cmd_Attachment ();
		}
		else if (!strcmp (token, "$externaltextures"))
		{
			split_textures = 1;
		}
		else if (!strcmp (token, "$cliptotextures"))
		{
			clip_texcoords = 0;
		}
		else if (!strcmp (token, "$renamebone"))
		{
			Cmd_Renamebone ();
		}
  		else if (!strcmp (token, "$texrendermode"))
  		{
			Cmd_SetTextureRendermode();
  		}
  		else
		{
			Error ("bad command %s\n", token);
		}

	}
}

/*
==============
main
==============
*/
int main (int argc, char **argv)
{
	int		i;
	char	path[1024];

	default_scale = 1.0;
	defaultzrotation = Q_PI / 2;

	numrep = 0;
	tag_reversed = 0;
	tag_normals = 0;
	flip_triangles = 1;
	maxseqgroupsize = 1024 * 1024;

	normal_blend = cos( 2.0 * (Q_PI / 180.0));

	gamma = 1.8;

	if (argc == 1)
		Error ("usage: studiomdl [-t texture] -r(tag reversed) -n(tag bad normals) -f(flip all triangles) [-a normal_blend_angle] -h(dump hboxes) -i(ignore warnings) -p(force power of 2 textures) [-g max_sequencegroup_size(K)] file.qc");
		
	for (i = 1; i < argc - 1; i++) {
		if (argv[i][0] == '-') {
			switch( argv[i][1] ) {
			case 't':
				i++;
				strcpy ( defaulttexture[numrep], argv[i]);
				if (i < argc - 2 && argv[i + 1][0] != '-') {
					i++;
					strcpy ( sourcetexture[numrep], argv[i]);
					printf ("Replaceing %s with %s\n", sourcetexture[numrep], defaulttexture[numrep] );
				}
				printf ("Using default texture: %s\n", defaulttexture);
				numrep++;
				break;
			case 'r':
				tag_reversed = 1;
				break;
			case 'n':
				tag_normals = 1;
				break;
			case 'f':
				flip_triangles = 0;
				break;
			case 'a':
				i++;
				normal_blend = cos( atof( argv[i] ) * (Q_PI / 180.0));
				break;
			case 'h':
				dump_hboxes = 1;
				break;
			case 'g':
				i++;
				maxseqgroupsize = 1024 * atoi( argv[i] );
				break;
			case 'p':
			case '2':
				force_powerof2_textures = 1;
				break;
			case 'i':
				ignore_warnings = 1;
				break;
			}
		}
	}	

	strcpy( sequencegroup[numseqgroups].label, "default" );
	numseqgroups = 1;

//
// load the script
//
	strcpy (path, argv[i]);
	DefaultExtension (path, ".qc");
	// SetQdirFromPath (path);
	LoadScriptFile (path);
	
//
// parse it
//

	ClearModel ();
	strcpy (outname, argv[i]);

	ParseScript ();
	SetSkinValues ();
	SimplifyModel ();
	WriteFile ();

	return 0;
}




