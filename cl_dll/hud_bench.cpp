//
//-----------------------------------------------------
//
#define BENCH_TIME 10.0

#include "hud.h"
#include "cl_util.h"

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "event_api.h"

#include "bench.h"

#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

#include "con_nprint.h"

#include "netadr.h"
#include "hud_benchtrace.h"

#include "net_api.h"

#include "entity_types.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#define NUM_BENCH_OBJ 12
#define BENCH_CYCLE_TIME 10.0
#define BENCH_INNER_CYCLE_TIME 4.0
#define BENCH_VIEW_CYCLE_TIME 7.1
#define BENCH_SWEEP 360.0
#define BENCH_RADIUS 80.0
#define BENCH_VIEW_OFFSET 250.0
#define BLEND_IN_SPEED 150.0
#define BENCH_BALLHEIGHT 72.0
#define BENCH_BALL_VIEWDRIFT 60.0;
#define BENCH_RANGE 60.0
// Scale:
// 0 - 100
// 0 is worst
// 100 is best
//  PP has 40 - 100 range
//  Non-pp has 0 - 60 range
const float weights[3] = { 0.2, 0.3, 0.5 };

const char *g_title = "PowerPlay QoS Test"; //uality of Service Test";
const char *pp_strings[2] =
{
	"  PowerPlay Detected",
	"  PowerPlay Not Detected" ,
};
const char *g_stage1[2] =
{
	"  Stage 1:  Testing System Connectivity...",
	"  Stage 1:  %i",
};
const char *g_stage2[2] =
{
	"  Stage 2:  Testing System Performance...",
	"  Stage 2:  %i",
};
const char *g_stage3[2] =
{
	"  Stage 3:  Testing Tracking Accuracy...",
	"  Stage 3:  %i",
};
const char *g_stage4 = "  Composite Score:  %i";

extern vec3_t v_origin;

static int g_isPowerPlay = 0;
static int g_currentstage = 0;
static int g_renderedBenchmarkDot = 0;
static float g_benchSwitchTime = 0.0;
static float g_benchSwitchTimes[ LAST_STAGE + 1 ] = { 0.0, 10.0, 12.0, 10.0, 5.0 };

#define SCORE_TIME_UP 1.5

DECLARE_MESSAGE(m_Benchmark, Bench);

void VectorAngles( const float *forward, float *angles );

void Bench_SetStage( int stage )
{
	g_currentstage = stage;
}

int Bench_GetStage( void )
{
	return g_currentstage;
}

float Bench_GetSwitchTime( void )
{
	return g_benchSwitchTimes[ min( Bench_GetStage(), LAST_STAGE ) ];
}

int Bench_InStage( int stage )
{
	return ( Bench_GetStage() == stage ) ? 1 : 0;
}

void Bench_SetPowerPlay( int set )
{
	g_isPowerPlay = set ? 1 : 0;
}

int Bench_GetPowerPlay( void )
{
	return g_isPowerPlay;
}

int Bench_Active( void )
{
	return g_currentstage != 0 ? 1 : 0;
}

void __CmdFunc_BenchMark( void )
{
	gHUD.m_Benchmark.Restart();
}


void CHudBenchmark::Restart( void )
{
	Bench_SetStage( FIRST_STAGE );
	g_benchSwitchTime = gHUD.m_flTime + g_benchSwitchTimes[ FIRST_STAGE ];
	StartNextSection( FIRST_STAGE );

	gHUD.m_Benchmark.m_iFlags |= HUD_ACTIVE;
	gHUD.m_Benchmark.m_fDrawTime = gHUD.m_flTime + BENCH_TIME;
}

int CHudBenchmark::MsgFunc_Bench(const char *pszName, int iSize, void *pbuf)
{
	int section = READ_BYTE();

	m_fReceiveTime = gHUD.m_flTime;
	m_StoredLatency = ( m_fReceiveTime - m_fSendTime );

	m_StoredLatency = min( 1.0f, m_StoredLatency );
	m_StoredLatency = max( 0.0f, m_StoredLatency );

	m_StoredPacketLoss = 0.0;

	{
		char sz[ 256 ];
		netadr_t adr;
		net_status_t status;

		gEngfuncs.pNetAPI->Status( &status );

		if ( status.connected )
		{
			adr = status.remote_address;

			sprintf( sz, "%i.%i.%i.%i",
				adr.ip[ 0 ], adr.ip[ 1 ], adr.ip[ 2 ], adr.ip[ 3 ] );

			if ( adr.type == NA_IP )
			{
				Trace_StartTrace( &m_nStoredHopCount, &m_nTraceDone, (const char *)sz );
			}
			else
			{
				m_nStoredHopCount = 0;
			}
		}
	}

	return 1;
}

void CHudBenchmark::StartNextSection( int section )
{
	net_status_t status;

	switch ( section )
	{
	case 1:
		// Stage 2 requires that we tell the server to "drop" an item
		m_fSendTime = gHUD.m_flTime;
		m_fReceiveTime = 0.0;
		m_StoredLatency = 0.0;
		m_StoredPacketLoss = 0.0;
		m_nStoredHopCount = 0;
		m_nTraceDone = 0;
		ServerCmd( "ppdemo 1 start\n" );
		break;
	case 2:
		if ( m_nTraceDone )
		{
			gEngfuncs.pNetAPI->Status( &status );

			gEngfuncs.Con_Printf( "Hops == %i\n", m_nStoredHopCount );
			m_StoredPacketLoss = status.packet_loss;
			gEngfuncs.Con_Printf( "PL == %i\n", (int)m_StoredPacketLoss );

		}
		m_nSentFinish = 0;	// added by minman
		ServerCmd( "ppdemo 2\n" );
		break;
	case 3:
		m_nSentFinish = 0;	// added by minman
		ServerCmd( "ppdemo 3\n" );
		break;
	default:
		break;
	}

	m_fStageStarted = gHUD.m_flTime;
	g_benchSwitchTime = gHUD.m_flTime + Bench_GetSwitchTime();
}

void CHudBenchmark::CountFrame( float dt )
{
	m_nFPSCount++;
	m_fAverageFT += dt;
}


static int started = 0;

void Bench_CheckStart( void )
{
	const char *level;
	if ( !started && !Bench_Active() )
	{
		level = gEngfuncs.pfnGetLevelName();
		if ( level && level[0] && !stricmp( level, "maps/ppdemo.bsp" ) )
		{
			started = 1;
			EngineClientCmd( "ppdemostart\n" );
		}
	}
}

void CHudBenchmark::Think( void )
{
	if ( !Bench_Active() )
		return;

	Trace_Think();

	if ( started )
	{
		started = 0;

		// Clear variable
		m_fReceiveTime = 0.0;
		m_nFPSCount = 0;
		m_fAverageFT = 0.0;
		m_nSentFinish = 0;
		m_StoredLatency = 0.0;
		m_StoredPacketLoss = 0.0;
		m_nStoredHopCount = 0;
		m_nTraceDone = 0;
		m_nObjects = 0;
		m_nScoreComputed = 0;
		m_nCompositeScore = 0;
		m_fAvgScore = 0;
		m_fDrawScore = 0.0;
		m_fAvgFrameRate = 0.0;
	}

	if ( gHUD.m_flTime > g_benchSwitchTime )
	{
		Bench_SetStage( Bench_GetStage() + 1 );
		StartNextSection( Bench_GetStage() );
	}

	if ( Bench_InStage( FIRST_STAGE ) )
	{
		// Assume 1000 ms lag is the max and that would take all but 2 seconds of this interval to traverse
		if ( m_fReceiveTime )
		{
			float latency = 2.0 * m_StoredLatency;
			float switch_time;
			float total_time;
			
			latency = max( 0.0f, latency );
			latency = min( 1.0f, latency );

			total_time = Bench_GetSwitchTime();
			total_time -= 2.0;

			switch_time = m_fStageStarted + latency * total_time;
			switch_time += 1.0;

			if ( gHUD.m_flTime >= switch_time )
			{
				if ( !m_nSentFinish )
				{
					g_benchSwitchTime = gHUD.m_flTime + 1.0 + SCORE_TIME_UP;

					ServerCmd( "ppdemo 1 finish\n" );
					m_nSentFinish = 1;
				}
			}
			else
			{
				g_benchSwitchTime = gHUD.m_flTime + 10.0;
			}
		}
	}

	if ( Bench_InStage( SECOND_STAGE ) )
	{
		// frametime
		static float lasttime;
		float elapsed;
		float total;
		float frac;
		float switch_time;	// added by minman

		if ( lasttime )
		{
			float dt;

			dt = gHUD.m_flTime - lasttime;
			if ( dt > 0 )
			{
				CountFrame( dt );
			}
		}
		lasttime = gHUD.m_flTime;

		elapsed = gHUD.m_flTime - m_fStageStarted;
		total = Bench_GetSwitchTime();
		if ( total )
		{
			frac = elapsed / total;

			// Only takes 1/2 time to get up to maximum speed
			frac *= 2.0;
			frac = max( 0.0f, frac );
			frac = min( 1.0f, frac );

			m_nObjects = (int)(NUM_BENCH_OBJ * frac);
		}
		switch_time = m_fStageStarted + total;

		/* BELOW ADDED BY minman */
		if (gHUD.m_flTime >= switch_time)
		{
			if ( !m_nSentFinish)
			{
				g_benchSwitchTime = gHUD.m_flTime + SCORE_TIME_UP;
				m_nSentFinish = 1;
			}
		}
		else
			g_benchSwitchTime = gHUD.m_flTime + 10.0;
	}

	/* BELOW ADDED BY minman */
	if ( Bench_InStage (THIRD_STAGE))
	{
		float switch_time = m_fStageStarted + Bench_GetSwitchTime();

		if (gHUD.m_flTime >= switch_time)
		{
			if ( !m_nSentFinish)
			{
				g_benchSwitchTime = gHUD.m_flTime + SCORE_TIME_UP;
				m_nSentFinish = 1;
			}
		}
		else
			g_benchSwitchTime = gHUD.m_flTime + 10.0;
	}

	if ( Bench_InStage( FOURTH_STAGE ) )
	{
		if ( !m_nScoreComputed )
		{
			m_nScoreComputed = 1;
			gHUD.m_Benchmark.SetCompositeScore();
		}
	}

	if ( Bench_GetStage() > LAST_STAGE )
	{
		m_iFlags &= ~HUD_ACTIVE;
		EngineClientCmd( "quit\n" );
	}
}


int CHudBenchmark::Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_COMMAND( "ppdemostart", BenchMark );

	HOOK_MESSAGE(Bench);

	return 1;
}

int CHudBenchmark::VidInit( void )
{
	return 1;
}

int CHudBenchmark::Bench_ScoreForValue( int stage, float raw )
{
	int	score = 100.0;
	int power_play = Bench_GetPowerPlay() ? 1 : 0;
	
	switch ( stage )
	{
	case 1:  // ping
		score = 100.0 * ( m_StoredLatency );
		score = max( score, 0 );
		score = min( score, 100 );

		// score is inverted
		score = 100 - score;

		break;
	case 2:  // framerate/performance
		score = (int)( 100 * m_fAvgFrameRate ) / 72;
		score = min( score, 100 );
		score = max( score, 0 );

		score *= BENCH_RANGE/100.0;
		if ( power_play )
		{
			score += ( 100 - BENCH_RANGE );
		}
		break;
	case 3:  // tracking
		score = (100 * m_fAvgScore) / 40;
		score = max( score, 0 );
		score = min( score, 100 );

		// score is inverted
		score = 100 - score;

		score *= BENCH_RANGE/100.0;
		if ( power_play )
		{
			score += ( 100 - BENCH_RANGE );
		}
		break;
	}

	return score;
}

void CHudBenchmark::SetCompositeScore( void )
{
	int	tracking_score	= Bench_ScoreForValue( THIRD_STAGE, m_fAvgScore );
	int ping_score		= Bench_ScoreForValue( FIRST_STAGE, m_StoredLatency );
	int frame_score		= Bench_ScoreForValue( SECOND_STAGE, m_fAvgFrameRate );

	int composite = ( ping_score * weights[ 0 ] + frame_score * weights[ 1 ] + tracking_score * weights[ 2 ] );
	
	composite = min( 100, composite );
	composite = max( 0, composite );

	m_nCompositeScore = composite;
}

int CHudBenchmark::Draw( float flTime )
{
	char sz[ 256 ];
	int x, y;

	if ( m_fDrawTime < flTime || !Bench_Active() )
	{
		m_iFlags &= ~HUD_ACTIVE;
		return 1;
	}

	x = 10;
	y = 25; //480 - 150;

	sprintf( sz, "%s: %s", g_title , pp_strings[ Bench_GetPowerPlay() ? 0 : 1]);

	gHUD.DrawHudString( x, y, 320, sz, 251, 237, 7);// , 200, 200); //255, 255, 255 );

	y += 20;
	
//	sprintf( sz, pp_strings[ Bench_GetPowerPlay() ? 0 : 1 ] );

//	gHUD.DrawHudString( x, y, 320, sz, 31, 200, 200 );

//	y += 20;

	
	if ( Bench_InStage( FIRST_STAGE) /*|| Bench_InStage( SECOND_STAGE ) || Bench_InStage( THIRD_STAGE )*/ || Bench_InStage( FOURTH_STAGE ) )
	{
		if ( m_fReceiveTime && m_nSentFinish )
		{
			sprintf( sz, g_stage1[1], Bench_ScoreForValue( FIRST_STAGE, m_StoredLatency ));
		}
		else
		{
			sprintf( sz, g_stage1[0] );
		}
		gHUD.DrawHudString( x, y, 320, sz, 255, 255, 255 );

		y += 20;

	}


	if ( Bench_InStage( SECOND_STAGE )/* || Bench_InStage( THIRD_STAGE )*/ || Bench_InStage( FOURTH_STAGE ) )
	{
		float avg = 0.0;
		
		if ( m_nFPSCount > 0 )
		{
			avg = m_fAverageFT / (float)m_nFPSCount;
			m_fAvgFrameRate = 1.0 / avg;
		}

		if ( m_nSentFinish /* Bench_InStage( THIRD_STAGE ) */|| Bench_InStage( FOURTH_STAGE ) )
		{
			sprintf( sz, g_stage2[1], Bench_ScoreForValue( SECOND_STAGE, m_fAvgFrameRate ) );
		}
		else
		{
			sprintf( sz, g_stage2[0] );
		}
		gHUD.DrawHudString( x, y, 320, sz, 255, 255, 255 );
		y += 20;
	}


	if ( Bench_InStage( THIRD_STAGE ) || Bench_InStage( FOURTH_STAGE ) )
	{
		if ( m_nSentFinish || Bench_InStage( FOURTH_STAGE ) )
		{
			sprintf( sz, g_stage3[1], Bench_ScoreForValue( THIRD_STAGE, m_fAvgScore ) );
		}
		else
		{
			sprintf( sz, g_stage3[0] );
		}

		gHUD.DrawHudString( x, y, 320, sz, 255, 255, 255 );

		y += 20;
	}

	if ( Bench_InStage( FOURTH_STAGE ) )
	{
		sprintf( sz, g_stage4, m_nCompositeScore );
		gHUD.DrawHudString( x, y, 320, sz, 31, 200, 200 );
	}

	m_fDrawTime = gHUD.m_flTime + BENCH_TIME;

	return 1;
}

#define SCORE_AVG 0.9 

void CHudBenchmark::SetScore( float score )
{
	// added by minman
	if (m_nSentFinish)
		return;

	m_fDrawScore = score;
	m_fDrawTime = gHUD.m_flTime + BENCH_TIME;

	m_fAvgScore = ( SCORE_AVG ) * m_fAvgScore + ( 1.0 - SCORE_AVG ) * m_fDrawScore;
}

void Bench_SetDotAdded( int dot )
{
	g_renderedBenchmarkDot = dot;
}

int Bench_GetDotAdded( void )
{
	return g_renderedBenchmarkDot;
}

void Bench_SpotPosition( vec3_t dot, vec3_t target )
{
	// Compute new score
	vec3_t delta;

	VectorSubtract( target, dot, delta );

	gHUD.m_Benchmark.SetScore( delta.Length() );
}

typedef struct model_s
{
	char		name[64];
	qboolean	needload;		// bmodels and sprites don't cache normally

	int			type;
	int			numframes;
	int			synctype;
	
	int			flags;

//
// volume occupied by the model
//		
	vec3_t		mins, maxs;
} model_t;

static vec3_t g_dotorg;
vec3_t g_aimorg;
float g_fZAdjust = 0.0;

void Bench_CheckEntity( int type, struct cl_entity_s *ent, const char *modelname )
{
	if ( Bench_InStage( THIRD_STAGE ) && !stricmp( modelname, "*3" ) )
	{
		model_t *pmod;
		vec3_t v;
		pmod = (model_t *)( ent->model );

		VectorAdd( pmod->mins, pmod->maxs, v );
		VectorScale( v, 0.5, v );

		VectorAdd( v, ent->origin, g_aimorg );
	}

	if ( Bench_InStage( THIRD_STAGE ) && strstr( modelname, "ppdemodot" ) )
	{
		Bench_SetDotAdded( 1 );
		VectorCopy( ent->origin, g_dotorg );

		// Adjust end position
		if ( Bench_Active() && Bench_InStage( THIRD_STAGE ) )
		{
			static float fZAdjust = 0.0;
			static float fLastTime;
			float dt;
			float fRate = Bench_GetPowerPlay() ? 4.0 : 8.0;
			float fBounds = Bench_GetPowerPlay() ? 8.0 : 15.0;

			dt = gHUD.m_flTime - fLastTime;
			if ( dt > 0.0 && dt < 1.0 )
			{
				fZAdjust += gEngfuncs.pfnRandomFloat( -fRate, fRate );
				fZAdjust = min( fBounds, fZAdjust );
				fZAdjust = max( -fBounds, fZAdjust );

				ent->origin[2] += fZAdjust;

				g_fZAdjust = fZAdjust;
			}
			fLastTime = gHUD.m_flTime;
		}
	}
}


void NormalizeVector( vec3_t v )
{
	int i;
	for ( i = 0; i < 3; i++ )
	{
		while ( v[i] < -180.0 )
		{
			v[i] += 360.0;
		}

		while ( v[i] > 180.0 )
		{
			v[i] -= 360.0;
		}
	}
}

float g_flStartTime;
int HUD_SetupBenchObjects( cl_entity_t *bench, int plindex, vec3_t origin )
{
	int i, j;
	vec3_t ang;
	float offset;
	struct model_s *mdl;
	int index;
	vec3_t forward, right, up;
	vec3_t farpoint;
	vec3_t centerspot;
	pmtrace_t tr;
	
	ang = vec3_origin;
	//ang[1] = 90.0;
	
	// Determine forward vector
	AngleVectors ( ang, forward, right, up );

	// Try to find the laserdot sprite model and retrieve the modelindex for it
	mdl = gEngfuncs.CL_LoadModel( "models/spikeball.mdl", &index );
	if ( !mdl )
		return 0;

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( plindex );	

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

	centerspot = origin;
	centerspot[2] -= 512;

	gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)&origin, (float *)&centerspot, PM_NORMAL, -1, &tr );

	centerspot = tr.endpos;
	centerspot[2] += BENCH_BALLHEIGHT;

	// Move center out from here
	centerspot = centerspot + BENCH_VIEW_OFFSET * forward;
	
	g_flStartTime = gHUD.m_flTime;

	for ( i = 0; i < NUM_BENCH_OBJ; i++ )
	{
		offset = ( float ) i / (float) ( NUM_BENCH_OBJ - 1 );

		ang[ 0 ] = 0;
		ang[ 2 ] = 0;
		ang[ 1 ] = BENCH_SWEEP * offset;

		// normalize
		NormalizeVector( ang );

		// Determine forward vector
		AngleVectors ( ang, forward, right, up );

		bench[ i ].model = mdl;

		bench[ i ].curstate.modelindex		= index;

		// Set up dot info.
		bench[ i ].curstate.movetype		= MOVETYPE_NONE;
		bench[ i ].curstate.solid			= SOLID_NOT;

		// Get a far point for ray trace
		farpoint = centerspot + BENCH_RADIUS * forward;

		gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)&centerspot, (float *)&farpoint, PM_NORMAL, -1, &tr );

		// Move dot to trace endpoint
		bench[ i ].origin			= tr.endpos;
		bench[ i ].curstate.origin = bench[ i ].origin;
		//bench[ i ].curstate.gravity = 0.5;
		for ( j = 0; j < 2; j++ )
		{
		//	bench[ i ].curstate.velocity[ j ] = gEngfuncs.pfnRandomLong( -300, 300 );
		}
		//bench[ i ].curstate.velocity[ 2 ] = gEngfuncs.pfnRandomLong( 0, 50 );
		bench[ i ].curstate.velocity = vec3_origin;

		bench[ i ].curstate.angles[ 2 ] = 0.0;
		bench[ i ].curstate.angles[ 0 ] = 0.0;
		bench[ i ].curstate.angles[ 1 ] = 0.0; // gEngfuncs.pfnRandomLong( -180, 180 );

		for ( j = 0; j < 3; j++ )
		{
			// angular velocity
			bench[ i ].baseline.angles[ 0 ] = gEngfuncs.pfnRandomLong( -300, 300 );
			//bench[ i ].baseline.angles[ 0 ] = 0;
			bench[ i ].baseline.angles[ 2 ] = 0;
			bench[ i ].baseline.angles[ 1 ] = gEngfuncs.pfnRandomLong( -300, 300 );
		}

		bench[ i ].curstate.renderamt = 0;

		// Force no interpolation, etc., probably not necessary
		bench[ i ].prevstate		= bench[ i ].curstate;
	}
	
	gEngfuncs.pEventAPI->EV_PopPMStates();

	return 1;
}

void HUD_CreateBenchObjects( vec3_t origin )
{
	static cl_entity_t bench[ NUM_BENCH_OBJ ];
	cl_entity_t *player;
	vec3_t forward, right, up;
	vec3_t farpoint;
	vec3_t centerspot;
	static int first = true;
	static int failed = false;
	static float last_time;
	float frametime;
	float frac;
	float frac2;
	float dt;

	pmtrace_t tr;
	int i = 0;

	if ( gHUD.m_flTime == last_time )
		return;

	frametime = gHUD.m_flTime - last_time;
	last_time = gHUD.m_flTime;

	if ( frametime <= 0.0 )
		return;

	if ( failed )
		return;

	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
	{
		failed = true;
		return;
	}

	if ( first )
	{
		first = false;
		
		if ( !HUD_SetupBenchObjects( bench, player->index - 1, origin ) )
		{
			failed = true;
			return;
		}
	}

	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction( false, true );

	// Store off the old count
	gEngfuncs.pEventAPI->EV_PushPMStates();

	// Now add in all of the players.
	gEngfuncs.pEventAPI->EV_SetSolidPlayers ( player->index - 1 );	

	gEngfuncs.pEventAPI->EV_SetTraceHull( 2 );

	dt = gHUD.m_flTime - g_flStartTime;
	if ( dt < 0 )
		return;

	frac = dt / BENCH_CYCLE_TIME;
	if ( frac > 1.0 )
	{
		frac = frac - (float)(int)frac;
	}

	frac2 = dt /BENCH_INNER_CYCLE_TIME;
	if ( frac2 > 1.0 )
	{
		frac2 = frac2 - (float)(int)frac2;
	}

	// Determine forward vector
	AngleVectors ( vec3_origin, forward, right, up );

	centerspot = origin;
	centerspot[2] -= 512;

	gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)&origin, (float *)&centerspot, PM_NORMAL, -1, &tr );

	centerspot = tr.endpos;
	centerspot[2] += BENCH_BALLHEIGHT;

	// Move center out from here
	centerspot = centerspot + BENCH_VIEW_OFFSET * forward;

	for ( i = 0; i < NUM_BENCH_OBJ; i++ )
	{
		int j;
		float jitter = 0.0;
		float jfrac;
		float offset;
		float ofs_radius = 5.0;

		vec3_t ang;
		offset = ( float ) i / (float) ( NUM_BENCH_OBJ - 1 );

		ang[ 0 ] = 0;
		ang[ 2 ] = 0;
		ang[ 1 ] = BENCH_SWEEP * offset + frac * 360.0;
		// normalize
		NormalizeVector( ang );

		// Determine forward vector
		AngleVectors ( ang, forward, right, up );

		// Get a far point for ray trace
		farpoint = centerspot + ( BENCH_RADIUS + ofs_radius * sin( BENCH_SWEEP * offset + frac2 * 2 * M_PI ) ) * forward;
		farpoint[2] += 10 * cos( BENCH_SWEEP * offset + frac2 * 2 * M_PI );

		gEngfuncs.pEventAPI->EV_PlayerTrace( (float *)&centerspot, (float *)&farpoint, PM_NORMAL, -1, &tr );

		// Add angular velocity
		VectorMA( bench[ i ].curstate.angles, frametime, bench[ i ].baseline.angles, bench[ i ].curstate.angles );

		NormalizeVector( bench[ i ].curstate.angles );
		
		jfrac = ( (float)gHUD.m_Benchmark.GetObjects() / (float)NUM_BENCH_OBJ );

		// Adjust velocity
		//bench[ i ].curstate.velocity[ 2 ] -= bench[ i ].curstate.gravity * frametime * 800;

		/*
		// Did we hit something?
		if ( tr.fraction != 1.0 && !tr.inwater )
		{
			float damp;
			float proj;
			vec3_t traceNormal;
			int j;

			traceNormal = tr.plane.normal;

			damp = 0.9;

			// Reflect velocity
			if ( damp != 0 )
			{
				proj = DotProduct( bench[ i ].curstate.velocity, traceNormal );
				VectorMA( bench[ i ].curstate.velocity, -proj*2, traceNormal, bench[ i ].curstate.velocity );
				// Reflect rotation (fake)

				for ( j = 0 ; j < 3; j++ )
				{
					if ( bench[ i ].curstate.velocity[ j ] > 1000.0 )
					{
						bench[ i ].curstate.velocity[ j ] = 1000.0;
					}
					else if ( bench[ i ].curstate.velocity[ j ] < -1000.0 )
					{
						bench[ i ].curstate.velocity[ j ] = -1000.0;
					}
				}

				bench[ i ].baseline.angles[1] = -bench[ i ].baseline.angles[1];

				VectorScale( bench[ i ].curstate.velocity, damp, bench[ i ].curstate.velocity );
			}
		}
		*/

		if ( i == ( NUM_BENCH_OBJ - 1 ) )
		{
			g_aimorg = tr.endpos;
		}

		if ( Bench_GetPowerPlay() )
		{
			jitter = 0.5;
		}
		else
		{
			jitter = 8.0;
		}
				
		jitter *= jfrac;

		for ( j = 0; j < 2; j++ )
		{
			tr.endpos[ j ] += gEngfuncs.pfnRandomFloat( -jitter, jitter );
		}

		// Add to visedicts list for rendering
		// Move dot to trace endpoint
		bench[ i ].origin			= tr.endpos;
		bench[ i ].curstate.origin = bench[ i ].origin;
		bench[ i ].angles = bench[ i ].curstate.angles;

		// Force no interpolation, etc., probably not necessary
		bench[ i ].prevstate		= bench[ i ].curstate;

		if ( ( NUM_BENCH_OBJ - i - 1 ) < gHUD.m_Benchmark.GetObjects() )
		{
			if ( bench[ i ].curstate.renderamt == 255 )
			{
				bench[i].curstate.rendermode = kRenderNormal;
			}
			else
			{
				bench[i].curstate.renderamt += BLEND_IN_SPEED * frametime;
				bench[i].curstate.renderamt = min( 255, bench[i].curstate.renderamt );
				bench[i].curstate.rendermode = kRenderTransAlpha;
			}

			gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, &bench[ i ] );
		}
	}

	gEngfuncs.pEventAPI->EV_PopPMStates();
}

void Bench_AddObjects( void )
{
	if ( Bench_GetDotAdded() )
	{
		Bench_SpotPosition( g_dotorg, g_aimorg );
		Bench_SetDotAdded( 0 );
	}

	if ( Bench_InStage( SECOND_STAGE ) )
	{
		HUD_CreateBenchObjects( v_origin );	
	}
}


static vec3_t v_stochastic;

void Bench_SetViewAngles( int recalc_wander, float *viewangles, float frametime, struct usercmd_s *cmd )
{
	if ( !Bench_Active() )
		return;

	int i;
	vec3_t lookdir;

	// Clear stochastic offset between runs
	if ( Bench_InStage( FIRST_STAGE ) )
	{
		VectorCopy( vec3_origin, v_stochastic );
	}

	if ( Bench_InStage( SECOND_STAGE ) || Bench_InStage( THIRD_STAGE ) )
	{
		VectorSubtract( g_aimorg, v_origin, lookdir );
		VectorNormalize( lookdir );
		VectorAngles( (float *)&lookdir, viewangles );
		
		viewangles[0] = -viewangles[0];

		/*
		if ( recalc_wander )
		{
			float fmag = 2.0;
			if ( Bench_GetPowerPlay() )
			{
				fmag = 10.0;
			}

			for ( i = 0; i < 2; i++ )
			{
				v_stochastic[ i ] += frametime * gEngfuncs.pfnRandomFloat( -fmag, fmag );
				v_stochastic[ i ] = max( -15.0, v_stochastic[ i ] );
				v_stochastic[ i ] = min( 15.0, v_stochastic[ i ] );
			}

			v_stochastic[ 2 ] = 0.0;
		}
		*/

		VectorAdd( viewangles, v_stochastic, viewangles );

		for ( i = 0; i < 3; i++ )
		{
			if ( viewangles[ i ] > 180 )
				viewangles[ i ] -= 360;
			if ( viewangles[ i ] < -180 )
				viewangles[ i ] += 360;
		}
	}
	else
	{
		VectorCopy( vec3_origin, viewangles )

		if ( Bench_InStage( FIRST_STAGE ) )
		{
			viewangles[ 1 ] = -90;
		}
	}

	if ( cmd )
	{
		if ( Bench_InStage( THIRD_STAGE ) )
		{
			cmd->buttons = IN_ATTACK;
		}
		else
		{
			cmd->buttons = 0;
		}
	}
}

void Bench_SetViewOrigin( float *vieworigin, float frametime )
{
	float dt;
	float frac;
	float offset_amt = BENCH_BALL_VIEWDRIFT;
	float drift;
	vec3_t ang, right;
	vec3_t move;

	if ( !Bench_InStage( SECOND_STAGE ) )
		return;

	dt = gHUD.m_flTime - g_flStartTime;
	if ( dt < 0 )
		return;

	frac = dt / BENCH_VIEW_CYCLE_TIME;
	frac *= 2 * M_PI;

	drift = sin( frac ) * offset_amt;
	
	ang = vec3_origin;

	AngleVectors( ang, NULL, right, NULL );

	// offset along right axis
	move = right * drift;
	
	VectorAdd( vieworigin, move, vieworigin );
}

