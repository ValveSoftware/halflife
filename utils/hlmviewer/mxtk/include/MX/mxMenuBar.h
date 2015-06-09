//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxMenuBar.h
// implementation: all
// last modified:  Apr 28 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXMENUBAR
#define INCLUDED_MXMENUBAR



#ifndef INCLUDED_MXWIDGET
#include <mx/mxWidget.h>
#endif



class mxWindow;
class mxMenu;



class mxMenuBar_i;
class mxMenuBar : public mxWidget
{
	mxMenuBar_i *d_this;

public:
	// CREATORS
	mxMenuBar (mxWindow *parent);
	virtual ~mxMenuBar ();

	// MANIPULATORS
	void addMenu (const char *item, mxMenu *menu);
	void setEnabled (int id, bool b);
	void setChecked (int id, bool b);
	void modify (int id, int newId, const char *newItem);

	// ACCESSORS
	bool isEnabled (int id) const;
	bool isChecked (int id) const;
	int getHeight () const;

private:
	// NOT IMPLEMENTED
	mxMenuBar (const mxMenuBar&);
	mxMenuBar& operator= (const mxMenuBar&);
};



#endif // INCLUDED_MXMENUBAR
