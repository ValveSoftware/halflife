//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include<stdio.h>
#include"vgui_ControlConfigPanel.h"
#include<VGUI_HeaderPanel.h>
#include<VGUI_TablePanel.h>
#include<VGUI_Label.h>
#include<VGUI_ScrollPanel.h>
#include<VGUI_Scheme.h>
#include<VGUI_DataInputStream.h>
#include<VGUI.h>
#include<VGUI_TextEntry.h>

using namespace vgui;

namespace
{
class FooTablePanel : public TablePanel
{
private:
	Label*              _label;
	TextEntry*          _textEntry;
	ControlConfigPanel* _controlConfigPanel;
public:
	FooTablePanel(ControlConfigPanel* controlConfigPanel,int x,int y,int wide,int tall,int columnCount) : TablePanel(x,y,wide,tall,columnCount)
	{
		_controlConfigPanel=controlConfigPanel;
		_label=new Label("You are a dumb monkey",0,0,100,20);
		_label->setBgColor(Scheme::sc_primary3);
		_label->setFgColor(Scheme::sc_primary1);
		_label->setFont(Scheme::sf_primary3);

		_textEntry=new TextEntry("",0,0,100,20);
		//_textEntry->setFont(Scheme::sf_primary3);
	}
public:
	virtual int getRowCount()
	{
		return _controlConfigPanel->GetCVarCount();
	}
	virtual int getCellTall(int row)
	{
		return 12;
	}
	virtual Panel* getCellRenderer(int column,int row,bool columnSelected,bool rowSelected,bool cellSelected)
	{
		char cvar[128],desc[128],bind[128],bindAlt[128];
		_controlConfigPanel->GetCVar(row,cvar,128,desc,128);

		if(cellSelected)
		{
			_label->setBgColor(Scheme::sc_primary1);
			_label->setFgColor(Scheme::sc_primary3);
		}
		else
		if(rowSelected)
		{
			_label->setBgColor(Scheme::sc_primary2);
			_label->setFgColor(Scheme::sc_primary1);
		}
		else
		{
			_label->setBgColor(Scheme::sc_primary3);
			_label->setFgColor(Scheme::sc_primary1);
		}

		switch(column)
		{
			case 0:
			{
				_label->setText(desc);
				_label->setContentAlignment(Label::a_west);
				break;
			}
			case 1:
			{
				_controlConfigPanel->GetCVarBind(cvar,bind,128,bindAlt,128);
				_label->setText(bind);
				_label->setContentAlignment(Label::a_center);
				break;
			}
			case 2:
			{
				_controlConfigPanel->GetCVarBind(cvar,bind,128,bindAlt,128);
				_label->setText(bindAlt);
				_label->setContentAlignment(Label::a_center);
				break;
			}
			default:
			{
				_label->setText("");
				break;
			}
		}

		return _label;
	}
	virtual Panel* startCellEditing(int column,int row)
	{
		_textEntry->setText("Goat",strlen("Goat"));
		_textEntry->requestFocus();
		return _textEntry;
	}
};
}

ControlConfigPanel::ControlConfigPanel(int x,int y,int wide,int tall) : Panel(x,y,wide,tall)
{
	setPaintBorderEnabled(false);
	setPaintBackgroundEnabled(false);
	setPaintEnabled(false);

	_actionLabel=new Label("Action");
	_actionLabel->setBgColor(Scheme::sc_primary3);
	_actionLabel->setFgColor(Scheme::sc_primary3);

	_keyButtonLabel=new Label("Key / Button");
	_keyButtonLabel->setBgColor(Scheme::sc_primary3);
	_keyButtonLabel->setFgColor(Scheme::sc_primary3);

	_alternateLabel=new Label("Alternate");
	_alternateLabel->setBgColor(Scheme::sc_primary3);
	_alternateLabel->setFgColor(Scheme::sc_primary3);
	
	_headerPanel=new HeaderPanel(0,0,wide,20);
	_headerPanel->setParent(this);

	_headerPanel->addSectionPanel(_actionLabel);
	_headerPanel->addSectionPanel(_keyButtonLabel);
	_headerPanel->addSectionPanel(_alternateLabel);
	
	_headerPanel->setSliderPos( 0, wide/2 );
	_headerPanel->setSliderPos( 1, (wide/2) + (wide/4) );
	_headerPanel->setSliderPos( 2, wide );

	_scrollPanel=new ScrollPanel(0,20,wide,tall-20);
	_scrollPanel->setParent(this);
	_scrollPanel->setPaintBorderEnabled(false);
	_scrollPanel->setPaintBackgroundEnabled(false);
	_scrollPanel->setPaintEnabled(false);
	_scrollPanel->getClient()->setPaintBorderEnabled(false);
	_scrollPanel->getClient()->setPaintBackgroundEnabled(false);
	_scrollPanel->getClient()->setPaintEnabled(false);
	_scrollPanel->setScrollBarVisible(false,true);

	_tablePanel=new FooTablePanel(this,0,0,_scrollPanel->getClient()->getWide(),800, 3);
	_tablePanel->setParent(_scrollPanel->getClient());
	_tablePanel->setHeaderPanel(_headerPanel);
	_tablePanel->setBgColor(Color(200,0,0,255));
	_tablePanel->setFgColor(Color(Scheme::sc_primary2));
	_tablePanel->setGridVisible(true,true);
	_tablePanel->setGridSize(1,1);
}

void ControlConfigPanel::AddCVar(const char* cvar,const char* desc)
{
	_cvarDar.addElement(vgui_strdup(cvar));
	_descDar.addElement(vgui_strdup(desc));
}

int ControlConfigPanel::GetCVarCount()
{
	return _cvarDar.getCount();
}

void ControlConfigPanel::GetCVar(int index,char* cvar,int cvarLen,char* desc,int descLen)
{
	vgui_strcpy(cvar,cvarLen,_cvarDar[index]);
	vgui_strcpy(desc,descLen,_descDar[index]);
}

void ControlConfigPanel::AddCVarFromInputStream(InputStream* is)
{
	if(is==null)
	{
		return;
	}
	
	DataInputStream dis(is);

	bool success;

	while(1)
	{
		char buf[256],cvar[128],desc[128];
		dis.readLine(buf,256,success);
		if(!success)
		{
			break;
		}
		if(sscanf(buf,"\"%[^\"]\" \"%[^\"]\"",cvar,desc)==2)
		{
			AddCVar(cvar,desc);
		}
	}
}

void ControlConfigPanel::GetCVarBind(const char* cvar,char* bind,int bindLen,char* bindAlt,int bindAltLen)
{
	sprintf(bind,"%s : Bind",cvar);
	sprintf(bindAlt,"%s : BindAlt",cvar);
}

void ControlConfigPanel::SetCVarBind(const char* cvar,const char* bind,const char* bindAlt)
{
}

