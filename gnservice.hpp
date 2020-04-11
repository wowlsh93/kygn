#pragma once

#include <atlstr.h>
#include <windows.h>
#include <winsvc.h>
#pragma comment(lib, "advapi32.lib")
#include <netfw.h>
#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "oleaut32.lib" )


class GNService
{
public:
	static bool Install(const CString& pPath, const CString& pName)
	{
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CREATE_SERVICE);
		if (schSCManager==0)
		{
			long nError = GetLastError();
			wprintf(L"[%s] Service Manager Open failed(%d).\n", pName, nError);
			return false;
		}
		else
		{
			SC_HANDLE schService = CreateService
			(
				schSCManager,												/* SCManager database      */
				pName,														/* name of service         */
				pName,														/* service name to display */
				SERVICE_ALL_ACCESS,											/* desired access          */
				SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS ,		/* service type            */
				SERVICE_AUTO_START,											/* start type              */
				SERVICE_ERROR_NORMAL,										/* error control type      */
				pPath,														/* service's binary        */
				NULL,														/* no load ordering group  */
				NULL,														/* no tag identifier       */
				NULL,														/* no dependencies         */
				NULL,														/* LocalSystem account     */
				NULL														/* no password             */
			);

			if (schService==0)
			{
				long nError =  GetLastError();
				wprintf(L"[%s] Service install failed(%d).\n", pName, nError);
				return false;
			}
			else
			{
				wprintf(L"[%s] Service successfully installed.\n", pName);
				CloseServiceHandle(schService);
			}
			CloseServiceHandle(schSCManager);
		}
		return true;
	}

	static bool UnInstall(const CString& pName)
	{
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager==0)
		{
			long nError = GetLastError();
			wprintf(L"[%s] Service Manager Open failed(%d).\n", pName, nError);
			return false;
		}
		else
		{
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0)
			{
				long nError = GetLastError();
				wprintf(L"[%s] Service open failed(%d).\n", pName, nError);
				return false;
			}
			else
			{
				if (!DeleteService(schService))
				{
					long nError = GetLastError();
					wprintf(L"[%s] Service uninstall failed(%d).\n", pName, nError);
					return false;
				}
				else
				{
					wprintf(L"[%s] Service successfully uninstalled.\n", pName);
				}
				CloseServiceHandle(schService);
			}
			CloseServiceHandle(schSCManager);
		}
		return true;
	}

	static bool Startup(const CString& pName)
	{
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager==0)
		{
			long nError = GetLastError();
			printf("Service Manager Open failed(%d).\n", nError);
			return false;
		}
		else
		{
			SC_HANDLE schService = OpenService( schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0)
			{
				long nError = GetLastError();
				printf("Service Open failed(%d).\n", nError);
				return false;
			}
			else
			{
				if (! StartService(schService, 0, NULL))
				{
					long nError = GetLastError();
					printf("Service Start failed(%d).\n", nError);
					return false;
				}
				CloseServiceHandle(schService);
			}
			CloseServiceHandle(schSCManager);
		}
		return true;
	}

	static bool Cleanup(const CString& pName)
	{
		SC_HANDLE schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS);
		if (schSCManager==0)
		{
			long nError = GetLastError();
			printf("Service Manager Open failed(%d).\n", nError);
			return false;
		}
		else
		{
			SC_HANDLE schService = OpenService(schSCManager, pName, SERVICE_ALL_ACCESS);
			if (schService==0)
			{
				long nError = GetLastError();
				printf("Service Open failed(%d).\n", nError);
				return false;
			}
			else
			{
				SERVICE_STATUS res;
				if (! ControlService(schService, SERVICE_CONTROL_STOP, &res))
				{
					long nError = GetLastError();
					printf("Service Control failed(%d).\n", nError);
					return false;
				}
				CloseServiceHandle(schService);
			}
			CloseServiceHandle(schSCManager);
		}
		return true;
	}
};

class GNFirewall
{

private:
	// Instantiate INetFwPolicy2
	inline static HRESULT WFCOMInitialize(INetFwPolicy2** ppNetFwPolicy2)
	{
		HRESULT hr = S_OK;

		hr = CoCreateInstance(
			__uuidof(NetFwPolicy2), 
			NULL, 
			CLSCTX_INPROC_SERVER, 
			__uuidof(INetFwPolicy2), 
			(void**)ppNetFwPolicy2);

		if (FAILED(hr))
		{
			printf("CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
			goto Cleanup;        
		}

	Cleanup:
		return hr;
	}

public:
	inline static bool addServiceRule(LPCWSTR szRuleGroup, LPCWSTR szRuleName, LPCWSTR szRulePort, LPCWSTR szAppPath)
	{
		HRESULT hrComInit = S_OK;
		HRESULT hr = S_OK;

		INetFwPolicy2 *pNetFwPolicy2 = NULL;
		INetFwRules *pFwRules = NULL;
		INetFwRule *pFwRule = NULL;

		long CurrentProfilesBitMask = 0;

		BSTR bstrRuleName			= SysAllocString(L"SERVICE_RULE");
		BSTR bstrRuleDescription	= SysAllocString(szRuleName);
		BSTR bstrRuleGroup			= SysAllocString(szRuleName);
		BSTR bstrRuleApplication	= SysAllocString(szAppPath);
		BSTR bstrRuleService		= SysAllocString(szRuleName);
		BSTR bstrRuleLPorts			= SysAllocString(szRulePort);

		// Initialize COM.
		hrComInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED );

		// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
		// initialized with a different mode. Since we don't care what the mode is,
		// we'll just use the existing mode.
		if (hrComInit != RPC_E_CHANGED_MODE)
		{
			if (FAILED(hrComInit))
			{
				printf("CoInitializeEx failed: 0x%08lx\n", hrComInit);
				goto Cleanup;
			}
		}

		// Retrieve INetFwPolicy2
		hr = WFCOMInitialize(&pNetFwPolicy2);
		if (FAILED(hr))
		{
			goto Cleanup;
		}

		// Retrieve INetFwRules
		hr = pNetFwPolicy2->get_Rules(&pFwRules);
		if (FAILED(hr))
		{
			printf("get_Rules failed: 0x%08lx\n", hr);
			goto Cleanup;
		}

		// Retrieve Current Profiles bitmask
		hr = pNetFwPolicy2->get_CurrentProfileTypes(&CurrentProfilesBitMask);
		if (FAILED(hr))
		{
			printf("get_CurrentProfileTypes failed: 0x%08lx\n", hr);
			goto Cleanup;
		} 

		// When possible we avoid adding firewall rules to the Public profile.
		// If Public is currently active and it is not the only active profile, we remove it from the bitmask
		if ((CurrentProfilesBitMask & NET_FW_PROFILE2_PUBLIC) && (CurrentProfilesBitMask != NET_FW_PROFILE2_PUBLIC))
		{
			CurrentProfilesBitMask ^= NET_FW_PROFILE2_PUBLIC;
		}
		CurrentProfilesBitMask |= NET_FW_PROFILE2_ALL;

		// Create a new Firewall Rule object.
		hr = CoCreateInstance(__uuidof(NetFwRule), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwRule), (void**)&pFwRule);
		if (FAILED(hr))
		{
			printf("CoCreateInstance for Firewall Rule failed: 0x%08lx\n", hr);
			goto Cleanup;
		}

		// Populate the Firewall Rule object
		pFwRule->put_Name(bstrRuleName);
		pFwRule->put_Description(bstrRuleDescription);
		pFwRule->put_ApplicationName(bstrRuleApplication);
		pFwRule->put_ServiceName(bstrRuleService);
		pFwRule->put_Protocol(NET_FW_IP_PROTOCOL_TCP);
		pFwRule->put_LocalPorts(bstrRuleLPorts);
		pFwRule->put_Grouping(bstrRuleGroup);
		pFwRule->put_Profiles(CurrentProfilesBitMask);
		pFwRule->put_Direction(NET_FW_RULE_DIR_IN);
		pFwRule->put_Action(NET_FW_ACTION_ALLOW);
		pFwRule->put_Enabled(VARIANT_TRUE);

		// Add the Firewall Rule
		hr = pFwRules->Add(pFwRule);
		if (FAILED(hr))
		{
			printf("Firewall Rule Add failed: 0x%08lx\n", hr);
			goto Cleanup;
		}

	Cleanup:

		// Free BSTR's
		SysFreeString(bstrRuleName);
		SysFreeString(bstrRuleDescription);
		SysFreeString(bstrRuleGroup);
		SysFreeString(bstrRuleApplication);
		SysFreeString(bstrRuleService);
		SysFreeString(bstrRuleLPorts);

		// Release the INetFwRule object
		if (pFwRule != NULL)
		{
			pFwRule->Release();
		}

		// Release the INetFwRules object
		if (pFwRules != NULL)
		{
			pFwRules->Release();
		}

		// Release the INetFwPolicy2 object
		if (pNetFwPolicy2 != NULL)
		{
			pNetFwPolicy2->Release();
		}

		// Uninitialize COM.
		if (SUCCEEDED(hrComInit))
		{
			CoUninitialize();
		}

		return hr == S_OK;
	}

};