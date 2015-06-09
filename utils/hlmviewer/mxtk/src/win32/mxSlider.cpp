//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxSlider.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxSlider.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commctrl.h>



class mxSlider_i
{
public:
	int dummy;
};



mxSlider::mxSlider (mxWindow *parent, int x, int y, int w, int h, int id, int style)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (dwStyle == Horizontal)
		dwStyle = WS_CHILD | WS_VISIBLE | TBS_HORZ;
	else if (dwStyle == Vertical)
		dwStyle = WS_CHILD | WS_VISIBLE | TBS_VERT;

	void *handle = (void *) CreateWindowEx (0, TRACKBAR_CLASS, "", dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) id, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));
	SetWindowLong ((HWND) handle, GWL_USERDATA, (LONG) this);

	setHandle (handle);
	setType (MX_SLIDER);
	setParent (parent);
	setId (id);

	parent->addWidget (this);
}



mxSlider::~mxSlider ()
{
}



void
mxSlider::setValue (int value)
{
	SendMessage ((HWND) getHandle (), TBM_SETPOS, (WPARAM) true, (LPARAM) value);
}
	


void
mxSlider::setRange (int min, int max)
{
	SendMessage ((HWND) getHandle (), TBM_SETRANGE, (WPARAM) true, (LPARAM) MAKELONG(min, max));
}



void
mxSlider::setSteps (int line, int page)
{
	SendMessage ((HWND) getHandle (), TBM_SETLINESIZE, 0, (LPARAM) line);
	SendMessage ((HWND) getHandle (), TBM_SETPAGESIZE, 0, (LPARAM) page);
}



int
mxSlider::getValue () const
{
	return (int) SendMessage ((HWND) getHandle (), TBM_GETPOS, 0, 0L);
}



int
mxSlider::getMinValue () const
{
	return (int) SendMessage ((HWND) getHandle (), TBM_GETRANGEMIN, 0, 0L);
}



int
mxSlider::getMaxValue () const
{
	return (int) SendMessage ((HWND) getHandle (), TBM_GETRANGEMAX, 0, 0L);
}



int
mxSlider::getLineStep () const
{
	return (int) SendMessage ((HWND) getHandle (), TBM_GETLINESIZE, 0, 0L);
}



int
mxSlider::getPageStep () const
{
	return (int) SendMessage ((HWND) getHandle (), TBM_GETPAGESIZE, 0, 0L);
}
