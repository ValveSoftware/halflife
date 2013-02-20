
#include"vgui_int.h"
#include<VGUI_Label.h>
#include<VGUI_BorderLayout.h>
#include<VGUI_LineBorder.h>
#include<VGUI_SurfaceBase.h>
#include<VGUI_TextEntry.h>
#include<VGUI_ActionSignal.h>
#include<string.h>
#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ControlConfigPanel.h"

namespace
{

class TexturePanel : public Panel , public ActionSignal
{
private:
	int _bindIndex;
	TextEntry* _textEntry;
public:
	TexturePanel() : Panel(0,0,256,276)
	{
		_bindIndex=2700;
		_textEntry=new TextEntry("2700",0,0,128,20);
		_textEntry->setParent(this);
		_textEntry->addActionSignal(this);
	}
public:
	virtual bool isWithin(int x,int y)
	{
		return _textEntry->isWithin(x,y);
	}
public:
	virtual void actionPerformed(Panel* panel)
	{
		char buf[256];
		_textEntry->getText(0,buf,256);
		sscanf(buf,"%d",&_bindIndex);
	}
protected:
	virtual void paintBackground()
	{
			Panel::paintBackground();
			
			int wide,tall;
			getPaintSize(wide,tall);
		
			drawSetColor(0,0,255,0);
			drawSetTexture(_bindIndex);
			drawTexturedRect(0,19,257,257);
	}

};

}

using namespace vgui;

void VGui_ViewportPaintBackground(int extents[4])
{
	gEngfuncs.VGui_ViewportPaintBackground(extents);
}

void* VGui_GetPanel()
{
	return (Panel*)gEngfuncs.VGui_GetPanel();
}

void VGui_Startup()
{
	Panel* root=(Panel*)VGui_GetPanel();
	root->setBgColor(128,128,0,0);
	//root->setNonPainted(false);
	//root->setBorder(new LineBorder());
	root->setLayout(new BorderLayout(0));

	
	//root->getSurfaceBase()->setEmulatedCursorVisible(true);

	if (gViewPort != NULL)
	{
//		root->removeChild(gViewPort);

		// free the memory
//		delete gViewPort;
//		gViewPort = NULL;

		gViewPort->Initialize();
	}
	else
	{
		gViewPort = new TeamFortressViewport(0,0,root->getWide(),root->getTall());
		gViewPort->setParent(root);
	}

	/*
	TexturePanel* texturePanel=new TexturePanel();
	texturePanel->setParent(gViewPort);
	*/

}

void VGui_Shutdown()
{
	delete gViewPort;
	gViewPort = NULL;
}





