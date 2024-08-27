#ifndef COMMONMACROS_H
#define COMMONMACROS_H
#pragma once


// -------------------------------------------------------
//
// commonmacros.h
//
// This should contain ONLY general purpose macros that are 
// appropriate for use in engine/launcher/all tools
//
// -------------------------------------------------------

// Makes a 4-byte "packed ID" int out of 4 characters
#define MAKEID(d,c,b,a)					( ((int)(a) << 24) | ((int)(b) << 16) | ((int)(c) << 8) | ((int)(d)) )

// Compares a string with a 4-byte packed ID constant
#define STRING_MATCHES_ID( p, id )		( (*((int *)(p)) == (id) ) ? true : false )
#define ID_TO_STRING( id, p )			( (p)[3] = (((id)>>24) & 0xFF), (p)[2] = (((id)>>16) & 0xFF), (p)[1] = (((id)>>8) & 0xFF), (p)[0] = (((id)>>0) & 0xFF) )

#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))


#endif		// COMMONMACROS_H
