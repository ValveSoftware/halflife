//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// conproc.h -- support for external server monitoring programs
#ifndef INC_CONPROCH
#define INC_CONPROCH

#define CCOM_WRITE_TEXT		0x2
// Param1 : Text

#define CCOM_GET_TEXT		0x3
// Param1 : Begin line
// Param2 : End line

#define CCOM_GET_SCR_LINES	0x4
// No params

#define CCOM_SET_SCR_LINES	0x5
// Param1 : Number of lines

void InitConProc ( void );
void DeinitConProc ( void );

void WriteStatusText( char *psz );

#endif // !INC_CONPROCH