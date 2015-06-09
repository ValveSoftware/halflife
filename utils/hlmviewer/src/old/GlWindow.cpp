//
//                 Half-Life Model Viewer (c) 1999 by Mete Ciragan
//
// file:           GlWindow.cpp
// last modified:  May 04 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
// version:        1.2
//
// email:          mete@swissquake.ch
// web:            http://www.swissquake.ch/chumbalum-soft/
//
#include <mx/mx.h>
#include <mx/mxMessageBox.h>
#include <mx/mxTga.h>
#include <mx/mxPcx.h>
#include <mx/mxBmp.h>
#include <mx/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "GlWindow.h"
#include "StudioModel.h"
#include "ViewerSettings.h"



extern char g_appTitle[];
extern bool g_bStopPlaying;



GlWindow *g_GlWindow = 0;
vec3_t g_vright = { 50, 50, 0 };		// needs to be set to viewer's right in order for chrome to work
float g_lambert = 1.5;



GlWindow::GlWindow (mxWindow *parent, int x, int y, int w, int h, const char *label, int style)
: mxGlWindow (parent, x, y, w, h, label, style)
{
	glDepthFunc (GL_LEQUAL);

	if (!parent)
		setVisible (true);
	else
		mx::setIdleWindow (this);
}



GlWindow::~GlWindow ()
{
	mx::setIdleWindow (0);
	loadTexture (0, 0);
	loadTexture (0, 1);
}



int
GlWindow::handleEvent (mxEvent *event)
{
	static float oldrx = 0, oldry = 0, oldtz = 50, oldtx = 0, oldty = 0;
	static int oldx, oldy;

	switch (event->event)
	{

	case mxEvent::Idle:
	{
		g_studioModel.SetBlending (0, 0.0);
		g_studioModel.SetBlending (1, 0.0);

		static float prev;
		float curr = (float) mx::getTickCount () / 1000.0f;

		if (!g_bStopPlaying)
			g_studioModel.AdvanceFrame ((curr - prev) * g_viewerSettings.speedScale);

		prev = curr;

		if (!g_viewerSettings.pause)
			redraw ();

		return 1;
	}
	break;

	case mxEvent::MouseDown:
	{
		oldrx = g_viewerSettings.rot[0];
		oldry = g_viewerSettings.rot[1];
		oldtx = g_viewerSettings.trans[0];
		oldty = g_viewerSettings.trans[1];
		oldtz = g_viewerSettings.trans[2];
		oldx = event->x;
		oldy = event->y;
		g_viewerSettings.pause = false;

		return 1;
	}
	break;

	case mxEvent::MouseDrag:
	{
		if (event->buttons & mxEvent::MouseLeftButton)
		{
			if (event->modifiers & mxEvent::KeyShift)
			{
				g_viewerSettings.trans[0] = oldtx - (float) (event->x - oldx);
				g_viewerSettings.trans[1] = oldty + (float) (event->y - oldy);
			}
			else
			{
				g_viewerSettings.rot[0] = oldrx + (float) (event->y - oldy);
				g_viewerSettings.rot[1] = oldry + (float) (event->x - oldx);
			}
		}
		else if (event->buttons & mxEvent::MouseRightButton)
		{
			g_viewerSettings.trans[2] = oldtz + (float) (event->y - oldy);
		}
		redraw ();

		return 1;
	}
	break;

	case mxEvent::KeyDown:
	{
		switch (event->key)
		{
		case 32:
		{
			int iSeq = g_studioModel.GetSequence ();
			if (iSeq == g_studioModel.SetSequence (iSeq + 1))
			{
				g_studioModel.SetSequence (0);
			}
		}
		break;

		case 27:
			if (!getParent ()) // fullscreen mode ?
				mx::quit ();
			break;

		case 'g':
			g_viewerSettings.showGround = !g_viewerSettings.showGround;
			if (!g_viewerSettings.showGround)
				g_viewerSettings.mirror = false;
			break;

		case 'h':
			g_viewerSettings.showHitBoxes = !g_viewerSettings.showHitBoxes;
			break;

		case 'o':
			g_viewerSettings.showBones = !g_viewerSettings.showBones;
			break;

		case '5':
			g_viewerSettings.transparency -= 0.05f;
			if (g_viewerSettings.transparency < 0.0f)
				g_viewerSettings.transparency = 0.0f;

			break;

		case '6':
			g_viewerSettings.transparency += 0.05f;
			if (g_viewerSettings.transparency > 1.0f)
				g_viewerSettings.transparency = 1.0f;

			break;

		case 'b':
			g_viewerSettings.showBackground = !g_viewerSettings.showBackground;
			break;

		case 's':
			g_viewerSettings.useStencil = !g_viewerSettings.useStencil;
			break;

		case 'm':
			g_viewerSettings.mirror = !g_viewerSettings.mirror;
			if (g_viewerSettings.mirror)
				g_viewerSettings.showGround = true;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
			g_viewerSettings.renderMode = event->key - '1';
			break;

		case '-':
			g_viewerSettings.speedScale -= 0.1f;
			if (g_viewerSettings.speedScale < 0.0f)
				g_viewerSettings.speedScale = 0.0f;
			break;

		case '+':
			g_viewerSettings.speedScale += 0.1f;
			if (g_viewerSettings.speedScale > 5.0f)
				g_viewerSettings.speedScale = 5.0f;
			break;
		}
	}
	break;

	} // switch (event->event)

	return 1;
}



void
drawFloor ()
{
	if (g_viewerSettings.use3dfx)
	{
		glBegin (GL_TRIANGLE_STRIP);
		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (100.0f, 100.0f, 0.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (100.0f, -100.0f, 0.0f);

		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-100.0f, 100.0f, 0.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-100.0f, -100.0f, 0.0f);

		glEnd ();
	}
	else
	{
		glBegin (GL_TRIANGLE_STRIP);
		glTexCoord2f (0.0f, 0.0f);
		glVertex3f (-100.0f, 100.0f, 0.0f);

		glTexCoord2f (0.0f, 1.0f);
		glVertex3f (-100.0f, -100.0f, 0.0f);

		glTexCoord2f (1.0f, 0.0f);
		glVertex3f (100.0f, 100.0f, 0.0f);

		glTexCoord2f (1.0f, 1.0f);
		glVertex3f (100.0f, -100.0f, 0.0f);

		glEnd ();
	}
}



void
setupRenderMode ()
{
	if (g_viewerSettings.renderMode == RM_WIREFRAME)
	{
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		glDisable (GL_TEXTURE_2D);
		glDisable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
	}
	else if (g_viewerSettings.renderMode == RM_FLATSHADED ||
			g_viewerSettings.renderMode == RM_SMOOTHSHADED)
	{
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDisable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);

		if (g_viewerSettings.renderMode == RM_FLATSHADED)
			glShadeModel (GL_FLAT);
		else
			glShadeModel (GL_SMOOTH);
	}
	else if (g_viewerSettings.renderMode == RM_TEXTURED)
	{
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_TEXTURE_2D);
		glEnable (GL_CULL_FACE);
		glEnable (GL_DEPTH_TEST);
		glShadeModel (GL_SMOOTH);
	}
}



void
GlWindow::draw ()
{
	glClearColor (g_viewerSettings.bgColor[0], g_viewerSettings.bgColor[1], g_viewerSettings.bgColor[2], 0.0f);

	if (g_viewerSettings.useStencil)
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	else
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport (0, 0, w2 (), h2 ());

	//
	// show textures
	//

	if (g_viewerSettings.showTexture)
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();

		glOrtho (0.0f, (float) w2 (), (float) h2 (), 0.0f, 1.0f, -1.0f);

		studiohdr_t *hdr = g_studioModel.getTextureHeader ();
		if (hdr)
		{
			mstudiotexture_t *ptextures = (mstudiotexture_t *) ((byte *) hdr + hdr->textureindex);
			float w = (float) ptextures[g_viewerSettings.texture].width * g_viewerSettings.textureScale;
			float h = (float) ptextures[g_viewerSettings.texture].height * g_viewerSettings.textureScale;

			glMatrixMode (GL_MODELVIEW);
			glPushMatrix ();
			glLoadIdentity ();

			glDisable (GL_CULL_FACE);

			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			float x = ((float) w2 () - w) / 2;
			float y = ((float) h2 () - h) / 2;

			glDisable (GL_TEXTURE_2D);
			glColor4f (1.0f, 0.0f, 0.0f, 1.0f);
			glRectf (x - 2, y - 2, x  + w + 2, y + h + 2);

			glEnable (GL_TEXTURE_2D);
			glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
			glBindTexture (GL_TEXTURE_2D, g_viewerSettings.texture + 3); //d_textureNames[0]);

			glBegin (GL_TRIANGLE_STRIP);

			glTexCoord2f (0, 0);
			glVertex2f (x, y);

			glTexCoord2f (1, 0);
			glVertex2f (x + w, y);

			glTexCoord2f (0, 1);
			glVertex2f (x, y + h);

			glTexCoord2f (1, 1);
			glVertex2f (x + w, y + h);

			glEnd ();

			glPopMatrix ();

			glClear (GL_DEPTH_BUFFER_BIT);
			glBindTexture (GL_TEXTURE_2D, 0);
		}
		return;
	}

	//
	// draw background
	//

	if (g_viewerSettings.showBackground && d_textureNames[0] && !g_viewerSettings.showTexture)
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();

		glOrtho (0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f);

		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		glLoadIdentity ();

		glDisable (GL_CULL_FACE);
		glEnable (GL_TEXTURE_2D);

		glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

		glBindTexture (GL_TEXTURE_2D, d_textureNames[0]);

		glBegin (GL_TRIANGLE_STRIP);

		glTexCoord2f (0, 0);
		glVertex2f (0, 0);

		glTexCoord2f (1, 0);
		glVertex2f (1, 0);

		glTexCoord2f (0, 1);
		glVertex2f (0, 1);

		glTexCoord2f (1, 1);
		glVertex2f (1, 1);

		glEnd ();

		glPopMatrix ();

		glClear (GL_DEPTH_BUFFER_BIT);
		glBindTexture (GL_TEXTURE_2D, 0);
	}

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (65.0f, (GLfloat) w () / (GLfloat) h (), 1.0f, 4096.0f);

	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

    glTranslatef(-g_viewerSettings.trans[0], -g_viewerSettings.trans[1], -g_viewerSettings.trans[2]);
    
	glRotatef (g_viewerSettings.rot[0], 1.0f, 0.0f, 0.0f);
	glRotatef (g_viewerSettings.rot[1], 0.0f, 0.0f, 1.0f);

	// setup stencil buffer
    if (g_viewerSettings.useStencil)
	{
		/* Don't update color or depth. */
		glDisable(GL_DEPTH_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		/* Draw 1 into the stencil buffer. */
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

		/* Now render floor; floor pixels just get their stencil set to 1. */
		drawFloor();

		/* Re-enable update of color and depth. */ 
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		/* Now, only render where stencil is set to 1. */
		glStencilFunc(GL_EQUAL, 1, 0xffffffff);  /* draw if ==1 */
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    }

	g_vright[0] = g_vright[1] = g_viewerSettings.trans[2];

	if (g_viewerSettings.mirror)
	{
		glPushMatrix ();
		glScalef (1, 1, -1);
		glCullFace (GL_BACK);
		setupRenderMode ();
		g_studioModel.DrawModel ();
		glPopMatrix ();
	}

	if (g_viewerSettings.useStencil)
		glDisable (GL_STENCIL_TEST);

	setupRenderMode ();

	glCullFace (GL_FRONT);
	g_studioModel.DrawModel ();

	//
	// draw ground
	//

	if (g_viewerSettings.showGround)
	{
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_CULL_FACE);

		if (g_viewerSettings.useStencil)
			glFrontFace (GL_CW);
		else
			glDisable (GL_CULL_FACE);

		glEnable (GL_BLEND);
		if (!d_textureNames[1])
		{
			glDisable (GL_TEXTURE_2D);
			glColor4f (g_viewerSettings.gColor[0], g_viewerSettings.gColor[1], g_viewerSettings.gColor[2], 0.7f);
			glBindTexture (GL_TEXTURE_2D, 0);
		}
		else
		{
			glEnable (GL_TEXTURE_2D);
			glColor4f (1.0f, 1.0f, 1.0f, 0.6f);
			glBindTexture (GL_TEXTURE_2D, d_textureNames[1]);
		}

		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		drawFloor ();

		glDisable (GL_BLEND);

		if (g_viewerSettings.useStencil)
		{
			glCullFace (GL_BACK);
			glColor4f (0.1f, 0.1f, 0.1f, 1.0f);
			glBindTexture (GL_TEXTURE_2D, 0);
			drawFloor ();

			glFrontFace (GL_CCW);
		}
		else
			glEnable (GL_CULL_FACE);
	}

	glPopMatrix ();
}



int
GlWindow::loadTexture (const char *filename, int name)
{
	if (!filename || !strlen (filename))
	{
		if (d_textureNames[name])
		{
			glDeleteTextures (1, (const GLuint *) &d_textureNames[name]);
			d_textureNames[name] = 0;

			if (name == 0)
				strcpy (g_viewerSettings.backgroundTexFile, "");
			else
				strcpy (g_viewerSettings.groundTexFile, "");
		}

		return 0;
	}

	mxImage *image = 0;

	char ext[16];
	strcpy (ext, mx_getextension (filename));

	if (!mx_strcasecmp (ext, ".tga"))
		image = mxTgaRead (filename);
	else if (!mx_strcasecmp (ext, ".pcx"))
		image = mxPcxRead (filename);
	else if (!mx_strcasecmp (ext, ".bmp"))
		image = mxBmpRead (filename);

	if (image)
	{
		if (name == 0)
			strcpy (g_viewerSettings.backgroundTexFile, filename);
		else
			strcpy (g_viewerSettings.groundTexFile, filename);

		d_textureNames[name] = name + 1;

		if (image->bpp == 8)
		{
			mstudiotexture_t texture;
			texture.width = image->width;
			texture.height = image->height;

			g_studioModel.UploadTexture (&texture, (byte *) image->data, (byte *) image->palette, name + 1);
		}
		else
		{
			glBindTexture (GL_TEXTURE_2D, d_textureNames[name]);
			glTexImage2D (GL_TEXTURE_2D, 0, 3, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
			glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}

		delete image;

		return name + 1;
	}

	return 0;
}



void
GlWindow::dumpViewport (const char *filename)
{
#ifdef WIN32
	redraw ();
	int w = w2 ();
	int h = h2 ();

	mxImage *image = new mxImage ();
	if (image->create (w, h, 24))
	{
#if 0
		glReadBuffer (GL_FRONT);
		glReadPixels (0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, image->data);
#else
		HDC hdc = GetDC ((HWND) getHandle ());
		byte *data = (byte *) image->data;
		int i = 0;
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				COLORREF cref = GetPixel (hdc, x, y);
				data[i++] = (byte) ((cref >> 0)& 0xff);
				data[i++] = (byte) ((cref >> 8) & 0xff);
				data[i++] = (byte) ((cref >> 16) & 0xff);
			}
		}
		ReleaseDC ((HWND) getHandle (), hdc);
#endif
		if (!mxTgaWrite (filename, image))
			mxMessageBox (this, "Error writing screenshot.", g_appTitle, MX_MB_OK | MX_MB_ERROR);

		delete image;
	}
#endif	
}
