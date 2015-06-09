//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxImage.h
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXIMAGE
#define INCLUDED_MXIMAGE



#include <stdlib.h>



#ifndef byte
typedef unsigned char byte;
#endif // byte

#ifndef word
typedef unsigned short word;
#endif // word



class mxImage
{
public:
	int width;
	int height;
	int bpp;
	void *data;
	void *palette;

	// CREATORS
	mxImage () : width (0), height (0), bpp (0), data (0), palette (0)
	{
	}

	mxImage (int w, int h, int bpp)
	{
		create (w, h, bpp);
	}

	virtual ~mxImage ()
	{
		destroy ();
	}

	// MANIPULATORS
	bool create (int w, int h, int pixelSize)
	{
		if (data)
			free (data);

		if (palette)
			free (palette);

		data = malloc (w * h * pixelSize / 8);
		if (!data)
			return false;

		// allocate a palette for 8-bit images
		if (pixelSize == 8)
		{
			palette = malloc (768);
			if (!palette)
			{
				//delete[] data;
				free (data);
				return false;
			}
		}
		else
			palette = 0;

		width = w;
		height = h;
		bpp = pixelSize;

		return true;
	}

	void destroy ()
	{
		if (data)
			free (data);
			//delete[] data;

		if (palette)
			free (palette);
			//delete[] palette;

		data = palette = 0;
		width = height = bpp = 0;
	}

private:
	// NOT IMPLEMENTED
	mxImage (const mxImage&);
	mxImage& operator= (const mxImage&);
};



#endif // INCLUDED_MXIMAGE
