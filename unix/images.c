#include "uipriv_unix.h"


struct uiImage
{
	GdkPixbuf *pixbuf;
};

uiImage* uiNewImage(uiImage *copy)
{
	uiImage* image = (uiImage*) uiAlloc(sizeof(uiImage), "uiImage");
	if (copy && copy->pixbuf)
	{
		image->pixbuf = copy->pixbuf;
	}
	if (image->pixbuf)
	{
		g_object_ref(image->pixbuf);
	}

	return image;
}

void uiImageDestroy(uiImage *image)
{
	if (image->pixbuf )
	{
		g_object_unref(image->pixbuf);
		image->pixbuf = NULL;
	}
	uiFree(image);
}

int uiImageLoad(uiImage* image, char const* file)
{
	if (image->pixbuf )
	{
		g_object_unref(image->pixbuf);
		image->pixbuf = NULL;
	}

	GError *error = NULL;
	image->pixbuf = gdk_pixbuf_new_from_file( file, &error );
	
	return (error == NULL);
}


int uiImageValid(uiImage *image)
{
	return !(image->pixbuf == NULL);
}

int uiImageSize(uiImage *image, int *width, int *height)
{
	if (image->pixbuf)
	{
		if (width)
			*width = gdk_pixbuf_get_width(image->pixbuf);
		if (height)
			*height = gdk_pixbuf_get_height(image->pixbuf);
		return 1;
	}

	return 0;
}


uiImage * uiImageResize(uiImage *image, int width, int height)
{
	if(image->pixbuf == NULL)
	{
		return 0;
	}

	GdkPixbuf *newpixbuf = gdk_pixbuf_scale_simple(image->pixbuf, width, height, GDK_INTERP_BILINEAR);
	if (newpixbuf)
	{
		uiImage* newimage = (uiImage*) uiAlloc(sizeof(uiImage), "uiImage");
		newimage->pixbuf = newpixbuf;
		return newimage;
	}

	return 0;
}


GdkPixbuf* uiImageGetPixbuf(uiImage *image)
{
	return image->pixbuf;
}
