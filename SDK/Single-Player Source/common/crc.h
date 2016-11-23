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
/* crc.h */
#ifndef CRC_H
#define CRC_H
#ifdef _WIN32
#pragma once
#endif

// MD5 Hash
typedef struct
{
	unsigned int buf[4];
    unsigned int bits[2];
    unsigned char in[64];
} MD5Context_t;


typedef unsigned long CRC32_t;
void CRC32_Init(CRC32_t *pulCRC);
CRC32_t CRC32_Final(CRC32_t pulCRC);
void CRC32_ProcessBuffer(CRC32_t *pulCRC, void *p, int len);
void CRC32_ProcessByte(CRC32_t *pulCRC, unsigned char ch);
int CRC_File(CRC32_t *crcvalue, char *pszFileName);

unsigned char COM_BlockSequenceCRCByte (unsigned char *base, int length, int sequence);

void MD5Init(MD5Context_t *context);
void MD5Update(MD5Context_t *context, unsigned char const *buf,
               unsigned int len);
void MD5Final(unsigned char digest[16], MD5Context_t *context);
void Transform(unsigned int buf[4], unsigned int const in[16]);

int MD5_Hash_File(unsigned char digest[16], char *pszFileName, int bUsefopen, int bSeed, unsigned int seed[4]);
char *MD5_Print(unsigned char hash[16]);
int MD5_Hash_CachedFile(unsigned char digest[16], unsigned char *pCache, int nFileSize, int bSeed, unsigned int seed[4]);

int CRC_MapFile(CRC32_t *crcvalue, char *pszFileName);

#endif
