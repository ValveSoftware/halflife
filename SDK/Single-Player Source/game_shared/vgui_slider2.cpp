//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: New version of the slider bar
//
// $NoKeywords: $
//=============================================================================

#include "VGUI_Slider2.h"

#include<VGUI_InputSignal.h>
#include<VGUI_App.h>
#include<VGUI_IntChangeSignal.h>
#include<VGUI_MouseCode.h>

using namespace vgui;

namespace
{
class FooDefaultSliderSignal : public InputSignal
{
private:
	Slider2* _slider;
public:
	FooDefaultSliderSignal(Slider2* slider)
	{
		_slider=slider;
	}
public:
	void cursorMoved(int x,int y,Panel* panel)
	{
		_slider->privateCursorMoved(x,y,panel);
	}
	void cursorEntered(Panel* panel){}
	void cursorExited(Panel* panel){}
	void mouseDoublePressed(MouseCode code,Panel* panel){}
	void mousePressed(MouseCode code,Panel* panel)
	{
		_slider->privateMousePressed(code,panel);
	}
	void mouseReleased(MouseCode code,Panel* panel)
	{
		_slider->privateMouseReleased(code,panel);
	}
	void mouseWheeled(int delta,Panel* panel){}
	void keyPressed(KeyCode code,Panel* panel){}
	void keyTyped(KeyCode code,Panel* panel){}
	void keyReleased(KeyCode code,Panel* panel){}
	void keyFocusTicked(Panel* panel){}
};
}

Slider2::Slider2(int x,int y,int wide,int tall,bool vertical) : Panel(x,y,wide,tall)
{
	_vertical=vertical;	
	_dragging=false;
	_value=0;
	_range[0]=0;
	_range[1]=299;
	_rangeWindow=0;
	_rangeWindowEnabled=false;
	_buttonOffset=0;
	recomputeNobPosFromValue();
	addInputSignal(new FooDefaultSliderSignal(this));
}

void Slider2::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);
	recomputeNobPosFromValue();
}

bool Slider2::isVertical()
{
	return _vertical;
}

void Slider2::setValue(int value)
{
	int oldValue=_value;

	if(value<_range[0])
	{
		value=_range[0];
	}

	if(value>_range[1])
	{
		value=_range[1];
	}
	
	_value=value;
	recomputeNobPosFromValue();

	if(_value!=oldValue)
	{
		fireIntChangeSignal();
	}
}

int Slider2::getValue()
{
	return _value;
}

void Slider2::recomputeNobPosFromValue()
{
	int wide,tall;

	getPaintSize(wide,tall);

	float fwide=(float)wide;
	float ftall=(float)tall;
	float frange=(float)(_range[1]-_range[0]);
	float fvalue=(float)(_value-_range[0]);
	float fper=fvalue/frange;
	float frangewindow=(float)(_rangeWindow);
	
	if(frangewindow<0)
	{
		frangewindow=0;
	}

	if(!_rangeWindowEnabled)
	{
		frangewindow=frange;
	}

	if ( frangewindow > 0 )
	{
		if(_vertical)
		{
			float fnobsize=frangewindow;
			float freepixels = ftall - fnobsize;

			float firstpixel = freepixels * fper;

			_nobPos[0]=(int)( firstpixel );
			_nobPos[1]=(int)( firstpixel + fnobsize );

			if(_nobPos[1]>tall)
			{
				_nobPos[0]=tall-((int)fnobsize);
				_nobPos[1]=tall;
			}
		}
		else
		{
			float fnobsize=frangewindow;
			float freepixels = fwide - fnobsize;

			float firstpixel = freepixels * fper;

			_nobPos[0]=(int)( firstpixel );
			_nobPos[1]=(int)( firstpixel + fnobsize );

			if(_nobPos[1]>wide)
			{
				_nobPos[0]=wide-((int)fnobsize);
				_nobPos[1]=wide;
			}
		}
	}
	
	repaint();
}

void Slider2::recomputeValueFromNobPos()
{
	int wide,tall;
	getPaintSize(wide,tall);

	float fwide=(float)wide;
	float ftall=(float)tall;
	float frange=(float)(_range[1]-_range[0]);
	float fvalue=(float)(_value-_range[0]);
	float fnob=(float)_nobPos[0];
	float frangewindow=(float)(_rangeWindow);

	if(frangewindow<0)
	{
		frangewindow=0;
	}

	if(!_rangeWindowEnabled)
	{
		frangewindow=frange;
	}

	if ( frangewindow > 0 )
	{
		if(_vertical)
		{
			float fnobsize=frangewindow;
			fvalue=frange*(fnob/(ftall-fnobsize));
		}
		else
		{
			float fnobsize=frangewindow;
			fvalue=frange*(fnob/(fwide-fnobsize));
		}
	}
	// Take care of rounding issues.
	_value=(int)(fvalue+_range[0]+0.5);

	// Clamp final result
	_value = ( _value < _range[1] ) ? _value : _range[1];
}

bool Slider2::hasFullRange()
{
	int wide,tall;
	getPaintSize(wide,tall);

	float fwide=(float)wide;
	float ftall=(float)tall;
	float frange=(float)(_range[1]-_range[0]);
	float frangewindow=(float)(_rangeWindow);

	if(frangewindow<0)
	{
		frangewindow=0;
	}

	if(!_rangeWindowEnabled)
	{
		frangewindow=frange;
	}

	if ( frangewindow > 0 )
	{
		if(_vertical)
		{
			if( frangewindow <= ( ftall + _buttonOffset ) )
			{
				return true;
			}
		}
		else
		{
			if( frangewindow <= ( fwide + _buttonOffset ) )
			{
				return true;
			}
		}
	}

	return false;
}
	
void Slider2::addIntChangeSignal(IntChangeSignal* s)
{
	_intChangeSignalDar.putElement(s);
}

void Slider2::fireIntChangeSignal()
{	
	for(int i=0;i<_intChangeSignalDar.getCount();i++)
	{
		_intChangeSignalDar[i]->intChanged(getValue(),this);
	}
}

void Slider2::paintBackground()
{
	int wide,tall;
	getPaintSize(wide,tall);

	if (_vertical)
	{
		// background behind slider
		drawSetColor(40, 40, 40, 0);
		drawFilledRect(0, 0, wide, tall);

		// slider front
		drawSetColor(0, 0, 0, 0);
		drawFilledRect(0,_nobPos[0],wide,_nobPos[1]); 

		// slider border
		drawSetColor(60, 60, 60, 0);
		drawFilledRect(0,_nobPos[0],wide,_nobPos[0]+1);      // top
		drawFilledRect(0,_nobPos[1],wide,_nobPos[1]+1);      // bottom
		drawFilledRect(0,_nobPos[0]+1,1,_nobPos[1]);         // left
		drawFilledRect(wide-1,_nobPos[0]+1,wide,_nobPos[1]); // right
	}
	else
	{
		//!! doesn't work

		drawSetColor(Scheme::sc_secondary3);
		drawFilledRect(0,0,wide,tall);

		drawSetColor(Scheme::sc_black);
		drawOutlinedRect(0,0,wide,tall);

		drawSetColor(Scheme::sc_primary2);
		drawFilledRect(_nobPos[0],0,_nobPos[1],tall);
		
		drawSetColor(Scheme::sc_black);
		drawOutlinedRect(_nobPos[0],0,_nobPos[1],tall);
	}
}

void Slider2::setRange(int min,int max)
{
	if(max<min)
	{
		max=min;
	}

	if(min>max)
	{
		min=max;
	}

	_range[0]=min;
	_range[1]=max;
}

void Slider2::getRange(int& min,int& max)
{
	min=_range[0];
	max=_range[1];
}

void Slider2::privateCursorMoved(int x,int y,Panel* panel)
{
	if(!_dragging)
	{
		return;
	}

	getApp()->getCursorPos(x,y);
	screenToLocal(x,y);

	int wide,tall;
	getPaintSize(wide,tall);

	if(_vertical)
	{
		_nobPos[0]=_nobDragStartPos[0]+(y-_dragStartPos[1]);
		_nobPos[1]=_nobDragStartPos[1]+(y-_dragStartPos[1]);

		if(_nobPos[1]>tall)
		{
			_nobPos[0]=tall-(_nobPos[1]-_nobPos[0]);
			_nobPos[1]=tall;
		}
		
		if(_nobPos[0]<0)
		{
			_nobPos[1]=_nobPos[1]-_nobPos[0];
			_nobPos[0]=0;
		}
	}
	else
	{
		_nobPos[0]=_nobDragStartPos[0]+(x-_dragStartPos[0]);
		_nobPos[1]=_nobDragStartPos[1]+(x-_dragStartPos[0]);

		if(_nobPos[1]>wide)
		{
			_nobPos[0]=wide-(_nobPos[1]-_nobPos[0]);
			_nobPos[1]=wide;
		}
		
		if(_nobPos[0]<0)
		{
			_nobPos[1]=_nobPos[1]-_nobPos[0];
			_nobPos[0]=0;
		}
	}

	recomputeValueFromNobPos();
	repaint();
	fireIntChangeSignal();
}

void Slider2::privateMousePressed(MouseCode code,Panel* panel)
{
	int x,y;
	getApp()->getCursorPos(x,y);
	screenToLocal(x,y);

	if(_vertical)
	{
		if((y>=_nobPos[0])&&(y<_nobPos[1]))
		{
			_dragging=true;
			getApp()->setMouseCapture(this);
			_nobDragStartPos[0]=_nobPos[0];
			_nobDragStartPos[1]=_nobPos[1];
			_dragStartPos[0]=x;
			_dragStartPos[1]=y;
		}
	}
	else
	{
		if((x>=_nobPos[0])&&(x<_nobPos[1]))
		{
			_dragging=true;
			getApp()->setMouseCapture(this);
			_nobDragStartPos[0]=_nobPos[0];
			_nobDragStartPos[1]=_nobPos[1];
			_dragStartPos[0]=x;
			_dragStartPos[1]=y;
		}
	}

}

void Slider2::privateMouseReleased(MouseCode code,Panel* panel)
{
	_dragging=false;
	getApp()->setMouseCapture(null);
}

void Slider2::getNobPos(int& min, int& max)
{
	min=_nobPos[0];
	max=_nobPos[1];
}

void Slider2::setRangeWindow(int rangeWindow)
{
	_rangeWindow=rangeWindow;
}

void Slider2::setRangeWindowEnabled(bool state)
{
	_rangeWindowEnabled=state;
}

void Slider2::setButtonOffset(int buttonOffset)
{
	_buttonOffset=buttonOffset;
}