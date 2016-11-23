//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_DESKTOPICON_H
#define VGUI_DESKTOPICON_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class MiniApp;
class Image;
class Desktop;

class VGUIAPI DesktopIcon : public Panel
{
public:
	DesktopIcon(MiniApp* miniApp,Image* image);
public:
	virtual void doActivate();
	virtual void setImage(Image* image);
public: //bullshit public
	virtual void     setDesktop(Desktop* desktop);
	virtual MiniApp* getMiniApp();
protected:
	virtual void paintBackground();
protected:
	Desktop* _desktop;
	MiniApp* _miniApp;
	Image*   _image;
};

}

#endif