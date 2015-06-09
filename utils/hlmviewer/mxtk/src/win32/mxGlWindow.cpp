//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxGlWindow.cpp
// implementation: Win32 API
// last modified:  Apr 21 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxGlWindow.h>
#include <windows.h>



void MsLog (const char *fmt, ...)
{
}



static int g_formatMode = mxGlWindow::FormatDouble;
static int g_formatColorBits = 32;
static int g_formatDepthBits = 24;



class mxGlWindow_i
{
public:
	HDC hdc;
	HGLRC hglrc;
};


void DumpPfd (int nIndex, PIXELFORMATDESCRIPTOR *ppfd)
{
	MsLog ("\nPixelFormat = %d", nIndex);
	MsLog (" nSize = %d", ppfd->nSize);
	MsLog (" nVersion = %d", ppfd->nVersion);
	
	MsLog (" dwFlags = ");
	if (ppfd->dwFlags & PFD_DRAW_TO_WINDOW)
		MsLog ("PFD_DRAW_TO_WINDOW|");
	if (ppfd->dwFlags & PFD_DRAW_TO_BITMAP)
		MsLog ("PFD_DRAW_TO_BITMAP|");
	if (ppfd->dwFlags & PFD_SUPPORT_GDI)
		MsLog ("PFD_SUPPORT_GDI|");
	if (ppfd->dwFlags & PFD_SUPPORT_OPENGL)
		MsLog ("PFD_SUPPORT_OPENGL|");
	if (ppfd->dwFlags & PFD_GENERIC_ACCELERATED)
		MsLog ("PFD_GENERIC_ACCELERATED|");
	if (ppfd->dwFlags & PFD_GENERIC_FORMAT)
		MsLog ("PFD_GENERIC_FORMAT|");
	if (ppfd->dwFlags & PFD_NEED_PALETTE)
		MsLog ("PFD_NEED_PALETTE|");
	if (ppfd->dwFlags & PFD_NEED_SYSTEM_PALETTE)
		MsLog ("PFD_NEED_SYSTEM_PALETTE|");
	if (ppfd->dwFlags & PFD_DOUBLEBUFFER)
		MsLog ("PFD_DOUBLEBUFFER|");
	if (ppfd->dwFlags & PFD_STEREO)
		MsLog ("PFD_STEREO|");
	if (ppfd->dwFlags & PFD_SWAP_LAYER_BUFFERS)
		MsLog ("PFD_SWAP_LAYER_BUFFERS|");
	if (ppfd->dwFlags & PFD_DEPTH_DONTCARE)
		MsLog ("PFD_DEPTH_DONTCARE|");
	if (ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE)
		MsLog ("PFD_DOUBLEBUFFER_DONTCARE|");
	if (ppfd->dwFlags & PFD_STEREO_DONTCARE)
		MsLog ("PFD_STEREO_DONTCARE|");
	if (ppfd->dwFlags & PFD_SWAP_COPY)
		MsLog ("PFD_SWAP_COPY|");
	if (ppfd->dwFlags & PFD_SWAP_EXCHANGE)
		MsLog ("PFD_SWAP_EXCHANGE|");

	MsLog (" iPixelType = ");
	switch (ppfd->iPixelType)
	{
	case PFD_TYPE_RGBA:
		MsLog ("PFD_TYPE_RGBA");
		break;
	case PFD_TYPE_COLORINDEX:
		MsLog ("PFD_TYPE_COLORINDEX");
		break;
	default:
		MsLog ("?\n");
	}
	MsLog ("*cColorBits = %d", ppfd->cColorBits);
	MsLog (" cRedBits = %d", ppfd->cRedBits);
	MsLog (" cRedShift = %d", ppfd->cRedShift);
	MsLog (" cGreenBits = %d", ppfd->cGreenBits);
	MsLog (" cGreenShift = %d", ppfd->cGreenShift);
	MsLog (" cBlueBits = %d", ppfd->cBlueBits);
	MsLog (" cBlueShift = %d", ppfd->cBlueShift);
	MsLog (" cAlphaBits = %d", ppfd->cAlphaBits);
	MsLog (" cAlphaShift = %d", ppfd->cAlphaShift);
	MsLog (" cAccumBits = %d", ppfd->cAccumBits);
	MsLog (" cAccumRedBits = %d", ppfd->cAccumRedBits);
	MsLog (" cAccumGreenBits = %d", ppfd->cAccumGreenBits);
	MsLog (" cAccumBlueBits = %d", ppfd->cAccumBlueBits);
	MsLog (" cAccumAlphaBits = %d", ppfd->cAccumAlphaBits);
	MsLog ("*cDepthBits = %d", ppfd->cDepthBits);
	MsLog ("*cStencilBits = %d", ppfd->cStencilBits);
	MsLog (" cAuxBuffers = %d", ppfd->cAuxBuffers);
	MsLog (" iLayerType = %d", ppfd->iLayerType);
	MsLog (" bReserved = %d", ppfd->bReserved);
	MsLog (" dwLayerMask = %d", ppfd->dwLayerMask);
	MsLog (" dwVisibleMask = %d", ppfd->dwVisibleMask);
	MsLog (" dwDamageMask = %d", ppfd->dwDamageMask);
}


mxGlWindow::mxGlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxWindow (parent, x, y, w, h, label, style)
{
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof (PIXELFORMATDESCRIPTOR),		// size of this pfd
		 1,		// version number
		 PFD_DRAW_TO_WINDOW |	// support window
		 PFD_SUPPORT_OPENGL |	// support OpenGL
		 PFD_DOUBLEBUFFER,	// double buffered
		 PFD_TYPE_RGBA,	// RGBA type
		 32,		// 24-bit color depth
		 0, 0, 0, 0, 0, 0,	// color bits ignored
		 0,		// no alpha buffer
		 0,		// shift bit ignored
		 0,		// no accumulation buffer
		 0, 0, 0, 0,	// accum bits ignored
		 24,		// 32-bit z-buffer      
		 0,		// no stencil buffer
		 0,		// no auxiliary buffer
		 PFD_MAIN_PLANE,	// main layer
		 0,		// reserved
		 0, 0, 0	// layer masks ignored
	};

	d_this = new mxGlWindow_i;

	setType (MX_GLWINDOW);
	setDrawFunc (0);

	pfd.cColorBits = g_formatColorBits;
	pfd.cDepthBits = g_formatDepthBits;

	bool error = false;

	MsLog ("mxGlWindow, GetDC");
	if ((d_this->hdc = GetDC ((HWND) getHandle ())) == NULL)
	{
		MsLog ("ERROR: mxGlWindow, GetDC");
		error = true;
		goto done;
	}

	MsLog ("mxGlWindow, ChoosePixelFormat");
	int pfm;
	if ((pfm = ::ChoosePixelFormat (d_this->hdc, &pfd)) == 0)
	{
		MsLog ("ERROR: mxGlWindow, ChoosePixelFormat");
		error = true;
		goto done;
	}
	
	MsLog ("mxGlWindow, SetPixelFormat");
	if (::SetPixelFormat (d_this->hdc, pfm, &pfd) == FALSE)
	{
		MsLog ("ERROR: mxGlWindow, SetPixelFormat");
		error = true;
		goto done;
	}

	MsLog ("mxGlWindow, GetPixelFormat");
	pfm = ::GetPixelFormat (d_this->hdc);
	MsLog ("mxGlWindow, DescribePixelFormat");
	::DescribePixelFormat (d_this->hdc, pfm, sizeof (pfd), &pfd);

	MsLog ("PixelFormat: %d", pfm);
	DumpPfd (pfm, &pfd);
	MsLog ("\n");

	MsLog ("mxGlWindow, wglCreateContext");	
	if ((d_this->hglrc = wglCreateContext (d_this->hdc)) == 0)
	{
		MsLog ("ERROR: mxGlWindow, wglCreateContext");	
		error = true;
		goto done;
	}

	MsLog ("mxGlWindow, wglMakeCurrent");	
	if (!wglMakeCurrent (d_this->hdc, d_this->hglrc))
	{
		MsLog ("ERROR: mxGlWindow, wglMakeCurrent");	
		error = true;
		goto done;
	}

done:
	if (error)
	{
		::MessageBox(0, "Error creating OpenGL window, please install the latest graphics drivers or Mesa 3.x!", "Error", MB_OK|MB_ICONERROR);
		exit(-1);
	}
	//else if (parent)
	//	parent->addWidget (this);
}



mxGlWindow::~mxGlWindow ()
{
	if (d_this->hglrc)
	{
		wglMakeCurrent (NULL, NULL);
		wglDeleteContext (d_this->hglrc);
	}

	if (d_this->hdc)
		ReleaseDC ((HWND) getHandle (), d_this->hdc);

	delete d_this;
}



int
mxGlWindow::handleEvent (mxEvent *event)
{
	return 0;
}



void
mxGlWindow::redraw ()
{
	makeCurrent ();
	if (d_drawFunc)
		d_drawFunc ();
	else
		draw ();
	swapBuffers ();
}



void
mxGlWindow::draw ()
{
}



int
mxGlWindow::makeCurrent ()
{
	if (wglMakeCurrent (d_this->hdc, d_this->hglrc))
		return 1;

	return 0;
}



int
mxGlWindow::swapBuffers ()
{
	if (SwapBuffers (d_this->hdc))
		return 1;

	return 0;
}



void
mxGlWindow::setDrawFunc (void (*func) (void))
{
	d_drawFunc = func;
}



void
mxGlWindow::setFormat (int mode, int colorBits, int depthBits)
{
	g_formatMode = mode;
	g_formatColorBits = colorBits;
	g_formatDepthBits = depthBits;
}
