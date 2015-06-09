//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxProgressBar.cpp
// implementation: Win32 API
// last modified:  Mar 18 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxProgressBar.h>
#include <mx/mxWindow.h>
#include <windows.h>
#include <commctrl.h>



class mxProgressBar_i
{
public:
	int d_value;
	int d_steps;
};



mxProgressBar::mxProgressBar (mxWindow *parent, int x, int y, int w, int h, int style)
: mxWidget (parent, x, y, w, h)
{
	if (!parent)
		return;

	d_this = new mxProgressBar_i;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD;
	HWND hwndParent = (HWND) ((mxWidget *) parent)->getHandle ();

	if (style == Smooth)
		dwStyle |= PBS_SMOOTH;

	void *handle = (void *) CreateWindowEx (0, PROGRESS_CLASS, "", dwStyle,
				x, y, w, h, hwndParent,
				(HMENU) NULL, (HINSTANCE) GetModuleHandle (NULL), NULL);
	
	SendMessage ((HWND) handle, WM_SETFONT, (WPARAM) (HFONT) GetStockObject (ANSI_VAR_FONT), MAKELPARAM (TRUE, 0));

	setHandle (handle);
	setType (MX_PROGRESSBAR);
	setParent (parent);

	parent->addWidget (this);
}



mxProgressBar::~mxProgressBar ()
{
	delete d_this;
}



void
mxProgressBar::setValue (int value)
{
	d_this->d_value = value;
	SendMessage ((HWND) getHandle (), PBM_SETPOS, (WPARAM) value, 0L);
}


void
mxProgressBar::setTotalSteps (int steps)
{
	d_this->d_steps = steps;
	SendMessage ((HWND) getHandle (), PBM_SETRANGE, 0, MAKELPARAM (0, steps));
}



int
mxProgressBar::getValue () const
{
	return d_this->d_value;
}



int
mxProgressBar::getTotalSteps () const
{
	return d_this->d_steps;
}
