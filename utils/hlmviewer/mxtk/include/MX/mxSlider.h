//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxSlider.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXSLIDER
#define INCLUDED_MXSLIDER



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;



class mxSlider_i;
class mxSlider : public mxWidget
{
	mxSlider_i *d_this;

public:
	// ENUMS
	enum { Horizontal, Vertical };

	// CREATORS
	mxSlider (mxWindow *parent, int x, int y, int w, int h, int id = 0, int style = 0);
	virtual ~mxSlider ();

	// MANIPULATORS
	void setValue (int value);
	void setRange (int min, int max);
	void setSteps (int line, int page);

	// ACCESSORS
	int getValue () const;
	int getMinValue () const;
	int getMaxValue () const;
	int getLineStep () const;
	int getPageStep () const;

private:
	// NOT IMPLEMENTED
	mxSlider (const mxSlider&);
	mxSlider& operator= (const mxSlider&);
};



#endif // INCLUDED_MXSLIDER
