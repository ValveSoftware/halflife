/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
// netadr.h
#ifndef NETADR_H
#define NETADR_H
#ifdef _WIN32
#pragma once
#endif

// JoshA: Unfortunately netadr_s is passed to clients for connectionless packets.
// No Valve mod uses them, but custom mods *might*, so not changing the start of this struct layout.
// It's very unlikely they touch this, but I'd like to play as safe as possible with all ABI etc for mod compat.
// If we want to add IPv6 someday, bung it at the end of netadr_s and leave ip + ipx alone.

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX, // deprecated
	NA_BROADCAST_IPX, // deprecated
} netadrtype_t;

typedef struct netadr_s
{
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned char	ipx[10]; // deprecated
	unsigned short	port;
} netadr_t;

#endif // NETADR_H
