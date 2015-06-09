//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxToolTip.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXTOOLTIP
#define INCLUDED_MXTOOLTIP



class mxWidget;



class mxToolTip  
{
public:
	// NO CREATORS
	mxToolTip() {}
	virtual ~mxToolTip () {}

	// MANIPULATORS
	static void add (mxWidget *widget, const char *text);

private:
	// NOT IMPLEMENTED
	mxToolTip (const mxToolTip&);
	mxToolTip& operator= (const mxToolTip&);
};



#endif // INCLUDED_MXTOOLTIP
