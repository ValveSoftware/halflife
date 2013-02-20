//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_CONFIGWIZARD_H
#define VGUI_CONFIGWIZARD_H

#include<VGUI.h>
#include<VGUI_Panel.h>

namespace vgui
{

class TreeFolder;
class Panel;
class Button;

class VGUIAPI ConfigWizard : public Panel
{
public:
	ConfigWizard(int x,int y,int wide,int tall);
public:
	virtual void        setSize(int wide,int tall);
	virtual Panel*      getClient();
	virtual TreeFolder* getFolder();
protected:
	TreeFolder* _treeFolder;
	Panel*      _client;
	Button*     _okButton;
	Button*     _cancelButton;
	Button*     _applyButton;
	Button*     _helpButton;
};

}

#endif