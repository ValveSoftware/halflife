
#ifndef CONTROLCONFIGPANEL_H
#define CONTROLCONFIGPANEL_H

#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{
class HeaderPanel;
class TablePanel;
class ScrollPanel;
class InputStream;
class Label;
}

class ControlConfigPanel : public vgui::Panel
{
private:
	vgui::HeaderPanel* _headerPanel;
	vgui::TablePanel*  _tablePanel;
	vgui::ScrollPanel* _scrollPanel;
	vgui::Dar<char*>   _cvarDar;
	vgui::Dar<char*>   _descDar;
	vgui::Label*       _actionLabel;
	vgui::Label*       _keyButtonLabel;
	vgui::Label*       _alternateLabel;
public:
	ControlConfigPanel(int x,int y,int wide,int tall);
public:
	void AddCVar(const char* cvar,const char* desc);
	void AddCVarFromInputStream(vgui::InputStream* is);
	int  GetCVarCount();
	void GetCVar(int index,char* cvar,int cvarLen,char* desc,int descLen);
	void GetCVarBind(const char* cvar,char* bind,int bindLen,char* bindAlt,int bindAltLen);
	void SetCVarBind(const char* cvar,const char* bind,const char* bindAlt);
};



#endif