/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
//  parsemsg.h
//	MDC - copying from cstrike\cl_dll so career-mode stuff can catch messages
//  in this dll. (and C++ifying it)
//

#ifndef PARSEMSG_H
#define PARSEMSG_H

#define ASSERT( x )
//--------------------------------------------------------------------------------------------------------------
void BEGIN_READ( void *buf, int size );
int READ_CHAR( void );
int READ_BYTE( void );
int READ_SHORT( void );
int READ_WORD( void );
int READ_LONG( void );
float READ_FLOAT( void );
char* READ_STRING( void );
float READ_COORD( void );
float READ_ANGLE( void );
float READ_HIRESANGLE( void );
int READ_OK( void );

//--------------------------------------------------------------------------------------------------------------
class BufferWriter
{
public:
	BufferWriter();
	BufferWriter( unsigned char *buffer, int bufferLen );
	void Init( unsigned char *buffer, int bufferLen );

	void WriteByte( unsigned char data );
	void WriteLong( int data );
	void WriteString( const char *str );

	bool HasOverflowed();
	int GetSpaceUsed();

protected:
	unsigned char *m_buffer;
	int m_remaining;
	bool m_overflow;
	int m_overallLength;
};

//--------------------------------------------------------------------------------------------------------------

#endif // PARSEMSG_H



