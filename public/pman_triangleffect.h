#ifndef TRIANGLEEFFECT_H__
#define TRIANGLEEFFECT_H__

#ifdef _WIN32
#pragma once
#endif

#define TRI_COLLIDEWORLD	0x00000020
#define TRI_COLLIDEALL		0x00001000 // will collide with world and slideboxes
#define TRI_COLLIDEKILL		0x00004000 // tent is removed upon collision with anything
#define TRI_SPIRAL			0x00008000
#define TRI_ANIMATEDIE		0x00016000 //animate once and then die
#define TRI_WATERTRACE		0x00032000


#define CULL_FRUSTUM_POINT ( 1 << 0 )
#define CULL_FRUSTUM_SPHERE ( 1 << 1 )
#define CULL_FRUSTUM_PLANE ( 1 << 2 )
#define CULL_PVS ( 1 << 3 )

#define LIGHT_NONE ( 1 << 4 )
#define LIGHT_COLOR ( 1 << 5 )
#define LIGHT_INTENSITY ( 1 << 6 )

#define RENDER_FACEPLAYER ( 1 << 7 ) // m_vAngles == Player view angles
#define RENDER_FACEPLAYER_ROTATEZ ( 1 << 8 ) //Just like above but m_vAngles.z is untouched so the sprite can rotate.


#include "pman_particlemem.h" 

//pure virtual baseclass
class CCoreTriangleEffect
{
private:
	int	   m_iRenderFlags;
	float  m_flNextPVSCheck;
	bool   m_bInPVS;

	int    m_iCollisionFlags;
	float  m_flPlayerDistance; //Used for sorting the particles, DO NOT TOUCH.
	
public:

	void * operator new(size_t size)
	{
		// Requested size should match size of class.
        if ( size != sizeof( CCoreTriangleEffect ) )
#ifdef _WIN32
             throw "Error in requested size of new particle class instance.";
#else
			return NULL;
#endif
		
      return((CCoreTriangleEffect *) CMiniMem::Instance()->newBlock());

	}//this asks for a new block of memory from the MiniMen class
		
	virtual void Think( float time ) = 0;
	virtual bool CheckVisibility ( void ) = 0;
	virtual void Draw( void ) = 0;
	virtual void Animate( float time ) = 0;
	virtual void AnimateAndDie( float time ) = 0;
	virtual void Expand( float time ) = 0;
	virtual void Contract( float time ) = 0;
	virtual void Fade( float time ) = 0;
	virtual void Spin( float time ) = 0;
	virtual void CalculateVelocity( float time ) = 0;
	virtual void CheckCollision( float time ) = 0;
	virtual void Touch(Vector pos, Vector normal, int index) = 0;
	virtual void Die ( void ) = 0;
	virtual void InitializeSprite( Vector org, Vector normal, model_s * sprite, float size, float brightness ) = 0;
	virtual void Force ( void ) = 0;

	float m_flSize; //scale of object
	float m_flScaleSpeed; //speed at which object expands
	float m_flContractSpeed; //speed at which object expands
	
	float m_flStretchX;
	float m_flStretchY;

	float m_flBrightness; //transparency of object
	float m_flFadeSpeed; //speed at which object fades

	float m_flTimeCreated; //time object was instanced
	float m_flDieTime; //time to remove an object

	float m_flGravity; //how effected by gravity is this object
	float m_flAfterDampGrav;
	float m_flDampingVelocity;
	float m_flDampingTime;

	int	  m_iFramerate;
	int   m_iNumFrames;
	int	  m_iFrame;
	int	  m_iRendermode;

	Vector m_vOrigin; //object's position
	Vector m_vAngles; //normal angles of object
	
	Vector m_vAVelocity;

	Vector m_vVelocity;
	
	Vector m_vLowLeft;
	Vector m_vLowRight; 
	Vector m_vTopLeft; 

	Vector m_vColor;
	float  m_flMass;

	model_s * m_pTexture;

	float  m_flBounceFactor;

	char   m_szClassname[32];

	bool   m_bInWater;
	bool   m_bAffectedByForce;

	int   m_iAfterDampFlags;

	void   SetLightFlag ( int iFlag )
	{
		m_iRenderFlags &= ~( LIGHT_NONE | LIGHT_INTENSITY | LIGHT_COLOR );
		m_iRenderFlags |= iFlag;
	}

	void   SetCullFlag( int iFlag )
	{
		m_iRenderFlags &= ~( CULL_PVS | CULL_FRUSTUM_POINT | CULL_FRUSTUM_PLANE | CULL_FRUSTUM_SPHERE );
		m_iRenderFlags |= iFlag;
	}

	int    GetRenderFlags( void )
	{
		 return m_iRenderFlags;
	}

	bool   GetParticlePVS ( void )
	{
		return m_bInPVS;
	}

	void   SetParticlePVS ( bool bPVSStat )
	{
		m_bInPVS = bPVSStat;
	}
	
	float GetNextPVSCheck( void )
	{
		 return m_flNextPVSCheck;
	}

	void SetNextPVSCheck( float flTime )
	{
		 m_flNextPVSCheck = flTime;
	}
	
	void SetCollisionFlags ( int iFlag )
	{
		 m_iCollisionFlags |= iFlag;
	}

	void ClearCollisionFlags ( int iFlag )
	{
		 m_iCollisionFlags &= ~iFlag;
	}
	
	int  GetCollisionFlags ( void )
	{
		return m_iCollisionFlags;
	}

	void SetRenderFlag( int iFlag )
	{
		m_iRenderFlags |= iFlag;
	}

	float GetPlayerDistance ( void ) {	return m_flPlayerDistance; 	}
	void  SetPlayerDistance ( float flDistance ) {	m_flPlayerDistance = flDistance; 	}

protected:
	float  m_flOriginalSize;
	Vector m_vOriginalAngles;
	float  m_flOriginalBrightness;
	Vector m_vPrevOrigin;

	float m_flNextCollisionTime;

protected:
	static bool CheckSize(int size)
	{
		// This check will help prevent a class frome being defined later,
        //  that is larger than the max size MemoryPool is expecting,
        //  from being successfully allocated.
        if (size > (unsigned long) CMiniMem::Instance()->MaxBlockSize())
		{
#ifdef _WIN32
            throw "New particle class is larger than memory pool max size, update lMaxParticleClassSize() function.";
#endif
			return(false);
		}

		return(true);
	}
};


#endif//TRIANGLEEFFECT_H__
