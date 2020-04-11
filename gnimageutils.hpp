#pragma once
// Version 1.3.625.1627
//#include <objidl.h>
//#pragma comment(lib, "uuid.lib")

#include <string>
#include <GdiPlus.h>
#include <atlimage.h>

class GNImageStream
{
private:
	ULONG_PTR GdiplusToken;
	Gdiplus::GdiplusStartupInput	inputs;
	Gdiplus::GdiplusStartupOutput	output;
	Gdiplus::EncoderParameters		m_jpegParam;
	LONG							m_quality;

	IStream* pIStream;
public:
	CLSID encCLS;
	inline GNImageStream() : pIStream(NULL), encCLS(CLSID_NULL)
	{
		inputs.GdiplusVersion = 1;
		inputs.DebugEventCallback = NULL;
		inputs.SuppressBackgroundThread = FALSE;
		inputs.SuppressExternalCodecs = FALSE;
		Gdiplus::GdiplusStartup(&GdiplusToken, &inputs, &output);
	}
	inline virtual ~GNImageStream()
	{
		if (pIStream)
			pIStream->Release();
		pIStream = NULL;
		Gdiplus::GdiplusShutdown(GdiplusToken);
	}
	
	inline operator IStream*()
	{
		IStream* istrm = GetStream();
		if (!istrm)
			return istrm;
		LARGE_INTEGER lSeek = {0};
		istrm->Seek(lSeek, STREAM_SEEK_SET, NULL);
		return istrm;
	}
	
	inline IStream* GetStream()
	{
		if (!pIStream)
		{
			GetEncoderClsid(L"image/bmp", &encCLS);
			::CreateStreamOnHGlobal(NULL, TRUE, (LPSTREAM*)&pIStream);
		}
		return pIStream;
	}
	
	inline IStream* NewStream()
	{
		if (pIStream)
		{
			pIStream->Release();
			pIStream = NULL;
		}
		return GetStream();
	}
	
	inline bool Load(int _width, int _height, const void* _r, const void* _g, const void* _b)
	{
		if ( _width == 0 || _height == 0) 
			return false;

		ULONG lSize;

		GetEncoderClsid(L"image/bmp", &encCLS);
		NewStream();

		BITMAPFILEHEADER bmFileHeader;
		bmFileHeader.bfSize			= sizeof(BITMAPFILEHEADER);
		bmFileHeader.bfType			= 0x4D42;
		bmFileHeader.bfOffBits		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmFileHeader.bfReserved1	= 0;
		bmFileHeader.bfReserved2	= 0;

		BITMAPINFOHEADER bmInfoHeader;
		bmInfoHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bmInfoHeader.biWidth		= _width; // Width
		bmInfoHeader.biHeight		= _height; // Height
		bmInfoHeader.biPlanes		= 1;
		bmInfoHeader.biBitCount		= 32; // Color bit size
		bmInfoHeader.biCompression	= BI_RGB;
		bmInfoHeader.biSizeImage	= _width * _height * 4;
		bmInfoHeader.biXPelsPerMeter = 0;
		bmInfoHeader.biYPelsPerMeter = 0;
		bmInfoHeader.biClrUsed		= 0;
		bmInfoHeader.biClrImportant	= 0;

		ULARGE_INTEGER uSize;
		uSize.QuadPart = bmFileHeader.bfSize + bmInfoHeader.biSize + bmInfoHeader.biSizeImage;
		pIStream->SetSize(uSize);
		pIStream->Write(&bmFileHeader, bmFileHeader.bfSize, &lSize);
		pIStream->Write(&bmInfoHeader, bmInfoHeader.biSize, &lSize);

		std::string buff;
		buff.resize(bmInfoHeader.biSizeImage);
		BYTE *pWrite = (BYTE *)buff.c_str();

#pragma omp parallel for
		for (int y = 0; y < _height; y ++)
		{
			for (int x = 0; x < _width ; x ++)
			{
				int z = (y * _width + x);
				pWrite[z*4 + 0] = ((const BYTE *)_b)[z];
				pWrite[z*4 + 1] = ((const BYTE *)_g)[z];
				pWrite[z*4 + 2] = ((const BYTE *)_r)[z];
				pWrite[z*4 + 3] = 255;
			}
		}
		pIStream->Write(buff.c_str(), (ULONG)buff.size(), &lSize);
		return true;
	}

	inline bool Load(int _width, int _height, const void* _rgba)
	{
		if ( _width == 0 || _height == 0) 
			return false;

		ULONG lSize;

		GetEncoderClsid(L"image/bmp", &encCLS);
		NewStream();

		BITMAPFILEHEADER bmFileHeader;
		bmFileHeader.bfSize			= sizeof(BITMAPFILEHEADER);
		bmFileHeader.bfType			= 0x4D42;
		bmFileHeader.bfOffBits		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmFileHeader.bfReserved1	= 0;
		bmFileHeader.bfReserved2	= 0;

		BITMAPINFOHEADER bmInfoHeader;
		bmInfoHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bmInfoHeader.biWidth		= _width; // Width
		bmInfoHeader.biHeight		= _height; // Height
		bmInfoHeader.biPlanes		= 1;
		bmInfoHeader.biBitCount		= 32; // Color bit size
		bmInfoHeader.biCompression	= BI_RGB;
		bmInfoHeader.biSizeImage	= _width * _height * 4;
		bmInfoHeader.biXPelsPerMeter = 0;
		bmInfoHeader.biYPelsPerMeter = 0;
		bmInfoHeader.biClrUsed		= 0;
		bmInfoHeader.biClrImportant	= 0;

		ULARGE_INTEGER uSize;
		uSize.QuadPart = bmFileHeader.bfSize + bmInfoHeader.biSize + bmInfoHeader.biSizeImage;
		pIStream->SetSize(uSize);
		pIStream->Write(&bmFileHeader, bmFileHeader.bfSize, &lSize);
		pIStream->Write(&bmInfoHeader, bmInfoHeader.biSize, &lSize);
#if 0
		std::string buff;
		buff.resize(bmInfoHeader.biSizeImage);
		BYTE *pWrite = (BYTE *)buff.c_str();

#pragma omp parallel for
		for (int y = 0; y < _height; y ++)
		{
			for (int x = 0; x < _width ; x ++)
			{
				int z = (y * _width + x);
				pWrite[z*4 + 0] = ((const BYTE*)_rgb)[z*4+0];
				pWrite[z*4 + 1] = ((const BYTE*)_rgb)[z*4+1];
				pWrite[z*4 + 2] = ((const BYTE*)_rgb)[z*4+2];
				pWrite[z*4 + 3] = 255;
			}
		}
#endif
		pIStream->Write(_rgba, bmInfoHeader.biSizeImage, &lSize);
		return true;
	}

	inline bool Load(LPCVOID pByte, ULONG size)
	{
		ULONG lSize;
		NewStream()->Write(pByte, size, &lSize);
		return true;
	}
	
	inline bool Load(HINSTANCE hInst, const std::wstring& name, const std::wstring& _type)
	{
		HRSRC hres = FindResource(hInst, name.c_str(), _type.c_str());
		if (!hres)
			return false;

		DWORD dwSize = SizeofResource(hInst, hres);
		if (!dwSize)
			return false;
		HGLOBAL hResource = ::LoadResource(hInst, hres);
		const void* pSrc = ::LockResource(hResource);
		if (!pSrc)
			return false;

		Load(pSrc, dwSize);

		::UnlockResource(hResource);
		::FreeResource(hResource);
		
		return true;
	}

	inline bool Load(CBitmap& bmp)
	{
		Load((HBITMAP)bmp.m_hObject);
#if 0
		GetEncoderClsid(L"image/bmp", &encCLS);
	
		ULONG lSize;
		NewStream();

		BITMAP bmpInfo;
		bmp.GetBitmap(&bmpInfo);

		BITMAPFILEHEADER bmFileHeader;
		bmFileHeader.bfSize			= sizeof(BITMAPFILEHEADER);
		bmFileHeader.bfType			= 0x4D42;
		bmFileHeader.bfOffBits		= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		bmFileHeader.bfReserved1	= 0;
		bmFileHeader.bfReserved2	= 0;

		BITMAPINFOHEADER bmInfoHeader;
		bmInfoHeader.biSize			= sizeof(BITMAPINFOHEADER);
		bmInfoHeader.biWidth		= bmpInfo.bmWidth; // Width
		bmInfoHeader.biHeight		= bmpInfo.bmHeight; // Height
		bmInfoHeader.biPlanes		= bmpInfo.bmPlanes;
		bmInfoHeader.biBitCount		= bmpInfo.bmBitsPixel; // Color bit size
		bmInfoHeader.biCompression	= 0;
		bmInfoHeader.biSizeImage	= bmpInfo.bmWidthBytes * bmInfoHeader.biHeight * (((bmInfoHeader.biBitCount-1) >> 3) + 1);
		bmInfoHeader.biXPelsPerMeter = 0;
		bmInfoHeader.biYPelsPerMeter = 0;
		bmInfoHeader.biClrUsed		= 0;
		bmInfoHeader.biClrImportant	= 0;

		ULARGE_INTEGER uSize;
		uSize.QuadPart = bmFileHeader.bfSize + bmInfoHeader.biSize + bmInfoHeader.biSizeImage;
		pIStream->SetSize(uSize);
		pIStream->Write(&bmFileHeader, bmFileHeader.bfSize, &lSize);
		pIStream->Write(&bmInfoHeader, bmInfoHeader.biSize, &lSize);

		std::string buff; buff.resize(bmInfoHeader.biSizeImage);
		bmp.GetBitmapBits(bmInfoHeader.biSizeImage, (LPVOID)buff.c_str());
		pIStream->Write(buff.c_str(), bmInfoHeader.biSizeImage, &lSize);
#endif
		return true;
	}

	inline bool Load(HBITMAP hBMP)
	{
		GetEncoderClsid(L"image/bmp", &encCLS);
		CLSID bmpCLS;
		GetEncoderFormatID(L"image/bmp", &bmpCLS);
		CImage img;
		img.Attach(hBMP);
		img.Save(NewStream(), bmpCLS);
		img.Detach();
		return true;
	}

	inline bool Load(Gdiplus::Image*& pImg, LPCWSTR szFormat = L"image/bmp", const void *fmtOption = NULL)
	{
		GetEncoderClsid(szFormat, &encCLS);
		pImg->Save(NewStream(), &encCLS, (const Gdiplus::EncoderParameters*)fmtOption);
		return true;
	}
	
	inline bool Save(Gdiplus::Image*& pImg)
	{
		if (!pIStream)
			return false;
			
		LARGE_INTEGER lSeek = {0};
		pIStream->Seek(lSeek, STREAM_SEEK_SET, NULL);
		pImg = Gdiplus::Image::FromStream(pIStream);
		return true;
	}

	inline bool Save(std::string& bytes)
	{
		if (!pIStream)
			return false;

		LARGE_INTEGER lSeek = {0};
		ULARGE_INTEGER ulSize;
		ULONG size;
		pIStream->Seek(lSeek, STREAM_SEEK_END, &ulSize);
		pIStream->Seek(lSeek, STREAM_SEEK_SET, NULL);
		bytes.resize((UINT)ulSize.QuadPart);
		pIStream->Read((LPVOID)bytes.c_str(), (ULONG)ulSize.QuadPart, (ULONG*)&size);
		return true;
	}
	
	inline bool Save(BYTE*& pByte, ULONG& size)
	{
		if (!pIStream)
			return false;

		LARGE_INTEGER lSeek = {0};
		ULARGE_INTEGER ulSize;
		pIStream->Seek(lSeek, STREAM_SEEK_END, &ulSize);
		pByte = new BYTE[(UINT)ulSize.QuadPart];
		pIStream->Seek(lSeek, STREAM_SEEK_SET, NULL);
		pIStream->Read(pByte, (ULONG)ulSize.QuadPart, (ULONG*)&size);
		return true;
	}
	
	inline bool Save(const std::wstring& filename)
	{
		if (! pIStream)
			return false;

		LARGE_INTEGER lSeek = {0};
		ULARGE_INTEGER ulSize;
		pIStream->Seek(lSeek, STREAM_SEEK_END, &ulSize);

		ULONG size = 0;
		BYTE *pByte = new BYTE[(UINT)ulSize.QuadPart];
		pIStream->Seek(lSeek, STREAM_SEEK_SET, NULL);
		pIStream->Read(pByte, (ULONG)ulSize.QuadPart, (ULONG*)&size);
		HANDLE hFILE = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, 0);
		if (hFILE && hFILE != INVALID_HANDLE_VALUE)
		{
			WriteFile(hFILE, pByte, size, &size, 0);
			CloseHandle(hFILE);
		}
		delete[] pByte;

		return true;
	}

	inline const void *JPEGQuality(LONG quality = 60)
	{
		m_quality = quality;
		m_jpegParam.Parameter[0].Guid = Gdiplus::EncoderQuality;
		m_jpegParam.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
		m_jpegParam.Parameter[0].NumberOfValues = 1;
		m_jpegParam.Parameter[0].Value = &m_quality;
		m_jpegParam.Count = 1;
		return &m_jpegParam;
	}

	inline bool SetImageType(const std::wstring& mime = L"image/bmp", const void *fmtOption = NULL)
	{
		CLSID mimeCLS;
		GetEncoderClsid(mime.c_str(), &mimeCLS);
		if (memcmp(&mimeCLS, &encCLS, sizeof(encCLS)) == 0)
			return true;
		Gdiplus::Image* pImg = Gdiplus::Image::FromStream(GetStream());
		if (pImg)
		{
			GetEncoderClsid(mime.c_str(), &encCLS);
			pImg->Save(NewStream(), &mimeCLS, (const Gdiplus::EncoderParameters*)fmtOption);
			delete pImg;
			return true;
		}
		return false;
	}

	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
	   UINT  num = 0;          // number of image encoders
	   UINT  size = 0;         // size of the image encoder array in bytes

	   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	   Gdiplus::GetImageEncodersSize(&num, &size);
	   if(size == 0)
		  return -1;  // Failure

	   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	   if(pImageCodecInfo == NULL)
		  return -1;  // Failure

	   GetImageEncoders(num, size, pImageCodecInfo);

	   for(UINT j = 0; j < num; ++j)
	   {
		  if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		  {
			 *pClsid = pImageCodecInfo[j].Clsid;
			 free(pImageCodecInfo);
			 return j;  // Success
		  }    
	   }

	   free(pImageCodecInfo);
	   return -1;  // Failure
	}

	static int GetEncoderFormatID(const WCHAR* format, CLSID* pClsid)
	{
	   UINT  num = 0;          // number of image encoders
	   UINT  size = 0;         // size of the image encoder array in bytes

	   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	   Gdiplus::GetImageEncodersSize(&num, &size);
	   if(size == 0)
		  return -1;  // Failure

	   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	   if(pImageCodecInfo == NULL)
		  return -1;  // Failure

	   GetImageEncoders(num, size, pImageCodecInfo);

	   for(UINT j = 0; j < num; ++j)
	   {
		  if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		  {
			 *pClsid = pImageCodecInfo[j].FormatID;
			 free(pImageCodecInfo);
			 return j;  // Success
		  }    
	   }

	   free(pImageCodecInfo);
	   return -1;  // Failure
	}
};

class GNImage
{
private:
	Gdiplus::Image*		m_pImage;

public:
	inline GNImage() : m_pImage(NULL) { }
	inline virtual ~GNImage()
	{
		Destroy();
	}

	inline void Destroy()
	{
		if (m_pImage)
			delete m_pImage;
		m_pImage = NULL;
	}

	inline operator Gdiplus::Image*& ()
	{
		return m_pImage;
	}
	
	inline Gdiplus::Image*& operator ->()
	{
		return m_pImage;
	}
};
