//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxPcx.cpp
// implementation: all
// last modified:  Apr 15 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#include <mx/mxPcx.h>
#include <stdio.h>
#include <stdlib.h>



mxImage *
mxPcxRead (const char *filename)
{
    FILE *file = fopen (filename, "rb");
    if (!file)
        return 0;

	mxPcxHeader header;
    if (fread (&header, sizeof (mxPcxHeader), 1, file) == -1)
	{
        fclose (file);
        return 0;
    }
/*
    if (header.bitsPerPixel != 8 ||
		header.version != 5)
	{
        fclose (file);
        return 0;
    }

    (void) fseek (file, -769, SEEK_END);
    if (fgetc (file) != 12) {
        fclose (file);
        return NULL;
    }
*/
	(void) fseek (file, -768, SEEK_END);

    int w = header.xmax - header.xmin + 1;
    int h = header.ymax - header.ymin + 1;

	mxImage *image = new mxImage ();
	if (!image->create (w, h, 8))
	{
		delete image;
		fclose (file);
		return 0;
	}

    if (fread ((byte *) image->palette, sizeof (byte), 768, file) == -1)
	{
        fclose (file);
        return 0;
    }

    (void) fseek(file, sizeof (mxPcxHeader), SEEK_SET);
    int ptr = 0;
	int ch, rep;
	byte *data = (byte *) image->data;
	int size = w * h;
    while (ptr < size)
	{
        ch = fgetc(file);
        if (ch >= 192)
		{
            rep = ch - 192;
            ch = fgetc(file);
        }
        else {
            rep = 1;
        }

        while (rep--)
			data[ptr++] = ch;
    }

    fclose(file);

	return image;
}



bool
mxPcxWrite (const char *filename, mxImage *image)
{
	return false;
}