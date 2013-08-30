//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_Point_H
#define VGUI_Point_H
#ifdef _WIN32
#pragma once
#endif

#include <VGUI.h>

namespace vgui
{

//-----------------------------------------------------------------------------
// Purpose: Basic handler for a Points in 2 dimensions
//			This class is fully inline
//-----------------------------------------------------------------------------
class Point
{
public:
	// constructors
	Point()
	{
		SetPoint(0, 0);
	}
	Point(int x,int y)
	{
		SetPoint(x,y);
	}

	void SetPoint(int x1, int y1)
	{
		x=x1;
		y=y1;	
	}

	void GetPoint(int& x1,int& y1) const
	{
		x1 = x;
		y1 = y;
	
	}

	bool operator == (Point &rhs) const
	{
		return ( x == rhs.x && y==rhs.y);
	}

private:
	int x,y;
};

}

#endif // VGUI_Point_H



//## <b>class Point</b>
//## Point is a class to handle Points in VGUI.

//## // The default Point is (0,0,0,0)
//## Point()
//##
//## // A Point may be created with x,yvalues.
//## Point(int x,int y)
//##
//## // Set the x,y components of the Point.
//## void SetPoint(int x1, int y1)
//##
//## // Get the position of the Point
//## void GetPoint(int& x1,int& y1) const
//##
//## // Point classes can be set equal to each other.
//## bool operator == (Point &rhs) const
//##
//##
//## <hr>
//##
//##
//## <method>Point()
//## Description: Default constructor. The default Point is (0,0,0,0)
//##	
//##
//## <method>Point(int x,int y)
//## Description:
//##	A Point may be created with a x,y position.
//## Arguments:
//##	x - x position (horizontal axis) (0-65535)
//##	y - y position (vertical axis) (0-65535)
//## 
//## 
//## <method>void SetPoint(int x1, int x1)
//## Description:
//##	Set the position of the Point.
//## Arguments:
//##	x - x position (horizontal axis) (0-65535)
//##	y - y position (vertical axis) (0-65535)
//## 
//## 
//## <method>void GetPoint(int& x1,int& y1) const
//## Description:
//##	Get the x,y components of a Point
//## Arguments:
//##	x - x position (horizontal axis) (0-65535)
//##	y - y position (vertical axis) (0-65535)	
//##  
//## <method>bool operator == (Point &rhs) const
//## Description:
//##	Point classes can be set equal to each other.
//##
//##
