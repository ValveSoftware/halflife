//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// dedicated.h
#ifndef INC_DEDICATEDH
#define INC_DEDICATEDH

int		Eng_Frame		( int fForce, double time );
int		Eng_Load		( const char *cmdline, struct exefuncs_s *pef, int memory, void *pmembase, const char *psz, int iSubMode );
void	Eng_Unload		( void);
void	Eng_SetState	( int );
void	Eng_SetSubState	( int );

char	*CheckParm		( const char *psz, char **ppszValue = (char **)0 );

extern int gDLLState;
extern int gDLLStateInfo;

#endif