//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: New version of the slider bar
//
// $NoKeywords: $
//=============================================================================

//=========================================================
// squad.h
//=========================================================

// these are special group roles that are assigned to members when the group is formed.
// the reason these are explicitly assigned and tasks like throwing grenades to flush out 
// enemies is that it's bad to have two members trying to flank left at the same time, but 
// ok to have two throwing grenades at the same time. When a squad member cannot attack the 
// enemy, it will choose to execute its special role.
#define		bits_SQUAD_FLANK_LEFT		( 1 << 0 )
#define		bits_SQUAD_FLANK_RIGHT		( 1 << 1 )
#define		bits_SQUAD_ADVANCE			( 1 << 2 )
#define		bits_SQUAD_FLUSH_ATTACK		( 1 << 3 )
