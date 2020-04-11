#pragma once

#include <map>
#include <vector>
#include <string>
#include "gnutils.hpp"

typedef std::string							GNLINKVALUES;
typedef std::map<std::string, GNLINKVALUES>	GNLINKPARAMS;

typedef struct tagGNLINKDATA
{
	inline static GNLINKVALUES szDATA(size_t size)
	{
		GNLINKVALUES values;
		if (size < 0x100)
		{
			values.append("b");
			BYTE sz = (BYTE)size;
			values.append((LPCSTR)&sz, sizeof(sz));
		}
		else if (size < 0x10000)
		{
			values.append("n");
			WORD sz = (WORD)size;
			values.append((LPCSTR)&sz, sizeof(sz));
		}
		else
		{
			values.append("u");
			DWORD sz = (DWORD)size;
			values.append((LPCSTR)&sz, sizeof(sz));
		}
		return values;
	}
	
	inline static GNLINKVALUES mDATA(const std::string& k, const std::string& v)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("m");
		value.append(szDATA(k.size()));
		value.append((LPCSTR)k.c_str(), k.size());
		value.append(szDATA(v.size()));
		value.append((LPCSTR)v.c_str(), v.size());
		return value;
	}

	inline static GNLINKVALUES S1DATA(const std::string& str)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("s");
		value.append(szDATA(str.size()));
		value.append((LPCSTR)str.c_str(), str.size());

		return value;
	}

	inline static GNLINKVALUES S2DATA(const std::wstring& wstr)
	{
		std::string str = GNCodec::toUTF8(wstr);
		GNLINKVALUES value;
		value.append("w");
		value.append(szDATA(str.size()));
		value.append((LPCSTR)str.c_str(), str.size());

		return value;
	}

	inline static GNLINKVALUES VDATA(const void *data, ULONG nSize)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("v");
		value.append(szDATA(nSize));
		value.append((LPCSTR)data, nSize);

		return value;
	}

	inline static GNLINKVALUES U1DATA(BYTE nData)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("b");
		value.append((LPCSTR)&nData, sizeof(nData));
		return value;
	}

	inline static GNLINKVALUES U2DATA(WORD nData)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("n");
		value.append((LPCSTR)&nData, sizeof(nData));
		return value;
	}

	inline static GNLINKVALUES U4DATA(DWORD nData)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("u");
		value.append((LPCSTR)&nData, sizeof(nData));
		return value;
	}
	inline static GNLINKVALUES F1DATA(float nData)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("f");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static GNLINKVALUES F2DATA(double nData)
	{
		GNLINKVALUES value;
		value.clear();
		value.append("d");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static bool szVALUE(const char *& ptr, DWORD& size, const char ** pdst = NULL)
	{
		if (ptr[0] == 'b')
		{
			BYTE sz;
			memcpy(&sz, ptr+1, sizeof(sz));
			size = (DWORD)sz;
			if (pdst) pdst[0] = ptr + 2;
			return true;
		}
		
		if (ptr[0] == 'n')
		{
			WORD sz;
			memcpy(&sz, ptr+1, sizeof(sz));
			size = (DWORD)sz;
			if (pdst) pdst[0] = ptr + 3;
			return true;
		}

		if (ptr[0] == 'u')
		{
			DWORD sz;
			memcpy(&sz, ptr+1, sizeof(sz));
			size = (DWORD)sz;
			if (pdst) pdst[0] = ptr + 5;
			return true;
		}
		return false;
	}

	inline static bool S1VALUE(const GNLINKVALUES& data, std::string& v)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 's')
			return false;
		DWORD size;
		szVALUE(pointer, size, &pointer);
		v.append(pointer, size);
		return true;
	}

	inline static bool S2VALUE(const GNLINKVALUES& data, std::wstring& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'w')
			return false;
		DWORD size;
		szVALUE(pointer, size, &pointer);
		std::string svalue;
		svalue.append(pointer, size);
		value = GNCodec::fromUTF8(svalue);
		return true;
	}

	inline static bool vVALUE(const GNLINKVALUES& data, void * value, ULONG& size)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'v')
			return false;
		szVALUE(pointer, size, &pointer);
		if (!value)
			return true;
		memcpy(value, pointer, size);
		return true;
	}

	inline static bool vVALUE(const GNLINKVALUES& data, std::string& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'v')
			return false;
		DWORD sz;
		szVALUE(pointer, sz, &pointer);
		value.resize(sz);
		memcpy((LPVOID)value.c_str(), pointer, sz);
		return true;
	}

	inline static bool mVALUE(const GNLINKVALUES& data, std::string& k, std::string& v)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'm')
			return false;
		DWORD sz;
		szVALUE(pointer, sz, &pointer);
		k.resize(sz);
		memcpy((LPVOID)k.c_str(), pointer, sz);
		pointer += sz;

		szVALUE(pointer, sz, &pointer);
		v.resize(sz);
		memcpy((LPVOID)v.c_str(), pointer, sz);
		return true;
	}

	inline static bool U4VALUE(const GNLINKVALUES& data, DWORD& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'u')
			return false;
		value = *(DWORD*)(pointer);
		return true;
	}

	inline static bool U2VALUE(const GNLINKVALUES& data, WORD& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'n')
			return false;
		value = *(WORD*)(pointer);
		return true;
	}

	inline static bool U1VALUE(const GNLINKVALUES& data, BYTE& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'n')
			return false;
		value = *(BYTE*)(pointer);
		return true;
	}

	inline static bool F1VALUE(const GNLINKVALUES& data, FLOAT& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'f')
			return false;
		value = *(FLOAT*)(pointer);
		return true;
	}

	inline static bool F2VALUE(const GNLINKVALUES& data, DOUBLE& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'd')
			return false;
		value = *(DOUBLE *)(pointer);
		return true;
	}

	inline static bool dumpLINK(const GNLINKVALUES& imported, std::string& dump_out)
	{
		ULONG lsize = (ULONG)imported.size();
		const char *pointer = imported.c_str();
		std::string k, v;
		while (lsize > 0)
		{
			switch (*pointer ++)
			{
				case 'm':{	// named item
					BYTE wSize = *((BYTE*)(pointer));
					pointer += sizeof(wSize);
					k.clear();
					k.append(pointer, wSize);
					pointer += wSize;

					ULONG size = *((ULONG*)(pointer));
					pointer += sizeof(size);
					v.clear();
					v.append(pointer, size);
					pointer += size;

					if (!dump_out.empty())
						dump_out.append(" ,");
					dump_out.append(k).append(":");
					switch (v.at(0))
					{
						case 'i':
						{
							dump_out.append("(int)");
						} break;

						case 'n':
						{
							dump_out.append("(short)");
						} break;

						case 'l':
						{
							dump_out.append("(long)");
						} break;

						case 'f':
						{
							dump_out.append("(float)");
						} break;

						case 'd':
						{
							dump_out.append("(double)");
						} break;
					
						case 's':
						{
							dump_out.append("(string)");
						} break;

						case 'w':
						{
							dump_out.append("(wstring)");
						} break;

						case 'v':
						{
							dump_out.append("(void*)");
						} break;

						case 'm':
						{
							dump_out.append("(map)[");
							dumpLINK(v, dump_out);
							dump_out.append("]");
						} break;
					}

					lsize -= (ULONG)k.size();
					lsize -= (ULONG)v.size();
					lsize -= sizeof(size);
					lsize -= sizeof(wSize);
					lsize --;
				} break;
			}
		}
		return true;
	}

	inline static bool fromLINK(const GNLINKVALUES& imported, GNLINKPARAMS& params)
	{
		ULONG lsize = (ULONG)imported.size();
		const char *pointer = imported.c_str();
		std::string k, v;
		params.clear();
		while ((pointer - imported.c_str()) < (int)lsize)
		{
			switch (*pointer ++)
			{
				case 'm':{	// named item
					DWORD sz;
					szVALUE(pointer, sz, &pointer);
					k.clear();
					k.append(pointer, sz);
					pointer += sz;

					szVALUE(pointer, sz, &pointer);
					v.clear();
					v.append(pointer, sz);
					pointer += sz;
					params[k] = v;
				} break;
				default:
				return false;
			}
		}
		return true;
	}

	inline static bool toLINK(GNLINKPARAMS& params, GNLINKVALUES& value)
	{
		value.clear();
		for (GNLINKPARAMS::iterator cp = params.begin(); cp != params.end(); cp ++)
			value.append(mDATA(cp->first, cp->second));
		return !value.empty();
	}

	inline static bool Push(GNLINKPARAMS& params, const GNLINKVALUES& value)
	{
		DWORD nLen = 0;
		char szIdx[20];

		U4VALUE(params["__$length"], nLen);
		_itoa_s<20>(nLen, szIdx, 10);
		params[szIdx] = value;
		params["__$length"] = U4DATA(++nLen);

		return true;
	}

	inline static bool GetLength(GNLINKPARAMS& params, DWORD& nLength)
	{
		return U4VALUE(params["__$length"], nLength);
	}

	inline static bool GetItem(GNLINKPARAMS& params, int nPosition, GNLINKVALUES& GNLINKDATA)
	{
		DWORD nLen = 0;
		char szIdx[20];
		U4VALUE(params["__$length"], nLen);
	
		if (nPosition < 0 || nPosition >= (int)nLen)
			return false;

		_itoa_s<20>(nPosition, szIdx, 10);
		GNLINKDATA = params[szIdx];
		return true;
	}

	inline static bool Split(const std::wstring& text, const std::wstring& token, std::vector<std::wstring>& vecTokens)
	{
		size_t epos = 0, pos;
		while ( (pos = text.find(token, epos))!=std::wstring::npos)
		{
			std::wstring item = text.substr(epos, pos-epos);
			epos = pos + token.size();
			vecTokens.push_back(item);
		}
		if (!text.substr(epos).empty())
			vecTokens.push_back(text.substr(epos));

		return true;
	} 

	inline static bool Join(const std::vector<std::wstring>& vecItems, const std::wstring& joiner, std::wstring& joins)
	{
		if (vecItems.empty())
			return false;

		joins = vecItems[0];
		for (size_t i = 1; i < vecItems.size(); i++)
			joins.append(joiner).append(vecItems[i]);

		return true;
	}

} GNLINKDATA;
