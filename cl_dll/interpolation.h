/************ (C) Copyright 2003 Valve, L.L.C. All rights reserved. ***********
**
** The copyright to the contents herein is the property of Valve, L.L.C.
** The contents may be used and/or copied only with the written permission of
** Valve, L.L.C., or in accordance with the terms and conditions stipulated in
** the agreement/contract under which the contents have been supplied.
**
*******************************************************************************
**
** Contents:
**
**	      interpolation.h: Bezier inpolation classes
**
******************************************************************************/

#ifndef INTERPOLATION_H
#define INTERPOLATION_H
#ifdef _WIN32
#pragma once
#endif


//  interpolation class
class CInterpolation  
{
public:

	CInterpolation();
	virtual ~CInterpolation();

	void SetWaypoints(vec3_t * prev, vec3_t start, vec3_t end, vec3_t * next);
	void SetViewAngles( vec3_t start, vec3_t end );
	void SetFOVs(float start, float end);
	void SetSmoothing(bool start, bool end);
	
	// get interpolated point 0 =< t =< 1, 0 = start, 1 = end
	void Interpolate(float t, vec3_t &point, vec3_t &angle, float * fov);
	
protected:

	void BezierInterpolatePoint( float t, vec3_t &point );
	void InterpolateAngle( float t, vec3_t &angle );

	vec3_t	m_StartPoint;
	vec3_t	m_EndPoint;
	vec3_t	m_StartAngle;
	vec3_t	m_EndAngle;
	vec3_t	m_Center;
	float	m_StartFov;
	float	m_EndFov;
		
	bool	m_SmoothStart;
	bool	m_SmoothEnd;
};

#endif // INTERPOLATION_H