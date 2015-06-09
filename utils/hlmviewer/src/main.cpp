//#include "GL/freeglut.h"
#include "FL/FL.h"
#include "FL/Fl_Gl_Window.h"

#include <stdlib.h>
#include <stdio.h>

class GlWindow : public Fl_Gl_Window 
{
public:
	GlWindow(int X, int Y, int W, int H, const char *L)
		: Fl_Gl_Window(X, Y, W, H, L)
	{

	}

	~GlWindow()
	{

	}

private:
	void draw()
	{

	}
	int handle(int)
	{

	}
};

int main(int argc, char**argv)
{
	GlWindow* window = new GlWindow(0, 0, 640, 480, "My FLTK window");

	return Fl::run();
}