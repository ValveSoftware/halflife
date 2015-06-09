//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxProgressBar.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXPROGRESSBAR
#define INCLUDED_MXPROGRESSBAR



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxProgressBar_i;
class mxProgressBar : public mxWidget
{
	mxProgressBar_i *d_this;

public:
	// ENUMS
	enum { Normal, Smooth };

	// CREATORS
	mxProgressBar (mxWindow *parent, int x, int y, int w, int h, int style = 0);
	virtual ~mxProgressBar ();

	// MANIPULATORS
	void setValue (int value);
	void setTotalSteps (int steps);

	// ACCESSORS
	int getValue () const;
	int getTotalSteps () const;

private:
	// NOT IMPLEMENTED
	mxProgressBar (const mxProgressBar&);
	mxProgressBar& operator= (const mxProgressBar&);
};



#endif // INCLUDED_MXPROGRESSBAR
