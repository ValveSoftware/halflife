/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

/*


*/


#define STUDIO_VERSION	10

#define IDSTUDIOHEADER	(('T'<<24)+('S'<<16)+('D'<<8)+'I')
														// little-endian "IDST"
#define IDSTUDIOSEQHEADER	(('Q'<<24)+('S'<<16)+('D'<<8)+'I')
														// little-endian "IDSQ"

#ifndef EXTERN
#define EXTERN extern
#endif

EXTERN	char		outname[1024];
EXTERN  qboolean	cdset;
EXTERN	char		cdpartial[256];
EXTERN	char		cddir[256];
EXTERN	int			cdtextureset;
EXTERN	char		cdtexture[16][256];

EXTERN	char		pivotname[32][256];	// names of the pivot points

EXTERN	float		default_scale;
EXTERN	float		scale_up;
EXTERN  float		defaultzrotation;
EXTERN	float		zrotation;


EXTERN	char		defaulttexture[16][256];
EXTERN	char		sourcetexture[16][256];

EXTERN	int			numrep;

EXTERN	int			tag_reversed;
EXTERN	int			tag_normals;
EXTERN	int			flip_triangles;
EXTERN	float		normal_blend;
EXTERN	int			dump_hboxes;
EXTERN	int			ignore_warnings;

EXTERN	vec3_t		eyeposition;
EXTERN	int			gflags;
EXTERN	vec3_t		bbox[2];
EXTERN	vec3_t		cbox[2];

EXTERN	int			maxseqgroupsize;

EXTERN	int			split_textures;
EXTERN	int			clip_texcoords;

#define ROLL	2
#define PITCH	0
#define YAW		1


extern vec_t Q_rint (vec_t in);

extern void WriteFile (void);
void *kalloc( int num, int size );

typedef struct {
	int					vertindex;
	int					normindex;		// index into normal array
	int					s,t;
	float				u,v;
} s_trianglevert_t;

typedef struct 
{
	int					bone;		// bone transformation index
	vec3_t				org;		// original position
} s_vertex_t;

typedef struct 
{
	int					skinref;
	int					bone;		// bone transformation index
	vec3_t				org;		// original position
} s_normal_t;


//============================================================================


// dstudiobone_t bone[MAXSTUDIOBONES];
typedef struct 
{
	vec3_t	worldorg;
	float m[3][4];
	float im[3][4];
	float length;
} s_bonefixup_t;
EXTERN	s_bonefixup_t bonefixup[MAXSTUDIOSRCBONES];

int numbones;
typedef struct 
{
	char			name[32];	// bone name for symbolic links
	int		 		parent;		// parent bone
	int				bonecontroller;	// -1 == 0
	int				flags;		// X, Y, Z, XR, YR, ZR
	// short		value[6];	// default DoF values
	vec3_t			pos;		// default pos
	vec3_t			posscale;	// pos values scale
	vec3_t			rot;		// default pos
	vec3_t			rotscale;	// rotation values scale
	int				group;		// hitgroup
	vec3_t			bmin, bmax;	// bounding box
} s_bonetable_t;
EXTERN	s_bonetable_t bonetable[MAXSTUDIOSRCBONES];

int numrenamedbones;
typedef struct 
{
	char			from[32];
	char			to[32];
} s_renamebone_t;
EXTERN s_renamebone_t renamedbone[MAXSTUDIOSRCBONES];

int numhitboxes;
typedef struct
{
	char			name[32];	// bone name
	int				bone;
	int				group;		// hitgroup
	int				model;
	vec3_t			bmin, bmax;	// bounding box
} s_bbox_t;
EXTERN s_bbox_t hitbox[MAXSTUDIOSRCBONES];

int numhitgroups;
typedef struct
{
	int				models;
	int				group;
	char			name[32];	// bone name
} s_hitgroup_t;
EXTERN s_hitgroup_t hitgroup[MAXSTUDIOSRCBONES];


typedef struct 
{
	char	name[32];
	int		bone;
	int		type;
	int		index;
	float	start;
	float	end;
} s_bonecontroller_t;

s_bonecontroller_t bonecontroller[MAXSTUDIOSRCBONES];
int numbonecontrollers;

typedef struct 
{
	char	name[32];
	char	bonename[32];
	int		index;
	int		bone;
	int		type;
	vec3_t	org;
} s_attachment_t;

s_attachment_t attachment[MAXSTUDIOSRCBONES];
int numattachments;

typedef struct 
{
	char			name[64];
	int				parent;
	int				mirrored;
} s_node_t;

EXTERN char mirrored[MAXSTUDIOSRCBONES][64];
EXTERN int nummirrored;

EXTERN	int numani;
typedef struct 
{
	char			name[64];
	int				startframe;
	int				endframe;
	int				flags;
	int				numbones;
	s_node_t		node[MAXSTUDIOSRCBONES];
	int				bonemap[MAXSTUDIOSRCBONES];
	int				boneimap[MAXSTUDIOSRCBONES];
	vec3_t			*pos[MAXSTUDIOSRCBONES];
	vec3_t			*rot[MAXSTUDIOSRCBONES];
	int				numanim[MAXSTUDIOSRCBONES][6];
	mstudioanimvalue_t *anim[MAXSTUDIOSRCBONES][6];
} s_animation_t;
EXTERN	s_animation_t *panimation[MAXSTUDIOANIMATIONS];


typedef struct 
{
	int				event;
	int				frame;
	char			options[64];
} s_event_t;

typedef struct 
{
	int				index;
	vec3_t			org;
	int				start;
	int				end;
} s_pivot_t;

EXTERN	int numseq;
typedef struct 
{
	int				motiontype;
	vec3_t			linearmovement;

	char			name[64];
	int				flags;
	float			fps;
	int				numframes;

	int				activity;
	int				actweight;

	int				frameoffset; // used to adjust frame numbers

	int				numevents;
	s_event_t		event[MAXSTUDIOEVENTS];

	int				numpivots;
	s_pivot_t		pivot[MAXSTUDIOPIVOTS];

	int				numblends;
	s_animation_t	*panim[MAXSTUDIOGROUPS];
	float			blendtype[2];
	float			blendstart[2];
	float			blendend[2];

	vec3_t			automovepos[MAXSTUDIOANIMATIONS];
	vec3_t			automoveangle[MAXSTUDIOANIMATIONS];

	int				seqgroup;
	int				animindex;

	vec3_t 			bmin;
	vec3_t			bmax;

	int				entrynode;
	int				exitnode;
	int				nodeflags;
} s_sequence_t;
EXTERN	s_sequence_t sequence[MAXSTUDIOSEQUENCES];


EXTERN int numseqgroups;
typedef struct {
	char	label[32];
	char	name[64];
} s_sequencegroup_t;
EXTERN s_sequencegroup_t sequencegroup[MAXSTUDIOSEQUENCES];

EXTERN int numxnodes;
EXTERN int xnode[100][100];

typedef struct {
	byte r, g, b;
} rgb_t;
typedef struct {
	byte b, g, r, x;
} rgb2_t;

// FIXME: what about texture overrides inline with loading models

typedef struct 
{
	char	name[64];
	int		flags;
	int		srcwidth;
	int		srcheight;
	byte	*ppicture;
	rgb_t 	*ppal;

	float	max_s;
	float   min_s;
	float	max_t;
	float	min_t;
	int		skintop;
	int		skinleft;
	int		skinwidth;
	int		skinheight;
	float	fskintop;
	float	fskinleft;
	float	fskinwidth;
	float	fskinheight;

	int		size;
	void	*pdata;

	int		parent;
} s_texture_t;
EXTERN	s_texture_t texture[MAXSTUDIOSKINS];
EXTERN	int numtextures;
EXTERN  float gamma;
EXTERN	int numskinref;
EXTERN  int numskinfamilies;
EXTERN  int skinref[256][MAXSTUDIOSKINS]; // [skin][skinref], returns texture index
EXTERN	int numtexturegroups;
EXTERN	int numtexturelayers[32];
EXTERN	int numtexturereps[32];
EXTERN  int texturegroup[32][32][32];

typedef struct 
{
	int alloctris;
	int numtris;
	s_trianglevert_t (*triangle)[3];

	int skinref;
	int numnorms;
} s_mesh_t;


typedef struct 
{
	vec3_t			pos;
	vec3_t			rot;
} s_bone_t;


typedef struct s_model_s 
{
	char name[64];

	int numbones;
	s_node_t node[MAXSTUDIOSRCBONES];
	s_bone_t skeleton[MAXSTUDIOSRCBONES];
	int boneref[MAXSTUDIOSRCBONES]; // is local bone (or child) referenced with a vertex
	int	bonemap[MAXSTUDIOSRCBONES]; // local bone to world bone mapping
	int	boneimap[MAXSTUDIOSRCBONES]; // world bone to local bone mapping

	vec3_t boundingbox[MAXSTUDIOSRCBONES][2];

	s_mesh_t *trimesh[MAXSTUDIOTRIANGLES];
	int trimap[MAXSTUDIOTRIANGLES];

	int numverts;
	s_vertex_t vert[MAXSTUDIOVERTS];

	int numnorms;
	s_normal_t normal[MAXSTUDIOVERTS];

	int nummesh;
	s_mesh_t *pmesh[MAXSTUDIOMESHES];

	float boundingradius;

	int numframes;
	float interval;
	struct s_model_s *next;
} s_model_t;

EXTERN	int nummodels;
EXTERN	s_model_t *model[MAXSTUDIOMODELS];



EXTERN	vec3_t adjust;
EXTERN	vec3_t defaultadjust;

typedef struct
{
	char				name[32];
	int					nummodels;
	int					base;
	s_model_t			*pmodel[MAXSTUDIOMODELS];
} s_bodypart_t;

EXTERN	int numbodyparts;
EXTERN	s_bodypart_t bodypart[MAXSTUDIOBODYPARTS];


extern int BuildTris (s_trianglevert_t (*x)[3], s_mesh_t *y, byte **ppdata );







