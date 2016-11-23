/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "qrad.h"

#define	HALFBIT

extern char		source[MAX_PATH];
extern char		vismatfile[_MAX_PATH];
extern char		incrementfile[_MAX_PATH];
extern qboolean	incremental;

/*
===================================================================

VISIBILITY MATRIX

Determine which patches can see each other
Use the PVS to accelerate if available
===================================================================
*/
byte	*vismatrix;

dleaf_t		*PointInLeaf (vec3_t point)
{
	int		nodenum;
	vec_t	dist;
	dnode_t	*node;
	dplane_t	*plane;

	nodenum = 0;
	while (nodenum >= 0)
	{
		node = &dnodes[nodenum];
		plane = &dplanes[node->planenum];
		dist = DotProduct (point, plane->normal) - plane->dist;
		if (dist > 0)
			nodenum = node->children[0];
		else
			nodenum = node->children[1];
	}

	return &dleafs[-nodenum - 1];
}


void PvsForOrigin (vec3_t org, byte *pvs)
{
	dleaf_t	*leaf;

	if (!visdatasize)
	{
		memset (pvs, 255, (numleafs+7)/8 );
		return;
	}

	leaf = PointInLeaf (org);
	if (leaf->visofs == -1)
		Error ("leaf->visofs == -1");

	DecompressVis (&dvisdata[leaf->visofs], pvs);
}


/*
==============
PatchPlaneDist

Fixes up patch planes for brush models with an origin brush
==============
*/
vec_t PatchPlaneDist( patch_t *patch )
{
	return patch->plane->dist + DotProduct( face_offset[ patch->faceNumber ], patch->normal );
}



/*
==============
TestPatchToFace

Sets vis bits for all patches in the face
==============
*/
void TestPatchToFace (unsigned patchnum, int facenum, int head, unsigned bitpos)
{
	patch_t		*patch = &patches[patchnum];
	patch_t		*patch2 = face_patches[facenum];

	// if emitter is behind that face plane, skip all patches

	if ( patch2 && DotProduct(patch->origin, patch2->normal) > PatchPlaneDist(patch2)+1.01 )
	{
		// we need to do a real test
		for ( ; patch2 ; patch2 = patch2->next)
		{
			unsigned m = patch2 - patches;

			// check vis between patch and patch2
			// if bit has not already been set
			//  && v2 is not behind light plane
			//  && v2 is visible from v1
			if ( m > patchnum
			  && DotProduct (patch2->origin, patch->normal) > PatchPlaneDist(patch)+1.01
		      && TestLine_r (head, patch->origin, patch2->origin) == CONTENTS_EMPTY )
			{
				// patchnum can see patch m
				int bitset = bitpos+m;
				vismatrix[ bitset>>3 ] |= 1 << (bitset&7);
			}
		}
	}
}

/*
==============
BuildVisRow

Calc vis bits from a single patch
==============
*/
void BuildVisRow (int patchnum, byte *pvs, int head, unsigned bitpos)
{
	int		j, k, l;
	patch_t	*patch;
	byte	face_tested[MAX_MAP_FACES];
	dleaf_t	*leaf;

	patch = &patches[patchnum];

	memset (face_tested, 0, numfaces);

	// leaf 0 is the solid leaf (skipped)
	for (j=1, leaf=dleafs+1 ; j<numleafs ; j++, leaf++)
	{
		if ( ! ( pvs[(j-1)>>3] & (1<<((j-1)&7)) ) )
			continue;		// not in pvs
		for (k=0 ; k<leaf->nummarksurfaces ; k++)
		{
			l = dmarksurfaces[leaf->firstmarksurface + k];

			// faces can be marksurfed by multiple leaves, but
			// don't bother testing again
			if (face_tested[l])
				continue;
			face_tested[l] = 1;

			TestPatchToFace (patchnum, l, head, bitpos);
		}
	}
}

/*
===========
BuildVisLeafs

  This is run by multiple threads
===========
*/
void BuildVisLeafs (int threadnum)
{
	int		i;
	int		lface, facenum, facenum2;
	byte	pvs[(MAX_MAP_LEAFS+7)/8];
	dleaf_t	*srcleaf, *leaf;
	patch_t	*patch;
	int		head;
	unsigned	bitpos;
	unsigned	patchnum;

	while (1)
	{
		//
		// build a minimal BSP tree that only
		// covers areas relevent to the PVS
		//
		i = GetThreadWork ();
		if (i == -1)
			break;
		i++;		// skip leaf 0
		srcleaf = &dleafs[i];
		DecompressVis (&dvisdata[srcleaf->visofs], pvs);
#if 0
	// is this valid multithreaded???
		memset (nodehit, 0, numnodes);
		for (j=1, leaf=dleafs+1 ; j<numleafs ; j++, leaf++)
		{
			if ( !( pvs[(j-1)>>3] & (1<<((j-1)&7)) ) )
				continue;
			n = leafparents[j];
			while (n != -1)
			{
				nodehit[n] = 1;
				n = nodeparents[n];
			}
		}

		head = PartialHead ();
#else
		head = 0;
#endif

		//
		// go through all the faces inside the
		// leaf, and process the patches that
		// actually have origins inside
		//
		for (lface = 0 ; lface < srcleaf->nummarksurfaces ; lface++)
		{
			facenum = dmarksurfaces[srcleaf->firstmarksurface + lface];
			for (patch = face_patches[facenum] ; patch ; patch=patch->next)
			{
				leaf = PointInLeaf (patch->origin);
				if (leaf != srcleaf)
					continue;

				patchnum = patch - patches;

			#ifdef HALFBIT
				bitpos = patchnum * num_patches - (patchnum*(patchnum+1))/2;
			#else
				bitpos = patchnum * num_patches;
			#endif
				// build to all other world leafs
				BuildVisRow (patchnum, pvs, head, bitpos);

				// build to bmodel faces
				if (nummodels < 2)
					continue;
				for (facenum2 = dmodels[1].firstface ; facenum2 < numfaces ; facenum2++)
					TestPatchToFace (patchnum, facenum2, head, bitpos);
			}
		}

	}
}

/*
==============
getfiletime
==============
*/

time_t
getfiletime(char *filename)
{
	time_t			filetime = 0;
	struct _stat	filestat;

	if ( _stat(filename, &filestat) == 0 )
		filetime = max( filestat.st_mtime, filestat.st_ctime );

	return filetime;
}

/*
==============
getfilesize
==============
*/

long
getfilesize(char *filename)
{
	long			size = 0;
	struct _stat	filestat;

	if ( _stat(filename, &filestat) == 0 )
		size = filestat.st_size;

	return size;
}

/*
==============
getfiledata
==============
*/

long
getfiledata(char *filename, char *buffer, int buffersize)
{
	long			size = 0;
	int				handle;
	long			start,end;
	time(&start);

	if ( (handle = _open( filename, _O_RDONLY | _O_BINARY )) != -1 )
	{
		int			bytesread;
		printf("%-20s Restoring [%-13s - ", "BuildVisMatrix:", filename );
		while( ( bytesread = _read( handle, buffer, min( 32*1024, buffersize - size ) ) ) > 0 )
		{
			size += bytesread;
			buffer += bytesread;
		}
		_close( handle );
		time(&end);
		printf("%10.3fMB] (%d)\n",size/(1024.0*1024.0), end-start);
	}

	if (buffersize != size)
	{
		printf( "Invalid file [%s] found.  File will be rebuilt!\n", filename );
		unlink(filename);
	}

	return size;
}

/*
==============
getfreespace
==============
*/

_int64
getfreespace(char *filename)
{
	_int64				freespace = 0;
	int					drive = 0;
	struct _diskfree_t	df;

	if ( filename[0] && filename[1] == ':' )
		drive = toupper(filename[0]) - 'A' + 1;
	else
		drive = _getdrive();

	if ( _getdiskfree(drive, &df) == 0 )
	{
		freespace = df.avail_clusters;
		freespace *= df.sectors_per_cluster;
		freespace *= df.bytes_per_sector;
	}

	return freespace;
}


/*
==============
putfiledata
==============
*/

long
putfiledata(char *filename, char *buffer, int buffersize)
{
	long			size = 0;
	int				handle;

	if ( getfreespace(filename) >= (_int64)(buffersize - getfilesize(filename)) )
	{
		if ( (handle = _open( filename, _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE )) != -1 )
		{
			int			byteswritten;
			qprintf("Writing [%s] with new saved qrad data", filename );
			while( ( byteswritten = _write( handle, buffer, min( 32*1024, buffersize - size ) ) ) > 0 )
			{
				size += byteswritten;
				buffer += byteswritten;
				if ( size >= buffersize )
					break;
			}
			qprintf("(%d)\n", size );
			
			_close( handle );
		}
	}
	else
		printf("Insufficient disk space(%ld) for 'incremental QRAD save file'!\n",
				buffersize - getfilesize(filename) );

	return size;
}

/*
==============
IsIncremental
==============
*/

qboolean
IsIncremental(char *filename)
{
	qboolean		status = false;
	int				sum;
	int				handle;

	if ( (handle = _open( filename, _O_RDONLY | _O_BINARY )) != -1 )
	{
		if ( _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dmodels_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dvertexes_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dplanes_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dleafs_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dnodes_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == texinfo_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dclipnodes_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dfaces_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dmarksurfaces_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dsurfedges_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dedges_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dtexdata_checksum
		  && _read( handle, &sum, sizeof(sum) ) == sizeof(sum) && sum == dvisdata_checksum )
			status = true;
		_close( handle );
	}

	return status;
}

/*
==============
SaveIncremental
==============
*/

int
SaveIncremental(char *filename)
{
	long			size = 0;
	int				handle;
	int				expected_size = 13*sizeof(int);

	if ( getfreespace(filename) >= expected_size )
	{
		if ( (handle = _open( filename, _O_WRONLY | _O_BINARY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE )) != -1 )
		{
			qprintf("Writing [%s] with new saved qrad data", filename );
			
			if ( _write( handle, &dmodels_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dvertexes_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dplanes_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dleafs_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dnodes_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &texinfo_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dclipnodes_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dfaces_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dmarksurfaces_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dsurfedges_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dedges_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dtexdata_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int))
			  && _write( handle, &dvisdata_checksum, sizeof(int) ) == sizeof(int) && (size += sizeof(int)) )
			{
				qprintf("(%d)\n", size );
			}
			else
			{
				qprintf("...failed!");
			}
			_close( handle );
		}
	}
	else
		printf("Insufficient disk space(%ld) for incremental file[%s]'!\n",
				expected_size, filename );

	return size;
}

/*
==============
BuildVisMatrix
==============
*/
void BuildVisMatrix (void)
{
	int		c;
    HANDLE h;

#ifdef HALFBIT
	c = ((num_patches+1)*(((num_patches+1)+15)/16));
#else
	c = num_patches*((num_patches+7)/8);
#endif

	qprintf ("visibility matrix: %5.1f megs\n", c/(1024*1024.0));

    if ( h = GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, c ) )
		vismatrix = GlobalLock( h );
	else
		Error ("vismatrix too big");
	
	strcpy(vismatfile, source);
	StripExtension (vismatfile);
	DefaultExtension(vismatfile, ".r1");

	if ( !incremental
	  || !IsIncremental(incrementfile)
	  || getfilesize(vismatfile) != c
	  || getfiledata(vismatfile,vismatrix, c) != c )
	{
		// memset (vismatrix, 0, c);
		RunThreadsOn (numleafs-1, true, BuildVisLeafs);
	}
	// Get rid of any old _bogus_ r1 files; we never read them!
	unlink(vismatfile);
}

void FreeVisMatrix (void)
{
	if ( vismatrix )
	{
		HANDLE h = GlobalHandle(vismatrix);
		GlobalUnlock(h);
		GlobalFree(h);
		vismatrix = NULL;
	}
}

/*
==============
touchfile
=============
*/

void
TouchFile(char *filename)
{
	int handle;
	if ( (handle = _open( filename, _O_RDWR | _O_BINARY  )) != -1 )
	{
		char	bytebuffer;
		qprintf("Updating saved qrad data <%s> with current time.\n", filename);
		_read( handle, &bytebuffer, sizeof(bytebuffer));
		_lseek(handle,0,SEEK_SET);
		_write( handle, &bytebuffer, sizeof(bytebuffer));
		_close( handle );
	}
}


/*
==============
CheckVisBit
==============
*/
qboolean CheckVisBit (int p1, int p2)
{
	int		t;
	int		bitpos;

	if (p1 > p2)
	{
		t = p1;
		p1 = p2;
		p2 = t;
	}

#ifdef HALFBIT
	bitpos = p1 * num_patches - (p1*(p1+1))/2 + p2;
#else
	bitpos = p1 * num_patches + p2;
#endif

	if (vismatrix[bitpos>>3] & (1<<(bitpos&7)))
		return true;
	return false;
}


