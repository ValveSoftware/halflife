#ifndef PM_DEBUG_H
#define PM_DEBUG_H
#pragma once

void PM_ViewEntity( void );
void PM_DrawBBox(vec3_t mins, vec3_t maxs, vec3_t origin, int pcolor, float life);
void PM_ParticleLine(vec3_t start, vec3_t end, int pcolor, float life, float vert);
void PM_ShowClipBox( void );

#endif // PMOVEDBG_H