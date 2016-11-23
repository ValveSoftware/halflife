/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/

#ifndef _MOVIE_H_
#define _MOVIE_H_

/*
	movie.h

	definitions and such for dumping screen shots to make a movie
*/

typedef struct
{
	unsigned long tag;
	unsigned long size;
} movieblockheader_t;


typedef struct	
{
	short width;
	short height;
	short depth;
} movieframe_t;



#endif _MOVIE_H_