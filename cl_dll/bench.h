#if !defined ( BENCHH )
#define BENCHH
#pragma once

#define FIRST_STAGE		1
#define SECOND_STAGE	2
#define THIRD_STAGE		3
#define FOURTH_STAGE	4
#define LAST_STAGE		( FOURTH_STAGE )

void Bench_CheckStart( void );

int Bench_InStage( int stage );
int Bench_GetPowerPlay( void );
int Bench_GetStage( void );
void Bench_SetPowerPlay( int set );
int Bench_Active( void );

void Bench_SetDotAdded( int dot );
void Bench_SpotPosition( vec3_t dot, vec3_t target );
void Bench_CheckEntity( int type, struct cl_entity_s *ent, const char *modelname );
void Bench_AddObjects( void );
void Bench_SetViewAngles( int recalc_wander, float *viewangles, float frametime, struct usercmd_s *cmd );
void Bench_SetViewOrigin( float *vieworigin, float frametime );

#endif