//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxWidget.h
// implementation: all
// last modified:  May 24 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXWIDGET
#define INCLUDED_MXWIDGET



enum
{
	MX_BUTTON,
	MX_CHECKBOX,
	MX_CHOICE,
	MX_GLWINDOW,
	MX_GROUPBOX,
	MX_LABEL,
	MX_LINEEDIT,
	MX_LISTBOX,
	MX_MENU,
	MX_MENUBAR,
	MX_MULTILINEEDIT,
	MX_POPUPMENU,
	MX_PROGRESSBAR,
	MX_RADIOBUTTON,
	MX_SLIDER,
	MX_TAB,
	MX_TOGGLEBUTTON,
	MX_TREEVIEW,
	MX_WINDOW,
	MX_DIALOGWINDOW
};



class mxWindow;



class mxWidget_i;
class mxWidget
{
	mxWidget_i *d_this;

protected:
	void setHandle (void *handle);
	void setType (int type);
	void setParent (mxWindow *parentWindow);

public:
	// CREATORS
	mxWidget (mxWindow *parent, int x, int y, int w, int h, const char *label = 0);
	virtual ~mxWidget ();

	// MANIPULATORS

	// void setBounds (int x, int y, int w, int h);
	//
	//
	void setBounds (int x, int y, int w, int h);

	// void setLabel (const char *label);
	//
	//
	void setLabel (const char *label);

	void setVisible (bool b);
	void setEnabled (bool b);
	void setId (int id);
	void setUserData (void *userData);

	// ACCESSORS
	void *getHandle () const;
	int getType () const;
	mxWindow *getParent () const;
	int x () const;
	int y () const;
	int w () const;
	int h () const;
	int w2 () const;
	int h2 () const;
	const char *getLabel () const;
	bool isVisible () const;
	bool isEnabled () const;
	int getId () const;
	void *getUserData () const;

private:
	// NOT IMPLEMENTED
	mxWidget (const mxWidget&);
	mxWidget& operator= (const mxWidget&);
};



#endif // INCLUDED_MXWIDGET
