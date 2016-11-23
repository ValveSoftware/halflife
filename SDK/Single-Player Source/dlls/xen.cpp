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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"
#include "effects.h"


#define XEN_PLANT_GLOW_SPRITE		"sprites/flare3.spr"
#define XEN_PLANT_HIDE_TIME			5


class CActAnimating : public CBaseAnimating
{
public:
	void			SetActivity( Activity act );
	inline Activity	GetActivity( void ) { return m_Activity; }

	virtual int	ObjectCaps( void ) { return CBaseAnimating :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

private:
	Activity	m_Activity;
};

TYPEDESCRIPTION	CActAnimating::m_SaveData[] = 
{
	DEFINE_FIELD( CActAnimating, m_Activity, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CActAnimating, CBaseAnimating );

void CActAnimating :: SetActivity( Activity act ) 
{ 
	int sequence = LookupActivity( act ); 
	if ( sequence != ACTIVITY_NOT_AVAILABLE )
	{
		pev->sequence = sequence;
		m_Activity = act; 
		pev->frame = 0;
		ResetSequenceInfo( );
	}
}




class CXenPLight : public CActAnimating
{
public:
	void		Spawn( void );
	void		Precache( void );
	void		Touch( CBaseEntity *pOther );
	void		Think( void );

	void		LightOn( void );
	void		LightOff( void );

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

private:
	CSprite		*m_pGlow;
};

LINK_ENTITY_TO_CLASS( xen_plantlight, CXenPLight );

TYPEDESCRIPTION	CXenPLight::m_SaveData[] = 
{
	DEFINE_FIELD( CXenPLight, m_pGlow, FIELD_CLASSPTR ),
};

IMPLEMENT_SAVERESTORE( CXenPLight, CActAnimating );

void CXenPLight :: Spawn( void )
{
	Precache();

	SET_MODEL( ENT(pev), "models/light.mdl" );
	pev->movetype	= MOVETYPE_NONE;
	pev->solid		= SOLID_TRIGGER;

	UTIL_SetSize( pev, Vector(-80,-80,0), Vector(80,80,32));
	SetActivity( ACT_IDLE );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT(0,255);

	m_pGlow = CSprite::SpriteCreate( XEN_PLANT_GLOW_SPRITE, pev->origin + Vector(0,0,(pev->mins.z+pev->maxs.z)*0.5), FALSE );
	m_pGlow->SetTransparency( kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx );
	m_pGlow->SetAttachment( edict(), 1 );
}


void CXenPLight :: Precache( void )
{
	PRECACHE_MODEL( "models/light.mdl" );
	PRECACHE_MODEL( XEN_PLANT_GLOW_SPRITE );
}


void CXenPLight :: Think( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	switch( GetActivity() )
	{
	case ACT_CROUCH:
		if ( m_fSequenceFinished )
		{
			SetActivity( ACT_CROUCHIDLE );
			LightOff();
		}
		break;

	case ACT_CROUCHIDLE:
		if ( gpGlobals->time > pev->dmgtime )
		{
			SetActivity( ACT_STAND );
			LightOn();
		}
		break;

	case ACT_STAND:
		if ( m_fSequenceFinished )
			SetActivity( ACT_IDLE );
		break;

	case ACT_IDLE:
	default:
		break;
	}
}


void CXenPLight :: Touch( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() )
	{
		pev->dmgtime = gpGlobals->time + XEN_PLANT_HIDE_TIME;
		if ( GetActivity() == ACT_IDLE || GetActivity() == ACT_STAND )
		{
			SetActivity( ACT_CROUCH );
		}
	}
}


void CXenPLight :: LightOn( void )
{
	SUB_UseTargets( this, USE_ON, 0 );
	if ( m_pGlow )
		m_pGlow->pev->effects &= ~EF_NODRAW;
}


void CXenPLight :: LightOff( void )
{
	SUB_UseTargets( this, USE_OFF, 0 );
	if ( m_pGlow )
		m_pGlow->pev->effects |= EF_NODRAW;
}



class CXenHair : public CActAnimating
{
public:
	void		Spawn( void );
	void		Precache( void );
	void		Think( void );
};

LINK_ENTITY_TO_CLASS( xen_hair, CXenHair );

#define SF_HAIR_SYNC		0x0001

void CXenHair::Spawn( void )
{
	Precache();
	SET_MODEL( edict(), "models/hair.mdl" );
	UTIL_SetSize( pev, Vector(-4,-4,0), Vector(4,4,32));
	pev->sequence = 0;
	
	if ( !(pev->spawnflags & SF_HAIR_SYNC) )
	{
		pev->frame = RANDOM_FLOAT(0,255);
		pev->framerate = RANDOM_FLOAT( 0.7, 1.4 );
	}
	ResetSequenceInfo( );

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.4 );	// Load balance these a bit
}


void CXenHair::Think( void )
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}


void CXenHair::Precache( void )
{
	PRECACHE_MODEL( "models/hair.mdl" );
}


class CXenTreeTrigger : public CBaseEntity
{
public:
	void		Touch( CBaseEntity *pOther );
	static CXenTreeTrigger *TriggerCreate( edict_t *pOwner, const Vector &position );
};
LINK_ENTITY_TO_CLASS( xen_ttrigger, CXenTreeTrigger );

CXenTreeTrigger *CXenTreeTrigger :: TriggerCreate( edict_t *pOwner, const Vector &position )
{
	CXenTreeTrigger *pTrigger = GetClassPtr( (CXenTreeTrigger *)NULL );
	pTrigger->pev->origin = position;
	pTrigger->pev->classname = MAKE_STRING("xen_ttrigger");
	pTrigger->pev->solid = SOLID_TRIGGER;
	pTrigger->pev->movetype = MOVETYPE_NONE;
	pTrigger->pev->owner = pOwner;

	return pTrigger;
}


void CXenTreeTrigger::Touch( CBaseEntity *pOther )
{
	if ( pev->owner )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(pev->owner);
		pEntity->Touch( pOther );
	}
}


#define TREE_AE_ATTACK		1

class CXenTree : public CActAnimating
{
public:
	void		Spawn( void );
	void		Precache( void );
	void		Touch( CBaseEntity *pOther );
	void		Think( void );
	int			TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { Attack(); return 0; }
	void		HandleAnimEvent( MonsterEvent_t *pEvent );
	void		Attack( void );	
	int			Classify( void ) { return CLASS_BARNACLE; }

	virtual int	Save( CSave &save );
	virtual int	Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

private:
	CXenTreeTrigger	*m_pTrigger;
};

LINK_ENTITY_TO_CLASS( xen_tree, CXenTree );

TYPEDESCRIPTION	CXenTree::m_SaveData[] = 
{
	DEFINE_FIELD( CXenTree, m_pTrigger, FIELD_CLASSPTR ),
};

IMPLEMENT_SAVERESTORE( CXenTree, CActAnimating );

void CXenTree :: Spawn( void )
{
	Precache();

	SET_MODEL( ENT(pev), "models/tree.mdl" );
	pev->movetype	= MOVETYPE_NONE;
	pev->solid		= SOLID_BBOX;

	pev->takedamage = DAMAGE_YES;

	UTIL_SetSize( pev, Vector(-30,-30,0), Vector(30,30,188));
	SetActivity( ACT_IDLE );
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT(0,255);
	pev->framerate = RANDOM_FLOAT( 0.7, 1.4 );

	Vector triggerPosition;
	UTIL_MakeVectorsPrivate( pev->angles, triggerPosition, NULL, NULL );
	triggerPosition = pev->origin + (triggerPosition * 64);
	// Create the trigger
	m_pTrigger = CXenTreeTrigger::TriggerCreate( edict(), triggerPosition );
	UTIL_SetSize( m_pTrigger->pev, Vector( -24, -24, 0 ), Vector( 24, 24, 128 ) );
}

const char *CXenTree::pAttackHitSounds[] = 
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char *CXenTree::pAttackMissSounds[] = 
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

void CXenTree :: Precache( void )
{
	PRECACHE_MODEL( "models/tree.mdl" );
	PRECACHE_MODEL( XEN_PLANT_GLOW_SPRITE );
	PRECACHE_SOUND_ARRAY( pAttackHitSounds );
	PRECACHE_SOUND_ARRAY( pAttackMissSounds );
}


void CXenTree :: Touch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() && FClassnameIs( pOther->pev, "monster_bigmomma" ) )
		return;

	Attack();
}


void CXenTree :: Attack( void )
{
	if ( GetActivity() == ACT_IDLE )
	{
		SetActivity( ACT_MELEE_ATTACK1 );
		pev->framerate = RANDOM_FLOAT( 1.0, 1.4 );
		EMIT_SOUND_ARRAY_DYN( CHAN_WEAPON, pAttackMissSounds );
	}
}


void CXenTree :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case TREE_AE_ATTACK:
		{
			CBaseEntity *pList[8];
			BOOL sound = FALSE;
			int count = UTIL_EntitiesInBox( pList, 8, m_pTrigger->pev->absmin, m_pTrigger->pev->absmax, FL_MONSTER|FL_CLIENT );
			Vector forward;

			UTIL_MakeVectorsPrivate( pev->angles, forward, NULL, NULL );

			for ( int i = 0; i < count; i++ )
			{
				if ( pList[i] != this )
				{
					if ( pList[i]->pev->owner != edict() )
					{
						sound = TRUE;
						pList[i]->TakeDamage( pev, pev, 25, DMG_CRUSH | DMG_SLASH );
						pList[i]->pev->punchangle.x = 15;
						pList[i]->pev->velocity = pList[i]->pev->velocity + forward * 100;
					}
				}
			}
					
			if ( sound )
			{
				EMIT_SOUND_ARRAY_DYN( CHAN_WEAPON, pAttackHitSounds );
			}
		}
		return;
	}

	CActAnimating::HandleAnimEvent( pEvent );
}

void CXenTree :: Think( void )
{
	float flInterval = StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents( flInterval );

	switch( GetActivity() )
	{
	case ACT_MELEE_ATTACK1:
		if ( m_fSequenceFinished )
		{
			SetActivity( ACT_IDLE );
			pev->framerate = RANDOM_FLOAT( 0.6, 1.4 );
		}
		break;

	default:
	case ACT_IDLE:
		break;

	}
}


// UNDONE:	These need to smoke somehow when they take damage
//			Touch behavior?
//			Cause damage in smoke area

//
// Spores
//
class CXenSpore : public CActAnimating
{
public:
	void		Spawn( void );
	void		Precache( void );
	void		Touch( CBaseEntity *pOther );
	void		Think( void );
	int			TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { Attack(); return 0; }
//	void		HandleAnimEvent( MonsterEvent_t *pEvent );
	void		Attack( void ) {}

	static const char *pModelNames[];
};

class CXenSporeSmall : public CXenSpore
{
	void		Spawn( void );
};

class CXenSporeMed : public CXenSpore
{
	void		Spawn( void );
};

class CXenSporeLarge : public CXenSpore
{
	void		Spawn( void );

	static const Vector m_hullSizes[];
};

// Fake collision box for big spores
class CXenHull : public CPointEntity
{
public:
	static CXenHull	*CreateHull( CBaseEntity *source, const Vector &mins, const Vector &maxs, const Vector &offset );
	int			Classify( void ) { return CLASS_BARNACLE; }
};

CXenHull *CXenHull :: CreateHull( CBaseEntity *source, const Vector &mins, const Vector &maxs, const Vector &offset )
{
	CXenHull *pHull = GetClassPtr( (CXenHull *)NULL );

	UTIL_SetOrigin( pHull->pev, source->pev->origin + offset );
	SET_MODEL( pHull->edict(), STRING(source->pev->model) );
	pHull->pev->solid = SOLID_BBOX;
	pHull->pev->classname = MAKE_STRING("xen_hull");
	pHull->pev->movetype = MOVETYPE_NONE;
	pHull->pev->owner = source->edict();
	UTIL_SetSize( pHull->pev, mins, maxs );
	pHull->pev->renderamt = 0;
	pHull->pev->rendermode = kRenderTransTexture;
	//	pHull->pev->effects = EF_NODRAW;

	return pHull;
}


LINK_ENTITY_TO_CLASS( xen_spore_small, CXenSporeSmall );
LINK_ENTITY_TO_CLASS( xen_spore_medium, CXenSporeMed );
LINK_ENTITY_TO_CLASS( xen_spore_large, CXenSporeLarge );
LINK_ENTITY_TO_CLASS( xen_hull, CXenHull );

void CXenSporeSmall::Spawn( void )
{
	pev->skin = 0;
	CXenSpore::Spawn();
	UTIL_SetSize( pev, Vector(-16,-16,0), Vector(16,16,64));
}
void CXenSporeMed::Spawn( void )
{
	pev->skin = 1;
	CXenSpore::Spawn();
	UTIL_SetSize( pev, Vector(-40,-40,0), Vector(40,40,120));
}


// I just eyeballed these -- fill in hulls for the legs
const Vector CXenSporeLarge::m_hullSizes[] = 
{
	Vector( 90, -25, 0 ),
	Vector( 25, 75, 0 ),
	Vector( -15, -100, 0 ),
	Vector( -90, -35, 0 ),
	Vector( -90, 60, 0 ),
};

void CXenSporeLarge::Spawn( void )
{
	pev->skin = 2;
	CXenSpore::Spawn();
	UTIL_SetSize( pev, Vector(-48,-48,110), Vector(48,48,240));
	
	Vector forward, right;

	UTIL_MakeVectorsPrivate( pev->angles, forward, right, NULL );

	// Rotate the leg hulls into position
	for ( int i = 0; i < ARRAYSIZE(m_hullSizes); i++ )
		CXenHull :: CreateHull( this, Vector(-12, -12, 0 ), Vector( 12, 12, 120 ), (m_hullSizes[i].x * forward) + (m_hullSizes[i].y * right) );
}

void CXenSpore :: Spawn( void )
{
	Precache();

	SET_MODEL( ENT(pev), pModelNames[pev->skin] );
	pev->movetype	= MOVETYPE_NONE;
	pev->solid		= SOLID_BBOX;
	pev->takedamage = DAMAGE_YES;

//	SetActivity( ACT_IDLE );
	pev->sequence = 0;
	pev->frame = RANDOM_FLOAT(0,255);
	pev->framerate = RANDOM_FLOAT( 0.7, 1.4 );
	ResetSequenceInfo( );
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT( 0.1, 0.4 );	// Load balance these a bit
}

const char *CXenSpore::pModelNames[] = 
{
	"models/fungus(small).mdl",
	"models/fungus.mdl",
	"models/fungus(large).mdl",
};


void CXenSpore :: Precache( void )
{
	PRECACHE_MODEL( (char *)pModelNames[pev->skin] );
}


void CXenSpore :: Touch( CBaseEntity *pOther )
{
}


void CXenSpore :: Think( void )
{
	float flInterval = StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

#if 0
	DispatchAnimEvents( flInterval );

	switch( GetActivity() )
	{
	default:
	case ACT_IDLE:
		break;

	}
#endif
}


