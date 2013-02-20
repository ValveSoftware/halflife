//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// exefuncs.h
#ifndef EXEFUNCS_H
#define EXEFUNCS_H

// Engine hands this to DLLs for functionality callbacks
typedef struct exefuncs_s
{
	int			fMMX;
	int			iCPUMhz;
	void		(*unused1)(void);
	void		(*unused2)(void);
	void		(*unused3)(void);
	void		(*unused4)(void);
	void		(*VID_ForceLockState)(int lk);
	int			(*VID_ForceUnlockedAndReturnState)(void);
	void		(*unused5)(void);
	void		(*unused6)(void);
	void		(*unused7)(void);
	void		(*unused8)(void);
	void		(*unused9)(void);
	void		(*unused10)(void);
	void		(*unused11)(void);
	void		(*unused12)(void);
	void		(*unused13)(void);
	void		(*unused14)(void);
	void		(*unused15)(void);
	void        (*ErrorMessage)(int nLevel, const char *pszErrorMessage);
	void		(*unused16)(void);
	void        (*Sys_Printf)(char *fmt, ...);
	void		(*unused17)(void);
	void		(*unused18)(void);
	void		(*unused19)(void);
	void		(*unused20)(void);
	void		(*unused21)(void);
	void		(*unused22)(void);
	void		(*unused23)(void);
	void		(*unused24)(void);
	void		(*unused25)(void);
	void		(*unused26)(void);
	void		(*unused27)(void);
} exefuncs_t;

#endif
