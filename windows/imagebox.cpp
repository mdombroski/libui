// 8 august 2016
#include "images.h"

struct uiImageBox {
	uiWindowsControl c;
	HWND hwnd;
};

static void uiImageBoxDestroy(uiControl *c)
{
	uiImageBox *b = uiImageBox(c);
	
	uiImageBoxSetImage(b, NULL);
	uiWindowsEnsureDestroyWindow(b->hwnd);
	uiFreeControl(uiControl(b));
}

uiWindowsControlAllDefaultsExceptDestroy(uiImageBox)

static void uiImageBoxMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	uiImageBox *i = uiImageBox(c);
	
	// minimum size
	*width = 5;
	*height = 5;

	// get the HBITMAP and fetch it's size
	HBITMAP hb = (HBITMAP) SendMessage(i->hwnd, STM_GETIMAGE, IMAGE_BITMAP, 0);
	if(hb != NULL)
	{
		BITMAP bm;
		if( GetObject( hb, sizeof(bm), &bm ) != 0 )
		{
			*height = bm.bmHeight;
			*width = bm.bmWidth;
		}
	}
}

void uiImageBoxSetImage(uiImageBox *i, uiImage *image)
{
	HBITMAP bmp = uiImageGetHBITMAP(image);

	// it is a responsibility of the programmer to delete the old bitmap
	HBITMAP old = (HBITMAP) SendMessage(i->hwnd, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bmp);
	if (old && old != bmp)
	{
		DeleteObject(old);
	}

	// sometimes an internal copy is made and bmp must be deleted
	HBITMAP copy = (HBITMAP) SendMessage(i->hwnd, STM_GETIMAGE, IMAGE_BITMAP, 0);
	if (copy && bmp && copy != bmp)
	{
		DeleteObject(bmp);
	}

	// changing the bitmap might necessitate a change in size
	uiWindowsControlMinimumSizeChanged(uiWindowsControl(i));
}

uiImageBox *uiNewImageBox(uiImage* image)
{
	uiImageBox *i;

	uiWindowsNewControl(uiImageBox, i);
	
	i->hwnd = uiWindowsEnsureCreateControlHWND(0,
		L"static", NULL,
		SS_BITMAP | SS_CENTERIMAGE,
		hInstance, NULL,
		TRUE);

	uiImageBoxSetImage(i, image);
	return i;
}
