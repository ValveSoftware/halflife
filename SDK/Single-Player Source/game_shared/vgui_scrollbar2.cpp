//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================


#include "VGUI_ScrollBar2.h"
#include "VGUI_Slider2.h"
#include "vgui_loadtga.h"

#include<VGUI_IntChangeSignal.h>
#include<VGUI_Button.h>
#include<VGUI_ActionSignal.h>
#include<VGUI_LineBorder.h>

using namespace vgui;


namespace
{
class FooDefaultScrollBarIntChangeSignal : public IntChangeSignal
{
public:
	FooDefaultScrollBarIntChangeSignal(ScrollBar2* scrollBar)
	{
		_scrollBar=scrollBar;
	}
	virtual void intChanged(int value,Panel* panel)
	{
		_scrollBar->fireIntChangeSignal();
	}
protected:
	ScrollBar2* _scrollBar;
};

class FooDefaultButtonSignal : public ActionSignal
{
public:
	ScrollBar2* _scrollBar;
	int        _buttonIndex;
public:
	FooDefaultButtonSignal(ScrollBar2* scrollBar,int buttonIndex)
	{
		_scrollBar=scrollBar;
		_buttonIndex=buttonIndex;
	}
public:
	virtual void actionPerformed(Panel* panel)
	{
		_scrollBar->doButtonPressed(_buttonIndex);
	}
};

}

//-----------------------------------------------------------------------------
// Purpose: Default scrollbar button
//			Draws in new scoreboard style
//-----------------------------------------------------------------------------
class ScrollBarButton : public Button
{
private:
	LineBorder m_Border;

public:
	ScrollBarButton(const char *filename, int x, int y, int wide, int tall) : m_Border(Color(60, 60, 60, 0)), Button("", x, y, wide, tall)
	{
		Image *image = vgui_LoadTGA(filename);
		if (image)
		{
			image->setColor(Color(140, 140, 140, 0));
			setImage(image);
		}

		setBorder(&m_Border);
	}

	virtual void paintBackground()
	{
		int wide,tall;
		getPaintSize(wide,tall);

		// fill the background
		drawSetColor(0, 0, 0, 0);
		drawFilledRect(0, 0, wide, tall);
	}
};




//-----------------------------------------------------------------------------
// Purpose: Constructor
// Input  : x - 
//			y - 
//			wide - 
//			tall - 
//			vertical - 
//-----------------------------------------------------------------------------
ScrollBar2::ScrollBar2(int x,int y,int wide,int tall,bool vertical) : Panel(x,y,wide,tall)
{
	_slider=null;
	_button[0]=null;
	_button[1]=null;
	
	if(vertical)
	{
		setSlider(new Slider2(0,wide-1,wide,(tall-(wide*2))+2,true));
		setButton(new ScrollBarButton("gfx/vgui/arrowup.tga",0,0,wide,wide),0);
		setButton(new ScrollBarButton("gfx/vgui/arrowdown.tga",0,tall-wide,wide,wide),1);
	}
	else
	{
		// untested code
		setSlider(new Slider2(tall,0,wide-(tall*2),tall,false));
		setButton(new ScrollBarButton("gfx/vgui/320_arrowlt.tga",0,0,tall+1,tall+1),0);
		setButton(new ScrollBarButton("gfx/vgui/320_arrowrt.tga",wide-tall,0,tall+1,tall+1),1);
	}

	setPaintBorderEnabled(true);
	setPaintBackgroundEnabled(true);
	setPaintEnabled(true);
	setButtonPressedScrollValue(15);

	validate();
 }

void ScrollBar2::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);

	if(_slider==null)
	{
		return;
	}

	if(_button[0]==null)
	{
		return;
	}

	if(_button[1]==null)
	{
		return;
	}

	getPaintSize(wide,tall);

	if(_slider->isVertical())
	{
		_slider->setBounds(0,wide,wide,tall-(wide*2));
		//_slider->setBounds(0,0,wide,tall);
		_button[0]->setBounds(0,0,wide,wide);
		_button[1]->setBounds(0,tall-wide,wide,wide);
	}
	else
	{
		_slider->setBounds(tall,0,wide-(tall*2),tall);
		//_slider->setBounds(0,0,wide,tall);
		_button[0]->setBounds(0,0,tall,tall);
		_button[1]->setBounds((wide-tall),0,tall,tall);
	}
}

void ScrollBar2::performLayout()
{
}

void ScrollBar2::setValue(int value)
{
	_slider->setValue(value);
}

int ScrollBar2::getValue()
{
	return _slider->getValue();
}

void ScrollBar2::addIntChangeSignal(IntChangeSignal* s)
{
	_intChangeSignalDar.putElement(s);
	_slider->addIntChangeSignal(new FooDefaultScrollBarIntChangeSignal(this));
}

void ScrollBar2::setRange(int min,int max)
{
	_slider->setRange(min,max);
}

void ScrollBar2::fireIntChangeSignal()
{
	for(int i=0;i<_intChangeSignalDar.getCount();i++)
	{
		_intChangeSignalDar[i]->intChanged(_slider->getValue(),this);
	}
}

bool ScrollBar2::isVertical()
{
	return _slider->isVertical();
}

bool ScrollBar2::hasFullRange()
{
	return _slider->hasFullRange();
}

//LEAK: new and old slider will leak
void ScrollBar2::setButton(Button* button,int index)
{
	if(_button[index]!=null)
	{
		removeChild(_button[index]);
	}
	_button[index]=button;
	addChild(_button[index]);

	_button[index]->addActionSignal(new FooDefaultButtonSignal(this,index));

	validate();

	//_button[index]->setVisible(false);
}

Button* ScrollBar2::getButton(int index)
{
	return _button[index];
}

//LEAK: new and old slider will leak
void ScrollBar2::setSlider(Slider2 *slider)
{
	if(_slider!=null)
	{
		removeChild(_slider);
	}
	_slider=slider;
	addChild(_slider);

	_slider->addIntChangeSignal(new FooDefaultScrollBarIntChangeSignal(this));

	validate();
}

Slider2 *ScrollBar2::getSlider()
{
	return _slider;
}

void ScrollBar2::doButtonPressed(int buttonIndex)
{
	if(buttonIndex==0)
	{
		_slider->setValue(_slider->getValue()-_buttonPressedScrollValue);
	}
	else
	{
		_slider->setValue(_slider->getValue()+_buttonPressedScrollValue);
	}

}

void ScrollBar2::setButtonPressedScrollValue(int value)
{
	_buttonPressedScrollValue=value;
}

void ScrollBar2::setRangeWindow(int rangeWindow)
{
	_slider->setRangeWindow(rangeWindow);
}

void ScrollBar2::setRangeWindowEnabled(bool state)
{
	_slider->setRangeWindowEnabled(state);
}

void ScrollBar2::validate()
{
	if(_slider!=null)
	{
		int buttonOffset=0;

		for(int i=0;i<2;i++)
		{
			if(_button[i]!=null)
			{
				if(_button[i]->isVisible())
				{
					if(_slider->isVertical())
					{					
						buttonOffset+=_button[i]->getTall();
					}
					else
					{
						buttonOffset+=_button[i]->getWide();
					}
				}
			}
		}

		_slider->setButtonOffset(buttonOffset);
	}

	int wide,tall;
	getSize(wide,tall);
	setSize(wide,tall);
}
