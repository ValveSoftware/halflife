//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "vgui_helpers.h"


using namespace vgui;


void AlignPanel(Panel *pChild, Panel *pParent, int alignment)
{
	int w, h, cw, ch;
	pParent->getSize(w, h);
	pChild->getSize(cw, ch);

	int xCenter = (w - cw) / 2;
	int yCenter = (h - ch) / 2;

	if(alignment == Label::a_west)
		pChild->setPos(0, yCenter);
	else if(alignment == Label::a_northwest)
		pChild->setPos(0,0);
	else if(alignment == Label::a_north)
		pChild->setPos(xCenter, 0);
	else if(alignment == Label::a_northeast)
		pChild->setPos(w - cw, 0);
	else if(alignment == Label::a_east)
		pChild->setPos(w - cw, yCenter);
	else if(alignment == Label::a_southeast)
		pChild->setPos(w - cw, h - ch);
	else if(alignment == Label::a_south)
		pChild->setPos(xCenter, h - ch);
	else if(alignment == Label::a_southwest)
		pChild->setPos(0, h - ch);
	else if(alignment == Label::a_center)
		pChild->setPos(xCenter, yCenter);
}




