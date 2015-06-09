//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxEvent.h
// implementation: all
// last modified:  Apr 12 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXEVENT
#define INCLUDED_MXEVENT



class mxWidget;



class mxEvent
{
public:
	// ENUMS
	enum { Action, Size, Timer, Idle, Show, Hide,
		MouseUp, MouseDown, MouseMove, MouseDrag,
		KeyUp, KeyDown, MouseWheel
	};

	enum { MouseLeftButton = 1, MouseRightButton = 2, MouseMiddleButton = 4};
	enum { KeyCtrl = 1, KeyShift = 2 };
	enum { RightClicked = 1, DoubleClicked = 2 };

	// DATA
	int event;
	mxWidget *widget;
	int action;
	int width, height;
	int x, y, buttons;
	int key;
	int modifiers;
	int flags;
	int zdelta;

	// NO CREATORS
	mxEvent () : event (0), widget (0), action (0), width (0), height (0), x (0), y (0), buttons (0), key (0), modifiers (0), flags (0) {}
	virtual ~mxEvent () {}

private:
	// NOT IMPLEMENTED
	mxEvent (const mxEvent&);
	mxEvent& operator= (const mxEvent&);
};



#endif // INCLUDED_MXEVENT
