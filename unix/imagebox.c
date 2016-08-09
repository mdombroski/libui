// 8 august 2016
#include "uipriv_unix.h"

struct uiImageBox {
	uiUnixControl c;
	GtkWidget *widget;
};

uiUnixControlAllDefaults(uiImageBox)

void uiImageBoxSetImage(uiImageBox *i, uiImage *image)
{
	gtk_image_set_from_pixbuf((GtkImage*) i->widget, uiImageGetPixbuf(image));
}

uiImageBox *uiNewImageBox(uiImage* image)
{
	uiImageBox *i;

	uiUnixNewControl(uiImageBox, i);
	i->widget = gtk_image_new();
	uiImageBoxSetImage(i, image);

	return i;
}
