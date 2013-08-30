//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_EDITPANEL_H
#define VGUI_EDITPANEL_H

#include<VGUI.h>
#include<VGUI_Panel.h>
#include<VGUI_Dar.h>

namespace vgui
{

class Font;

class VGUIAPI EditPanel : public Panel
{
public:
	EditPanel(int x,int y,int wide,int tall);
public:
	virtual void doCursorUp();
	virtual void doCursorDown();
	virtual void doCursorLeft();
	virtual void doCursorRight();
	virtual void doCursorToStartOfLine();
	virtual void doCursorToEndOfLine();
	virtual void doCursorInsertChar(char ch);
	virtual void doCursorBackspace();
	virtual void doCursorNewLine();
	virtual void doCursorDelete();
	virtual void doCursorPrintf(char* format,...);
	virtual int  getLineCount();
	virtual int  getVisibleLineCount();
	virtual void setCursorBlink(bool state);
	virtual void setFont(Font* font);
	virtual void getText(int lineIndex, int offset,char* buf,int bufLen);

public: //bullshit public
	void getCursorBlink(bool& blink,int& nextBlinkTime);
protected:
	virtual void       paintBackground();
	virtual void       paint();
	virtual void       addLine();
	virtual Dar<char>* getLine(int lineIndex);
	virtual void       setChar(Dar<char>* lineDar,int x,char ch,char fill);
	virtual void       setChar(Dar<char>* lineDar,int x,char ch);
	virtual void       shiftLineLeft(Dar<char>* lineDar,int x,int count);
	virtual void       shiftLineRight(Dar<char>* lineDar,int x,int count);
private:
	virtual int        spatialCharOffsetBetweenTwoLines(Dar<char>* srcDar,Dar<char>* dstDar,int x);
protected:
	Dar<Dar<char>*> _lineDarDar;
	int             _cursor[2];
	bool            _cursorBlink;
	int             _cursorNextBlinkTime;
	Font*           _font;
};

}

#endif