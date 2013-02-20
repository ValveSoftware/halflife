/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#include "bsp5.h"

int		outleafs;
int		valid;
int		c_falsenodes;
int		c_free_faces;
int		c_keep_faces;

/*
===========
PointInLeaf
===========
*/
node_t	*PointInLeaf (node_t *node, vec3_t point)
{
	vec_t	d;
	
	if (node->contents)
		return node;
		
	d = DotProduct (dplanes[node->planenum].normal, point) - dplanes[node->planenum]. dist;
	
	if (d > 0)
		return PointInLeaf (node->children[0], point);
	
	return PointInLeaf (node->children[1], point);
}

/*
===========
PlaceOccupant
===========
*/
qboolean PlaceOccupant (int num, vec3_t point, node_t *headnode)
{
	node_t	*n;
	
	n = PointInLeaf (headnode, point);
	if (n->contents == CONTENTS_SOLID)
		return false;
	n->occupied = num;
	return true;
}


/*
=============
WindingCenter
=============
*/
void	WindingCenter (winding_t *w, vec3_t center)
{
	int		i;
	vec3_t	d1, d2, cross;
	float	scale;

	VectorCopy (vec3_origin, center);
	for (i=0 ; i<w->numpoints ; i++)
		VectorAdd (w->points[i], center, center);

	scale = 1.0/w->numpoints;
	VectorScale (center, scale, center);
}

/*
==============
MarkLeakTrail
==============
*/
portal_t	*prevleaknode;
FILE	*pointfile, *linefile;
void MarkLeakTrail (portal_t *n2)
{
	int		i, j;
	vec3_t	p1, p2, dir;
	float	len;
	portal_t *n1;

	if (hullnum)
		return;

	n1 = prevleaknode;
	prevleaknode = n2;
	
	if (!n1)
		return;

	WindingCenter (n2->winding, p1);
	WindingCenter (n1->winding, p2);

	fprintf (linefile, "%f %f %f\n", p1[0], p1[1], p1[2]);
	
	VectorSubtract (p2, p1, dir);
	len = VectorLength (dir);
	VectorNormalize (dir);

	while (len > 2)
	{
		fprintf (pointfile,"%f %f %f\n", p1[0], p1[1], p1[2]);
		for (i=0 ; i<3 ; i++)
			p1[i] += dir[i]*2;
		len -= 2;
	}

}

/*
==================
RecursiveFillOutside

If fill is false, just check, don't fill
Returns true if an occupied leaf is reached
==================
*/
int		hit_occupied;
int		backdraw;
qboolean RecursiveFillOutside (node_t *l, qboolean fill)
{
	portal_t	*p;
	int			s;

	if (l->contents == CONTENTS_SOLID || l->contents == CONTENTS_SKY)
		return false;
		
	if (l->valid == valid)
		return false;
	
	if (l->occupied)
	{
		hit_occupied = l->occupied;
		backdraw = 1000;
		return true;
	}
	
	l->valid = valid;

// fill it and it's neighbors
	if (fill)
	{
		l->contents = CONTENTS_SOLID;
		l->planenum = -1;
	}
	outleafs++;

	for (p=l->portals ; p ; )
	{
		s = (p->nodes[0] == l);

		if (RecursiveFillOutside (p->nodes[s], fill) )
		{	// leaked, so stop filling
			if (backdraw-- > 0)
			{				
				MarkLeakTrail (p);
				DrawLeaf (l, 2);
			}
			return true;
		}
		p = p->next[!s];
	}
	
	return false;
}

/*
==================
ClearOutFaces_r

Removes unused nodes
==================
*/
node_t *ClearOutFaces_r (node_t *node)
{
	face_t		*f, *fnext;
	face_t		**fp;
	portal_t	*p;

	// mark the node and all it's faces, so they
	// can be removed if no children use them

	node->valid = 0;	// will be set if any children touch it
	for (f=node->faces ; f ; f=f->next)
		f->outputnumber = -1;

	// go down the children
	if (node->planenum != -1)
	{
		//
		// decision node
		//
		node->children[0] = ClearOutFaces_r (node->children[0]);
		node->children[1] = ClearOutFaces_r (node->children[1]);

		// free any faces not in open child leafs
		f=node->faces;
		node->faces = NULL;

		for ( ; f ; f=fnext)
		{
			fnext = f->next;
			if (f->outputnumber == -1)
			{	// never referenced, so free it
				c_free_faces++;
				FreeFace (f);
			}
			else
			{
				c_keep_faces++;
				f->next = node->faces;
				node->faces = f;
			}
		}

		if (!node->valid)
		{
			// this node does not touch any interior leafs

			// if both children are solid, just make this node solid
			if (node->children[0]->contents == CONTENTS_SOLID
			&& node->children[1]->contents == CONTENTS_SOLID)
			{
				node->contents = CONTENTS_SOLID;
				node->planenum = -1;
				return node;
			}

			// if one child is solid, shortcut down the other side
			if (node->children[0]->contents == CONTENTS_SOLID)
				return node->children[1];
			if (node->children[1]->contents == CONTENTS_SOLID)
				return node->children[0];

			c_falsenodes++;
		}
		return node;
	}

	//
	// leaf node
	//
	if (node->contents != CONTENTS_SOLID)
	{
		// this node is still inside

		// mark all the nodes used as portals
		for (p = node->portals ; p ; )
		{
			if (p->onnode)
				p->onnode->valid = 1;
			if (p->nodes[0] == node)		// only write out from first leaf
				p = p->next[0];
			else
				p = p->next[1];		
		}

		// mark all of the faces to be drawn
		for (fp = node->markfaces ; *fp ; fp++)
			(*fp)->outputnumber = 0;

		return node;
	}

	// this was a filled in node, so free the markfaces
	if (node->planenum != -1)
		free (node->markfaces);

	return node;
}


//=============================================================================

/*
===========
FillOutside

===========
*/
node_t *FillOutside (node_t *node, qboolean leakfile)
{
	int			s;
	vec_t		*v;
	int			i;
	qboolean	inside;
	qboolean	ret;
	vec3_t		origin;
	char		*cl;

	qprintf ("----- FillOutside ----\n");

	if (nofill)
	{
		printf ("skipped\n");
		return node;
	}
		
	//
	// place markers for all entities so
	// we know if we leak inside
	//
	inside = false;
	for (i=1 ; i<num_entities ; i++)
	{
		GetVectorForKey (&entities[i], "origin", origin);
		if (!VectorCompare(origin, vec3_origin))
		{
			cl = ValueForKey (&entities[i], "classname");
			origin[2] += 1;	// so objects on floor are ok

			// nudge playerstart around if needed so clipping hulls allways
			// have a vlaid point
			if (!strcmp (cl, "info_player_start"))
			{
				int	x, y;

				for (x=-16 ; x<=16 ; x += 16)
				{
					for (y=-16 ; y<=16 ; y += 16)
					{
						origin[0] += x;
						origin[1] += y;
						if (PlaceOccupant (i, origin, node))
						{
							inside = true;
							goto gotit;
						}
						origin[0] -= x;
						origin[1] -= y;
					}
				}
gotit: ;
			}
			else
			{
				if (PlaceOccupant (i, origin, node))
					inside = true;
			}
		}
	}

	if (!inside)
	{
		printf ("Hullnum %i: No entities in empty space -- no filling performed\n", hullnum);
		return node;
	}

	s = !(outside_node.portals->nodes[1] == &outside_node);

// first check to see if an occupied leaf is hit
	outleafs = 0;
	valid++;

	prevleaknode = NULL;
	
	if (leakfile)
	{
		pointfile = fopen (pointfilename, "w");
		if (!pointfile)
			Error ("Couldn't open %s\n", pointfilename);
		StripExtension (pointfilename);
		strcat (pointfilename, ".lin");
		linefile = fopen (pointfilename, "w");
		if (!linefile)
			Error ("Couldn't open %s\n", pointfilename);
	}

	ret = RecursiveFillOutside (outside_node.portals->nodes[s], false);

	if (leakfile)
	{
		fclose (pointfile);
		fclose (linefile);
	}

	if (ret)
	{
		printf("LEAK LEAK LEAK\n");
		GetVectorForKey (&entities[hit_occupied], "origin", origin);
		qprintf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		qprintf ("reached occupant at: (%4.0f,%4.0f,%4.0f)\n"
		, origin[0], origin[1], origin[2]);
		qprintf ("no filling performed\n");
		qprintf ("point file and line file generated\n");
		qprintf ("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		if (leakonly)
			Error ("Stopped by leak.");
		return node;
	}

// now go back and fill things in
	valid++;
	RecursiveFillOutside (outside_node.portals->nodes[s], true);

// remove faces and nodes from filled in leafs	
	c_falsenodes = 0;
	c_free_faces = 0;
	c_keep_faces = 0;
	node = ClearOutFaces_r (node);

	qprintf ("%5i outleafs\n", outleafs);
	qprintf ("%5i freed faces\n", c_free_faces);
	qprintf ("%5i keep faces\n", c_keep_faces);
	qprintf ("%5i falsenodes\n", c_falsenodes);

// save portal file for vis tracing
	if (leakfile)
		WritePortalfile (node);

	return node;
}


