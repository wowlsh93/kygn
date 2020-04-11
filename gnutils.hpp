#pragma once

#include "gnthread.hpp"
#include <vector>
#include <functional>
#include <algorithm>
#include <cctype>
#include <windows.h>
#include <tlhelp32.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <dbghelp.h>

#pragma comment(lib, "DbgHelp.lib")
#include <winver.h>
#pragma comment(lib, "Version.lib")

#ifndef SAFE_FREE
#define SAFE_FREE(X)  { if ((X)) { ::free((X)); (X)=NULL; } }
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(X)  { if ((X)) { delete (X); (X)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(X)  { if ((X)) { delete[] (X); (X)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(X)  { if ((X)) { (X)->Release(); } }
#endif

class GNDump
{
protected:
	static 
#ifdef WIN64
	LONG __cdecl _exception_minidump( _EXCEPTION_POINTERS* pExceptionInfo )
#else
	LONG __stdcall _exception_minidump( _EXCEPTION_POINTERS* pExceptionInfo )
#endif
	{
		SYSTEMTIME st;
		GetLocalTime(&st);

		CString fileName;
		fileName.Format(_T("GNDUMP_%d%02d%02d_%02d%02d%02d.dmp"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );

		HANDLE hProcess		= GetCurrentProcess();
		DWORD dwProcessID	= GetCurrentProcessId();
		HANDLE hFile		= CreateFile( fileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId			= GetCurrentThreadId();
		eInfo.ExceptionPointers = pExceptionInfo;
		eInfo.ClientPointers	= FALSE;

		MiniDumpWriteDump( hProcess, dwProcessID, hFile, MiniDumpWithFullMemory, pExceptionInfo ? &eInfo : NULL, NULL, NULL );

		return EXCEPTION_EXECUTE_HANDLER;
	}

public:
	static void install()
	{
		SetUnhandledExceptionFilter( _exception_minidump );
	}
};

class GNCodec
{
public:
	inline static DWORD CRC32(const void *str, int size, DWORD dwCRC)
	{
		static DWORD s_arrdwCrc32Table[256] = { 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
				0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A,
				0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,

				0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
				0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
				0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,

				0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0,
				0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
				0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,

				0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70,
				0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
				0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D, };

		if (size < 0)
			return dwCRC;
		unsigned char *b = (unsigned char *) str;
		int len = (int)(size ? size : strlen((LPSTR) str));
		for (int i = 0; i < len; i++)
			dwCRC = ((dwCRC) >> 8) ^ s_arrdwCrc32Table[(BYTE)(((DWORD) b[i]) ^ ((dwCRC) & 0x000000FF))];
		return dwCRC;
	}

	inline static void enc64(char *e,const char *q)
	{
		static const char *b64x = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		unsigned char *b = (unsigned char *)q;
		unsigned int cox = b[0] & 0xff;
		cox <<= 8; cox |= b[1] & 0xff;
		cox <<= 8; cox |= b[2] & 0xff;
		e[0] = b64x[(cox >> 18) & 0x3f];
		e[1] = b64x[(cox >> 12) & 0x3f];
		e[2] = b64x[(cox >>  6) & 0x3f];
		e[3] = b64x[(cox >>  0) & 0x3f];
	}

	inline static std::string B64Enc(const char *buf, size_t size)
	{
		std::string rets;
		if (size<1) return rets;
		rets.resize(((size-1)/3+1)*4 + 4);
		size_t ii = 0, i;
		char xbuf[3];

		for(i = 0; i < size; i += 3) {
			xbuf[0] = buf[i+0];
			if (i+1<size) xbuf[1] = buf[i+1]; else xbuf[1] = 0;
			if (i+2<size) xbuf[2] = buf[i+2]; else xbuf[2] = 0;
			enc64((LPSTR)rets.c_str() + ii, xbuf);
			rets[ii+=4] = 0;
		}

		for(; i > size; --i)
			rets[--ii] = '=';
		while (rets.at(rets.size()-1) == 0)
			rets.resize(rets.size()-1);
		return rets;
	}

	inline static std::string B64Enc(const std::string& ee)
	{
		return B64Enc(ee.c_str(), ee.size());
	}

	inline static BYTE d64(char c)
	{
		BYTE r = 0;
		if (c >= 'A' && c <= 'Z')
			r |= (c - 'A');
		else if (c >= 'a' && c <= 'z')
			r |= (c -'a') + 26;
		else if (c >= '0' && c <= '9')
			r |= (c -'0') + 52;
		else if (c == '+')
			r |= 53;
		else if (c == '/')
			r |= 54;
		return r;
	}

	inline static void dec64(std::string& e,const char *q)
	{
		BYTE c1 = d64(q[0]);
		BYTE c2 = d64(q[1]);
		BYTE c3 = d64(q[2]);
		BYTE c4 = d64(q[3]);
		
		char r[4] = {0,0,0,1};
		r[0] = (c1 << 2) | ((c2 >> 4) & 0x3);
		if (c3!='=')
		{
			r[3] ++;
			r[1] = ((c2&0xf) << 4) | ((c3 >> 2) & 0xf);
			if (c4 != '=')
			{
				r[3] ++;
				r[2] = ((c3&0x3) << 6) | (c4&0x3f);
			}
		}
		e.append(r, r[3]);
	}

	inline static std::string B64Dec(const char *buf, size_t size)
	{
		std::string rets;
		if (size<4) return rets;
		for (size_t i = 0; i < size; i += 4)
			dec64(rets, buf+i);
		return rets;
	}

	inline static std::string B64Dec(const std::string& ee)
	{
		return B64Dec(ee.c_str(), ee.size());
	}

	inline static std::wstring toString(const std::string& sData)
	{
		std::wstring sUTF8;
		if (sData.empty())
			return sUTF8;
		int nChars = ::MultiByteToWideChar(CP_ACP, 0, (LPSTR)sData.c_str(), -1, NULL, NULL);
		sUTF8.resize(nChars);
		nChars = ::MultiByteToWideChar(CP_ACP, 0, (LPSTR)sData.c_str(), -1, (LPWSTR)sUTF8.data(), (int)sUTF8.size());
		sUTF8.resize(nChars ? nChars-1 : nChars);
		return sUTF8;
	}

	inline static std::string toString(const std::wstring& sValue)
	{
		std::string sUTF8;
		BOOL bFALSE = FALSE;
		int nMultiByteLen = ::WideCharToMultiByte(CP_ACP, 0, sValue.c_str(), -1, NULL, 0, NULL, NULL);
		sUTF8.resize(nMultiByteLen);
		int nBytes = ::WideCharToMultiByte(CP_ACP, 0, sValue.c_str(), -1, (char *)sUTF8.data(), (int)sUTF8.size(), NULL, NULL);
		sUTF8.resize(nBytes ? nBytes-1 : nBytes);
		return sUTF8;
	}

	inline static bool isUTF8(const std::string& sData)
	{
		for(size_t i=0; i < sData.size(); i++)
		{
			if ((sData.at(i)&0x80) != 0x80)
				continue;
			if ((sData.at(i)&0xE0) == 0xC0 && (sData.at(++i) & 0xC0) == 0x80)
				return true;
			if ((sData.at(i)&0xF0) == 0xE0 && (sData.at(++i) & 0xC0) == 0x80 && (sData.at(++i) & 0xC0) == 0x80)
				return true;
			if ((sData.at(i)&0xF8) == 0xF0 && (sData.at(++i) & 0xC0) == 0x80 && (sData.at(++i) & 0xC0) == 0x80 && (sData.at(++i) & 0xC0) == 0x80)
				return true;
			if ((sData.at(i)&0x80) == 0x80)
				++ i;
		}
		return false;
	}

	inline static std::wstring fromUTF8(const std::string& sData)
	{
		std::wstring sUTF8;
		if (sData.empty())
			return sUTF8;

		int nChars = ::MultiByteToWideChar(CP_UTF8, 0, (LPSTR)sData.c_str(), -1, NULL, NULL);
		sUTF8.resize(nChars);
		nChars = ::MultiByteToWideChar(CP_UTF8, 0, (LPSTR)sData.c_str(), -1, (LPWSTR)sUTF8.data(), (int)sUTF8.size());
		sUTF8.resize(nChars ? nChars-1 : nChars);
		return sUTF8;
	}

	inline static std::string toUTF8(const std::wstring& sValue)
	{
		BOOL bFALSE = FALSE;
		int nMultiByteLen = ::WideCharToMultiByte(CP_UTF8, 0, sValue.c_str(), -1, NULL, 0, NULL, NULL);

		std::string sUTF8;
		sUTF8.resize(nMultiByteLen);

		int nBytes = ::WideCharToMultiByte(CP_UTF8, 0, sValue.c_str(), -1, (char *)sUTF8.data(), (int)sUTF8.size(), NULL, NULL);
		sUTF8.resize(nBytes ? nBytes-1 : nBytes);
		return sUTF8;
	}
	
	static std::string urlencode(const std::string& url)
	{
		static char sHex[] = "0123456789ABCDEF";
		static std::string sCod(" !*'();:@&=+$,/?#[]%-.<>\\^_`{|}~\"\n\r\t\b");
		std::string enc;

		for (size_t i = 0; i < url.size(); i ++)
		{
			char code = url.at(i);
			if ((code & 0x80) == 0)
			{// 0x00 ~ 0x7F
				if (sCod.find(code) != std::wstring::npos)
				{
					enc.push_back('%');
					enc.push_back(sHex[(code>>4)&0xF]);
					enc.push_back(sHex[(code>>0)&0xF]);
				}
				else
				{		
					enc.push_back(code);
				}
			}
			else
			{
				enc.push_back(L'%');
				enc.push_back(sHex[(code>>4)&0xF]);
				enc.push_back(sHex[(code>>0)&0xF]);
			}
		}
		return enc;
	}

	static std::string urldecode(const std::string& text)
	{
		char *end;
		std::string dest;
		for (std::string::const_iterator c = text.begin(); c != text.end(); c++)
		{
			if (*c == '%')
			{
				long uch = strtol(&(c[1]), &end, 16);
				c += 2;
				dest.push_back((char)uch);
			}
			else
			if (*c == '+')
			{
				dest.push_back(' ');
			}
			else
			{
				dest.push_back(*c);
			}
		
		}
		return dest;
	}

	static std::wstring urlencode(const std::wstring& url)
	{
		static wchar_t sHex[] = L"0123456789ABCDEF";
		static std::wstring sCod(L" !*'();:@&=+$,/?#[]%-.<>\\^_`{|}~\"\n\r\t\b");
		std::wstring enc;

		for (size_t i = 0; i < url.size(); i ++)
		{
			wchar_t code = url.at(i);
			if ((code & 0x80) == 0)
			{// 0x00 ~ 0x7F
				if (sCod.find(code) != std::wstring::npos)
				{
					enc.push_back(L'%');
					enc.push_back(sHex[(code>>4)&0xF]);
					enc.push_back(sHex[(code>>0)&0xF]);
				}
				else
				{		
					enc.push_back(code);
				}
			}
			else
			{
				enc.push_back(L'%');
				enc.push_back(sHex[(code>>4)&0xF]);
				enc.push_back(sHex[(code>>0)&0xF]);
			}
		}
		return enc;
	}

	static std::wstring urldecode(const std::wstring& text)
	{
		wchar_t *end;
		std::wstring dest;
		for (std::wstring::const_iterator c = text.begin(); c != text.end(); c++)
		{
			if (*c == L'%')
			{
				long uch = wcstol(&(c[1]), &end, 16);
				c += 2;
				dest.push_back((char)uch);
			}
			else
			if (*c == L'+')
			{
				dest.push_back(L' ');
			}
			else
			{
				dest.push_back(*c);
			}
		
		}
		return dest;
	}

	static std::wstring xmlencode(const std::wstring& xml)
	{
		std::wstring exml = xml;
		size_t xpos = -1;
		while ((xpos = exml.find_first_of(L"&<>", xpos+1))!=std::wstring::npos)
		{
			switch (exml.at(xpos))
			{
				case L'&': {
					exml.erase(xpos, 1);
					exml.insert(xpos, L"&amp;");
					xpos += 4;
				} break;
				case L'<': {
					exml.erase(xpos, 1);
					exml.insert(xpos, L"&lt;");
					xpos += 3;
				} break;
				case L'>': {
					exml.erase(xpos, 1);
					exml.insert(xpos, L"&gt;");
					xpos += 3;
				} break;
			}
		}
		return exml;
	}

	static std::wstring xmldecode(const std::wstring& xml)
	{
		std::wstring exml = xml;
		size_t xpos = -1;
		while ((xpos = exml.find(L"&lt;", xpos+1))!=std::wstring::npos)
		{
			exml.erase(xpos, 4);
			exml.insert(xpos, L"<");
		}

		xpos = -1;
		while ((xpos = exml.find(L"&gt;", xpos+1))!=std::wstring::npos)
		{
			exml.erase(xpos, 4);
			exml.insert(xpos, L">");
		}

		xpos = -1;
		while ((xpos = exml.find(L"&amp;", xpos+1))!=std::wstring::npos)
		{
			exml.erase(xpos, 5);
			exml.insert(xpos, L"&");
		}

		return exml;
	}
};

#include <lm.h>
#pragma comment(lib, "Netapi32.lib")
class GNUtils
{
public:
	inline static bool loadResource(HINSTANCE hInstance, IN LPCTSTR _name, IN LPCTSTR _type, OUT char** _buffer, OUT LPDWORD psize)
	{
		HRSRC hres = FindResource(hInstance, _name, _type);
		if (!hres)
			return false;

		DWORD dwSize = SizeofResource(hInstance, hres);
		if (!dwSize)
			return false;
		HGLOBAL hResource = ::LoadResource(hInstance, hres);
		const void* pSrc = ::LockResource(hResource);
		if (!pSrc)
			return false;

		*_buffer = (char*) new char[dwSize+1];
		(*_buffer)[dwSize] = 0;

		if (!pSrc || !*_buffer)
		{
			UnlockResource(hResource);
			FreeResource(hres);
			return false;
		};

		CopyMemory(*_buffer, pSrc, dwSize);
		*psize = dwSize;
	
		UnlockResource(hResource);
		FreeResource(hResource);

		return true;
	}

	inline static bool loadResource(HINSTANCE hInstance, IN LPCTSTR _name, IN LPCTSTR _type, std::string& data)
	{
		HRSRC hres = FindResource(hInstance, _name, _type);
		if (!hres)
			return false;

		DWORD dwSize = SizeofResource(hInstance, hres);
		if (!dwSize)
			return false;
		HGLOBAL hResource = ::LoadResource(hInstance, hres);
		const void* pSrc = ::LockResource(hResource);
		if (!pSrc)
			return false;

		data.reserve(dwSize+1);
		data.append((const char *)pSrc, dwSize);

		UnlockResource(hResource);
		FreeResource(hResource);

		return true;
	}

	inline static char *loadResource(HINSTANCE hInst, IN LPCTSTR _name, IN LPCTSTR _type, OUT LPDWORD psize = NULL)
	{
		char *content = NULL;
		DWORD count = 0;
		TCHAR szfn[_MAX_PATH];
		_tcscpy_s<_MAX_PATH>(szfn, _name);
		::PathStripPath(szfn);
		if (GNUtils::loadResource(hInst, szfn, _type, &content, &count)) {
			if (psize) psize[0] = count;
			return content;
		}
		return NULL;
	}

#ifdef _AFX_
	inline static char *loadResource(IN LPCTSTR _name, IN LPCTSTR _type, OUT LPDWORD psize = NULL)
	{
		return LoadResource(::AfxGetResourceHandle(), szfn, _type, psize);
	}

	inline static char *loadGLSL(const TCHAR *fn)
	{
		return loadResource(fn, _T("GLSL"));
	}
#endif

	inline static void log(const wchar_t* form, ...)
	{
		CString logdata, logmsg;
		SYSTEMTIME sm; GetLocalTime(&sm);
		logdata.Format(L"[%02d:%02d:%02d.%03d] ", sm.wHour, sm.wMinute, sm.wSecond, sm.wMilliseconds);

		va_list args;
		va_start(args, form);
		logmsg.FormatV(form, args);
		va_end(args);
		logmsg.Append(L"\n");
		logdata.Append(logmsg);
		OutputDebugString(logdata.GetString());
	}
	
	inline static void log(const char* form, ...)
	{
		CStringA logdata, logmsg;
		SYSTEMTIME sm; GetLocalTime(&sm);
		logdata.Format("[%02d:%02d:%02d.%03d] ", sm.wHour, sm.wMinute, sm.wSecond, sm.wMilliseconds);

		va_list args;
		va_start(args, form);
		logmsg.FormatV(form, args);
		va_end(args);
		logmsg.Append("\n");
		logdata.Append(logmsg);
		OutputDebugStringA(logdata.GetString());
	}
	
	inline static std::wstring LogTime()
	{
		CString logdata;
		SYSTEMTIME sm; GetLocalTime(&sm);
		logdata.Format(L"%04d/%02d/%02d %02d:%02d:%02d", sm.wYear, sm.wMonth, sm.wDay, sm.wHour, sm.wMinute, sm.wSecond);
		return logdata.GetString();
	}

	inline static bool BrowseFolderPath(CString& sPath, LPCTSTR szTitle = _T("Select Folder"))
	{
		BROWSEINFO   bi; 
		TCHAR szDisplayName[_MAX_PATH] = _T("");
		TCHAR szPathName[_MAX_PATH] = _T("");

		ZeroMemory(&bi, sizeof(bi));
		bi.pszDisplayName = szDisplayName;
		bi.lpszTitle = szTitle;
		bi.ulFlags = BIF_RETURNONLYFSDIRS;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
		if (pidl != NULL)
		{
			BOOL bRet = ::SHGetPathFromIDList(pidl,szPathName);
			if (FALSE == bRet)
				return false;
			sPath = szPathName;
			return true;
		}
		return false;
	}

	inline static bool BringWindowToTop(HWND hWnd)
	{
		if(::GetForegroundWindow() != hWnd) {
			HWND h_active_wnd = ::GetForegroundWindow();
			if (h_active_wnd != NULL) { 
				DWORD thread_id = GetWindowThreadProcessId(h_active_wnd, NULL);
				DWORD current_thread_id = GetCurrentThreadId(); 
				if (current_thread_id != thread_id && AttachThreadInput(current_thread_id, thread_id, TRUE)) {
					::BringWindowToTop(hWnd);
					AttachThreadInput(current_thread_id, thread_id, FALSE);
					return true;
				}
			}
		}
		return false;
	}
	
	inline static bool DeleteShareFolder(const CString& shareName)
	{
		NET_API_STATUS res;
		res = NetShareDel(NULL, (LPTSTR)shareName.GetString(), 0);
		return res == NERR_Success;
	}

	inline static bool CreateShareFolder(const CString& shareName, const CString& sharePath)
	{
		NET_API_STATUS res;
		SHARE_INFO_2 p;
		DWORD parm_err = 0;

		p.shi2_netname = (LPTSTR)shareName.GetString();
		p.shi2_type = STYPE_DISKTREE; // disk drive
		p.shi2_remark = (LPTSTR)shareName.GetString();
		p.shi2_permissions = 0;
		p.shi2_max_uses = 4;
		p.shi2_current_uses = 0;
		p.shi2_path = (LPTSTR)sharePath.GetString();
		p.shi2_passwd = NULL; // no password

		res = NetShareAdd(NULL, 2, (LPBYTE) &p, &parm_err);
		return res == NERR_Success;
	}

	inline static bool KillProcess(const CString& KillName)
	{
		HANDLE hProcessSnap = NULL;
		bool Return = false;
		PROCESSENTRY32 pe32 = {0};
		CString ProcessName = KillName;
		ProcessName.MakeLower();
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE)
			return false;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32))
		{
			DWORD Code = 0;
			DWORD dwPriorityClass;
			do
			{
				HANDLE hProcess;
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
				dwPriorityClass = GetPriorityClass(hProcess);
				CString Temp = pe32.szExeFile;
				Temp.MakeLower();
				if(Temp == ProcessName)
				{
					if(TerminateProcess(hProcess, 0))
						GetExitCodeProcess(hProcess, &Code);
					else
						return false;
				}
				CloseHandle(hProcess);
			} while(Process32Next(hProcessSnap, &pe32));
			Return = true;
		}
		else
		{
			Return = false;
		}
		CloseHandle(hProcessSnap);
		return Return;
	}

	template<typename _TY>
	inline static bool split(std::vector<_TY>& ary, const _TY& tokee, const _TY& token)
	{
		ary.clear();
		size_t spos = 0, epos = -1;
		do
		{
			epos = tokee.find(token, spos);
			if (epos != _TY::npos)
			{
				_TY item = tokee.substr(spos, epos-spos);
				ary.push_back(item);
				spos = epos + token.size();
			}
			else
			{
				_TY item = tokee.substr(spos);
				if (!item.empty())
					ary.push_back(item);
			}
		} while(epos != _TY::npos);
		
		return ! ary.empty();
	}

	// 해상도를 변경
	inline static bool changeDisplay(int width, int height, int bpp)
	{
		DEVMODE mode = {0,};
		mode.dmSize = sizeof (DEVMODE);
		mode.dmBitsPerPel = bpp;
		mode.dmPelsWidth  = width;
		mode.dmPelsHeight = height;
		mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		::ChangeDisplaySettings(&mode, CDS_FULLSCREEN);
		return true;
	}

	inline static const std::wstring AppPath(const std::wstring& file = L"")
	{
		WCHAR szTemp[_MAX_PATH];
		::GetModuleFileName(NULL, szTemp, _MAX_PATH);
		::PathRemoveFileSpec(szTemp);
		if (!file.empty())
			::PathAppend(szTemp, file.c_str());
		return szTemp;
	}

	inline static const std::wstring AppExt(const std::wstring& ext = L"")
	{
		WCHAR szTemp[_MAX_PATH];
		::GetModuleFileName(NULL, szTemp, _MAX_PATH);
		if (!ext.empty()) {
			::PathRemoveExtension(szTemp);
			::PathAddExtension(szTemp, ext.c_str());
		}
		return szTemp;
	}
	
	inline static const std::wstring AppFile(const std::wstring& ext = L"")
	{
		WCHAR szTemp[_MAX_PATH];
		::GetModuleFileName(NULL, szTemp, _MAX_PATH);
		if (!ext.empty()) {
			::PathRemoveExtension(szTemp);
			::PathAddExtension(szTemp, ext.c_str());
		}
		return szTemp;
	}
	
	inline static const std::wstring AppVersion()
	{
		CString m_sVersion(TEXT("1.0.0.0")), filename = GNUtils::AppFile().c_str();
		DWORD h_version_handle;
		DWORD version_info_size = GetFileVersionInfoSize(filename, &h_version_handle);
		HANDLE h_memory = GlobalAlloc(GMEM_MOVEABLE, version_info_size); 
		LPVOID p_info_memory = GlobalLock(h_memory);
		GetFileVersionInfo(filename, h_version_handle, version_info_size, p_info_memory);
		struct LANGANDCODEPAGE {WORD wLanguage; WORD wCodePage;} *lpTranslate;
		UINT cbTranslate = 0;
		VerQueryValue(p_info_memory, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate);
		for(int i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++ )
		{
			CString SubBlock;
			SubBlock.Format(TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
			wchar_t *lpBuffer = NULL;
			UINT dwBytes = 0;
			if (VerQueryValue(p_info_memory, SubBlock, (LPVOID*)&lpBuffer, &dwBytes))
			{
				m_sVersion.Empty();
				m_sVersion.Append(lpBuffer);
				m_sVersion.Replace(L", ", L".");
				break;
			}
		}
		GlobalUnlock(h_memory); 
		GlobalFree(h_memory);
		return m_sVersion.GetString();
	}
	
	inline static void EnableDragNDropFilter()
	{
		HMODULE hMOD = LoadLibraryA("USER32.DLL");
		FARPROC _msgFilter = GetProcAddress(hMOD, "ChangeWindowMessageFilter");
		if (_msgFilter)
		{
			((BOOL (WINAPI*)(UINT message, DWORD dwFlag))_msgFilter)(0x0233, 1);
			((BOOL (WINAPI*)(UINT message, DWORD dwFlag))_msgFilter)(0x004A, 1);
			((BOOL (WINAPI*)(UINT message, DWORD dwFlag))_msgFilter)(0x0049, 1);
		}
		FreeLibrary(hMOD);
	}

	// trim from start
	template<typename _TY>
	static inline _TY &ltrim(_TY &s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	// trim from end
	template<typename _TY>
	static inline _TY &rtrim(_TY &s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	// trim from both ends
	template<typename _TY>
	static inline _TY &trim(_TY &s) {
		return ltrim(rtrim(s));
	}

	template<typename _TY>
	inline static bool join(std::vector<_TY>& ary, _TY& tokee, const _TY& token)
	{
		tokee.clear();
		if (!ary.empty())
			tokee.append(ary.at(0));

		for (size_t i = 1; i < ary.size(); i++)
		{
			tokee.append(token);
			tokee.append(ary.at(i));
		}
		return ! tokee.empty();
	}

	template<typename _TY>
	inline static bool trim(std::vector<_TY>& ary)
	{
		for (size_t i = 0; i < ary.size(); i++)
			trim(ary.at(i));
		return true;
	}
};

class GNMutex
{
STATIC_INSTANCE(GNMutex)
protected:
	HANDLE	m_hMUTEX;
	LPCTSTR	m_sMutexName;
public:
	GNMutex(): m_hMUTEX(NULL), m_sMutexName(NULL) {}
	~GNMutex() { if (m_hMUTEX) CloseHandle(m_hMUTEX); m_hMUTEX = NULL; }
	GNMutex& SetName(LPCTSTR sMutexName) { if (!m_hMUTEX) m_sMutexName = sMutexName; return *this; }
	bool Unlock() { if (m_hMUTEX) ::ReleaseMutex(m_hMUTEX); }
	bool Lock()
	{
		m_hMUTEX = ::OpenMutex(MUTEX_ALL_ACCESS, FALSE, m_sMutexName);
		if (m_hMUTEX)
		{
			::CloseHandle(m_hMUTEX);
			m_hMUTEX = NULL;
			return false;
		}
		if (!m_hMUTEX)
			m_hMUTEX = ::CreateMutex(NULL, TRUE, m_sMutexName);
		if (m_hMUTEX == INVALID_HANDLE_VALUE)
			m_hMUTEX = NULL;
		if (! m_hMUTEX)
			return false;
		if (WaitForSingleObject(m_hMUTEX, 0)==WAIT_OBJECT_0)
			return true;
		return false;
	}
};
