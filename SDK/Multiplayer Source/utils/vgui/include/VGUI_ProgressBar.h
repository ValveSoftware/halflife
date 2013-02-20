//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_PROGRESSBAR_H
#define VGUI_PROGRESSBAR_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class VGUIAPI ProgressBar : public Panel
{
private:
	int   _segmentCount;
	float _progress;
public:
	ProgressBar(int segmentCount);
protected:
	virtual void paintBackground();
public:
	virtual void setProgress(float progress);
	virtual int  getSegmentCount();
};

}

#endif