#pragma once

#include <map>
#include <vector>
#include <string>
#include <algorithm>

typedef std::string							LINKVALUES;
typedef std::map<std::string, LINKVALUES>	LINKPARAMS;

typedef struct tagLINKDATA
{
	inline static std::string toUTF8(const std::wstring& U)
	{
		std::string utf8;
		utf8.resize(U.size() * 2);
		int nChar = WideCharToMultiByte(CP_UTF8, 0, U.c_str(), (int)U.size(), (LPSTR)utf8.c_str(), (int)utf8.size(), NULL, NULL);
		utf8.resize(nChar);
		return utf8;
	}

	inline static std::wstring fromUTF8(const std::string& utf8)
	{
		std::wstring utf16;
		utf16.resize(utf8.size());
		int nWide = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.c_str(), (int)utf8.length(), (LPWSTR)utf16.c_str(), (int)utf16.size());
		utf16.resize(nWide);
		return utf16;
	}

	inline static std::string CreateGUID()
	{
		GUID uuid;
		std::string retGUID;
		RPC_CSTR retUUID;
		UuidCreate(&uuid);
		UuidToStringA(&uuid, (RPC_CSTR*)&retUUID);
		retGUID.append("{").append((LPCSTR)retUUID).append("}");
		RpcStringFreeA(&retUUID);

		return retGUID;
	}

	inline static std::wstring CreateGUIDW()
	{
		std::string tempGUID = CreateGUID();
		std::wstring retGUID;
		retGUID.assign(tempGUID.begin(), tempGUID.end());
		return retGUID;
	}


	inline static LINKVALUES sDATA(const std::string& str)
	{
		ULONG nsize = (ULONG)str.size();

		std::string value;
		value.clear();
		value.append("s");
		value.append((const char *)&nsize, sizeof(nsize));
		value.append(str.c_str(), nsize);

		return value;
	}

	inline static LINKVALUES wDATA(const std::wstring& wstr, LINKVALUES& value)
	{
		std::string str = toUTF8(wstr);
		ULONG nsize = (ULONG)str.size();

		//std::string value;
		value.clear();
		value.append("w");
		value.append((const char *)&nsize, sizeof(nsize));
		value.append((const char *)str.c_str(), nsize);

		return value;
	}

	inline static LINKVALUES vDATA(const void *data, ULONG nSize)
	{
		std::string value;
		value.clear();
		value.append("v");
		value.append((const char *)&nSize, sizeof(nSize));
		value.append((const char *)data, nSize);

		return value;
	}

	inline static LINKVALUES mDATA(const std::string& k, const std::string& v)
	{
		ULONG nSize;

		std::string value;
		value.clear();
		value.append("m");
		nSize = (ULONG)k.size();
		value.append((const char *)&nSize, sizeof(nSize));
		value.append((const char *)k.c_str(), nSize);
		nSize = (ULONG)v.size();
		value.append((const char *)&nSize, sizeof(nSize));
		value.append((const char *)v.c_str(), nSize);

		return value;
	}

	inline static LINKVALUES intDATA(int nData)
	{
		std::string value;
		value.clear();
		value.append("i");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES longDATA(long nData)
	{
		std::string value;
		value.clear();
		value.append("l");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES shortDATA(short nData)
	{
		std::string value;
		value.clear();
		value.append("n");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES uintDATA(unsigned int nData)
	{
		std::string value;
		value.clear();
		value.append("i");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES ulongDATA(unsigned long nData)
	{
		std::string value;
		value.clear();
		value.append("l");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES ushortDATA(unsigned short nData)
	{
		std::string value;
		value.clear();
		value.append("n");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES floatDATA(float nData)
	{
		std::string value;
		value.clear();
		value.append("f");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static LINKVALUES doubleDATA(double nData)
	{
		std::string value;
		value.clear();
		value.append("d");
		value.append((const char *)&nData, sizeof(nData));

		return value;
	}

	inline static bool sVALUE(const LINKVALUES& data, std::string& v)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 's')
			return false;
		ULONG size = *(ULONG *)(pointer);
		pointer += sizeof(size);
		v.append(pointer, size);
		return true;
	}

	inline static bool wVALUE(const LINKVALUES& data, std::wstring& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'w')
			return false;
		ULONG size = *(ULONG *)(pointer);
		pointer += sizeof(size);
		std::string svalue;
		svalue.append(pointer, size);
		value = fromUTF8(svalue);
		return true;
	}

	inline static bool vVALUE(const LINKVALUES& data, void * value, ULONG& size)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'v')
			return false;
		size = *(ULONG *)(pointer);
		pointer += sizeof(size);
		if (!value)
			return true;

		memcpy(value, pointer, size);

		return true;
	}

	inline static bool mVALUE(const LINKVALUES& data, std::string& k, std::string& v)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'm')
			return false;

		ULONG size;

		size = *(ULONG *)(pointer);
		pointer += sizeof(size);
		k.append(pointer, size);
		pointer += size;

		size = *(ULONG *)(pointer);
		pointer += sizeof(size);
		v.append(pointer, size);

		return true;
	}

	inline static bool intVALUE(const LINKVALUES& data, int& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'i')
			return false;

		value = *(int *)(pointer);
		return true;
	}

	inline static bool longVALUE(const LINKVALUES& data, long& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'l')
			return false;

		value = *(long *)(pointer);
		return true;
	}

	inline static bool shortVALUE(const LINKVALUES& data, short& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'n')
			return false;

		value = *(short *)(pointer);
		return true;
	}

	inline static bool uintVALUE(const LINKVALUES& data, unsigned int& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'i')
			return false;

		value = *(unsigned int *)(pointer);
		return true;
	}

	inline static bool ulongVALUE(const LINKVALUES& data, unsigned long& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'l')
			return false;

		value = *(unsigned long *)(pointer);
		return true;
	}

	inline static bool ushortVALUE(const LINKVALUES& data, unsigned short& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'n')
			return false;

		value = *(unsigned short *)(pointer);
		return true;
	}

	inline static bool floatVALUE(const LINKVALUES& data, float& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'f')
			return false;

		value = *(float *)(pointer);
		return true;
	}

	inline static bool doubleVALUE(const LINKVALUES& data, double& value)
	{
		const char * pointer = data.c_str();
		if (*pointer ++ != 'd')
			return false;

		value = *(double *)(pointer);
		return true;
	}

	inline static bool dumpLink(const LINKVALUES& imported, std::string& dump_out)
	{
		ULONG lsize = (ULONG)imported.size();
		const char *pointer = imported.c_str();
		std::string k, v;
		while (lsize > 0)
		{
			switch (*pointer ++)
			{
			case 'm':{	// named item
				ULONG size = *((ULONG*)(pointer));
				pointer += sizeof(size);
				k.clear();
				k.append(pointer, size);
				pointer += size;
				size = *((ULONG*)(pointer));
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
						dumpLink(v, dump_out);
						dump_out.append("]");
					} break;
				}

				lsize -= (ULONG)k.size();
				lsize -= (ULONG)v.size();
				lsize -= sizeof(size) * 2;
				lsize --;
					 } break;
			}
		}
		return true;
	}

	inline static bool parseLink(const LINKVALUES& imported, LINKPARAMS& params)
	{
		ULONG lsize = (ULONG)imported.size();
		const char *pointer = imported.c_str();
		std::string k, v;
		while (lsize > 0)
		{
			switch (*pointer ++)
			{
			case 'm':{	// named item
				ULONG size = *((ULONG*)(pointer));
				pointer += sizeof(size);
				k.clear();
				k.append(pointer, size);
				pointer += size;
				size = *((ULONG*)(pointer));
				pointer += sizeof(size);
				v.clear();
				v.append(pointer, size);
				pointer += size;
				params[k] = v;
				lsize -= (ULONG)k.size();
				lsize -= (ULONG)v.size();
				lsize -= sizeof(size) * 2;
				lsize --;
					 } break;
			default:
				return false;
			}
		}
		return true;
	}

	inline static bool toLink(LINKPARAMS& params, LINKVALUES& value)
	{
		value.clear();
		for (LINKPARAMS::iterator cp = params.begin(); cp != params.end(); cp ++)
			value.append(mDATA(cp->first, cp->second));

		return !value.empty();
	}

	inline static bool Push(LINKPARAMS& params, const LINKVALUES& value)
	{
		int nLen = 0;
		char szIdx[20];

		intVALUE(params["length"], nLen);
		_itoa_s<20>(nLen, szIdx, 10);
		params[szIdx] = value;
		params["length"] = intDATA(++nLen);

		return true;
	}

	inline static bool GetLength(LINKPARAMS& params, int& nLength)
	{
		return intVALUE(params["length"], nLength);
	}

	inline static bool GetItem(LINKPARAMS& params, int nPosition, LINKVALUES& linkData)
	{
		int nLen = 0;
		char szIdx[20];
		intVALUE(params["length"], nLen);

		if (nPosition < 0 || nPosition >= nLen)
			return false;

		_itoa_s<20>(nPosition, szIdx, 10);
		linkData = params[szIdx];
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

} LINKDATA;
