#include "images.h"

#include <wincodec.h>

// using Windows Imaging Components(WIC)


struct uiImage
{
	IWICBitmapSource *ipBitmap;
};

uiImage* uiNewImage(uiImage *copy)
{
	uiImage* image = (uiImage*) uiAlloc(sizeof(uiImage), "uiImage");
	image->ipBitmap = NULL;

	if (copy && copy->ipBitmap)
	{
		// Create WIC factory
		IWICImagingFactory *factory = NULL;
		
		if(FAILED(CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&factory)))
		{
			return NULL;
		}

		// copy bitmap
		factory->CreateBitmapFromSource(copy->ipBitmap, WICBitmapCacheOnDemand, (IWICBitmap **) &image->ipBitmap);

		if (factory)
		{
			factory->Release();
		}
	}

	return image;
}

void uiImageDestroy(uiImage *image)
{
	if (image->ipBitmap)
	{
		image->ipBitmap->Release();
	}
	uiFree(image);
}

int uiImageLoad(uiImage* i, char const* file)
{
	IWICImagingFactory *pFactory = NULL;
	IWICBitmapDecoder *pDecoder = NULL;
	
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&pFactory);
	if(SUCCEEDED(hr))
	{
		WCHAR *wtext;
		wtext = toUTF16(file);
		hr = pFactory->CreateDecoderFromFilename( wtext, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder );
		uiFree(wtext);
	}

	UINT fc = 0;
	if(SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrameCount(&fc);
	}

	IWICBitmapFrameDecode *pFrameDecode = NULL;
	if(SUCCEEDED(hr) && fc > 0)
	{
		hr = pDecoder->GetFrame(0, &pFrameDecode);
		if(SUCCEEDED(hr))
		{
			hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, pFrameDecode, &i->ipBitmap);
		}
	}
	
	if (pFrameDecode)
	{
		pFrameDecode->Release();
	}

	if (pFactory)
	{
		pFactory->Release();
	}

	if (pDecoder)
	{
		pDecoder->Release();
	}

	return !( i->ipBitmap == 0 );
}


int uiImageValid(uiImage *image)
{
	return !(image->ipBitmap == NULL);
}

int uiImageSize(uiImage *image, int *width, int *height)
{
	if (image->ipBitmap)
	{
		UINT w = 0, h = 0;
		if (SUCCEEDED(image->ipBitmap->GetSize(&w, &h)))
		{
			if (width)
				*width = w;
			if (height)
				*height = h;
			return 1;
		}
	}

	return 0;
}


uiImage * uiImageResize(uiImage *image, int width, int height)
{
	HRESULT hr;

	if(image->ipBitmap == NULL)
	{
		return NULL;
	}

	// Create WIC factory
	IWICImagingFactory *factory = NULL;
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&factory);
	if(FAILED(hr))
	{
		return NULL;
	}

	// configure scaler
	IWICBitmapScaler *scaler = NULL;
	hr = factory->CreateBitmapScaler(&scaler);
	if(SUCCEEDED(hr))
	{
		hr = scaler->Initialize( image->ipBitmap, width, height, WICBitmapInterpolationModeFant );
	}
	
	// create new bitmap to invoke the scaler
	IWICBitmap* newbitmap = NULL;
	hr = factory->CreateBitmapFromSource(scaler, WICBitmapCacheOnDemand, &newbitmap);

	if (scaler)
	{
		scaler->Release();
	}

	if (factory)
	{
		factory->Release();
	}

	if (SUCCEEDED(hr))
	{
		uiImage* newimage = (uiImage*) uiAlloc(sizeof(uiImage), "uiImage");
		newimage->ipBitmap = newbitmap;
		return newimage;
	}

	return NULL;
}


HBITMAP uiImageGetHBITMAP(uiImage* image)
{
	if (image == NULL || image->ipBitmap == NULL)
	{
		return NULL;
	}

	// get image attributes and check for valid image
	UINT width = 0;
	UINT height = 0;
	if (FAILED(image->ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0)
		return NULL;

	// prepare structure giving bitmap information (negative height indicates a top-down DIB)
	BITMAPINFO bminfo;
	ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = -((LONG) height);
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	// create a DIB section that can hold the image
	void * pvImageBits = NULL;
	HDC hdcScreen = GetDC(NULL);
	HBITMAP hbmp = NULL;
	hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);
	if (hbmp == NULL)
		return NULL;

	// extract the image into the HBITMAP
	const UINT cbStride = width * 4;
	const UINT cbImage = cbStride * height;
	if (FAILED(image->ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
	{
		// couldn't extract image; delete HBITMAP
		DeleteObject(hbmp);
		hbmp = NULL;
	}

	// success!
	return hbmp;
}

HICON uiImageGetHICON(uiImage* image)
{
	HBITMAP bmp = uiImageGetHBITMAP(image);
	if (bmp == NULL)
	{
		return NULL;
	}

	ICONINFO iinfo =
	{
		TRUE,
		0, 0,
		bmp,
		bmp
	};

	HICON icon = CreateIconIndirect(&iinfo);
	DeleteObject(bmp);

	return icon;
}

IWICBitmapSource* uiImageGetIWICBitmapSource(uiImage* image)
{
	return image->ipBitmap;
}
