#pragma once

#include <string>
#include <atlstr.h>
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

class GNNetUtils
{
public:
	inline static bool GetMacAddress(std::vector<CString>& strMacArray, CString strSpecifiedIP)
	{
		PIP_ADAPTER_INFO pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;
		DWORD dwRetVal = 0;

		ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
		pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];

		if (pAdapterInfo == NULL)
			return false;

		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			delete pAdapterInfo;
			pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];
			if (pAdapterInfo == NULL)
				return false;
		}

		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
		{
			pAdapter = pAdapterInfo;
			while (pAdapter)
			{
				CStringA strMacAddress;
				strMacAddress.Format("%02X:%02X:%02X:%02X:%02X:%02X",
					pAdapter->Address[0],
					pAdapter->Address[1],
					pAdapter->Address[2],
					pAdapter->Address[3],
					pAdapter->Address[4],
					pAdapter->Address[5]);

				CStringA strIPAddress;
				strIPAddress.Format("%s", pAdapter->IpAddressList.IpAddress.String);

				if(strSpecifiedIP.GetLength() > 0)
				{
					if(strIPAddress.Find(CT2A(strSpecifiedIP)) >= 0)
					{
						strMacArray.push_back((LPWSTR)CA2T(strMacAddress));
						break;
					}
				}
				else
					strMacArray.push_back((LPWSTR)CA2T(strMacAddress));

				pAdapter = pAdapter->Next;
			}
		}

		delete pAdapterInfo;
		pAdapterInfo = NULL;
		return true;
	}

	inline static ULONG nameHost(const std::string& hostName)
	{
		ULONG host = ::inet_addr(hostName.c_str());
		if (host!=INADDR_NONE)
			return host;

		HOSTENT* ent = ::gethostbyname(hostName.c_str());
		if (ent && ent->h_length)
			host = *(ULONG *)(ent->h_addr_list[0]);

		return host;
	}

	inline static std::string remoteName(const std::string& hostip)
	{
		in_addr addr = {};
		addr.s_addr = ::inet_addr(hostip.c_str());
		hostent* pent = ::gethostbyaddr((const char *)&addr, 4, AF_INET);
		if (pent)
			return std::string(pent->h_name);
		return "";
	}
	
	inline static int connect_timeout(SOCKET sock, SOCKADDR_IN& addr, int mills = 0)
	{
		if (mills)
		{
			fd_set set;
			FD_ZERO(&set);
			timeval tvout = {mills/1000, mills%1000};
			FD_SET(sock, &set);
			
			unsigned long arg = 1;
			ioctlsocket(sock, FIONBIO, &arg);
			
			if ((connect(sock, (struct sockaddr *)&addr, sizeof(addr))) != 0)
			{
				if ( WSAGetLastError() != WSAEINPROGRESS && WSAGetLastError() != WSAEWOULDBLOCK)
					return -1;
			}
			if (select(((int)sock+1), NULL, &set, NULL, &tvout) <= 0)
				return -1;

			return 0;
		}
		return connect(sock, (struct sockaddr *)&addr, sizeof(addr));
	}
};