//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMenu.h
// implementation: all
// last modified:  Mar 14 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXMENU
#define INCLUDED_MXMENU



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxMenu_i;
class mxMenu : public mxWidget
{
	mxMenu_i *d_this;

public:
	// CREATORS
	mxMenu ();
	virtual ~mxMenu ();

	// MANIPULATORS
	void add (const char *item, int id);
	void addMenu (const char *item, mxMenu *menu);
	void addSeparator ();
	void setEnabled (int id, bool b);
	void setChecked (int id, bool b);

	// ACCESSORS
	bool isEnabled (int id) const;
	bool isChecked (int id) const;

private:
	// NOT IMPLEMENTED
	mxMenu (const mxMenu&);
	mxMenu& operator= (const mxMenu&);
};



#endif // INCLUDED_MXMENU
