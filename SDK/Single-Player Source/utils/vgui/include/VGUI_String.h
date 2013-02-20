//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_STRING_H
#define VGUI_STRING_H

#include<VGUI.h>


namespace vgui
{

class VGUIAPI String
{

friend class String;

private:

	char* _text;

public:

	String();
	String(const char* text);
	String(const String& src);

public:
	
	~String();

private:

	int getCount(const char* text);

public:

	int    getCount();
	String operator+(String text);
	String operator+(const char* text);
	bool   operator==(String text);
	bool   operator==(const char* text);
	char   operator[](int index);
	const char* getChars();

public:
	
	static void test();

};


}


#endif

