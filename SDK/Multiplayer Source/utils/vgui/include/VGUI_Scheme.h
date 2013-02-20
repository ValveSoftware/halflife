//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_SCHEME_H
#define VGUI_SCHEME_H

#include<VGUI.h>


namespace vgui
{

class Font;
class Cursor;

class VGUIAPI Scheme
{
public:
	enum SchemeColor
	{
		sc_user=0,
		sc_black,
		sc_white,
		sc_primary1,
		sc_primary2,
		sc_primary3,
		sc_secondary1,
		sc_secondary2,
		sc_secondary3,
		sc_last,
	};
	enum SchemeFont
	{
		sf_user=0,
		sf_primary1,
		sf_primary2,
		sf_primary3,
		sf_secondary1,
		sf_last,
	};
	enum SchemeCursor
	{
		scu_user=0,
		scu_none,
		scu_arrow,
		scu_ibeam,
		scu_hourglass,
		scu_crosshair,
		scu_up,
		scu_sizenwse,
		scu_sizenesw,
		scu_sizewe,
		scu_sizens,
		scu_sizeall,
		scu_no,
		scu_hand,
		scu_last,
	};
public:
	Scheme();
public:
	virtual void    setColor(SchemeColor sc,int r,int g,int b,int a);
	virtual void    getColor(SchemeColor sc,int& r,int& g,int& b,int& a);
	virtual void    setFont(SchemeFont sf,Font* font);
	virtual Font*   getFont(SchemeFont sf);
	virtual void    setCursor(SchemeCursor sc,Cursor* cursor);
	virtual Cursor* getCursor(SchemeCursor sc);
protected:
	int     _color[sc_last][4];
    Font*   _font[sf_last];
	Cursor* _cursor[scu_last];
    friend class Panel;
    friend class Canvas;
};

}

#endif