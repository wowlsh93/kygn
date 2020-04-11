#pragma once

#include <string>
#include <windows.h>
#include "gnutils.hpp"

class GNRemoteInput
{
public:
#pragma pack(push, 1)
	struct _ST_KEYBOARDDATA
	{
		WPARAM wParam;
		KBDLLHOOKSTRUCT ks;
	};

	struct _ST_MOUSEDATA
	{
		WORD w, h;
		WPARAM wParam;
		MSLLHOOKSTRUCT ms;
		DWORD flags;
	};
#pragma pack(pop)

	static bool DoKeyboardInput(const _ST_KEYBOARDDATA& pck)
	{
		INPUT Input;
		SecureZeroMemory(&Input, sizeof(Input));
		Input.type = INPUT_KEYBOARD;
		Input.ki.dwFlags = ((pck.ks.flags & LLKHF_UP)? KEYEVENTF_KEYUP : 0);
		Input.ki.wScan = (WORD)pck.ks.scanCode;
		Input.ki.wVk = (WORD)pck.ks.vkCode;
		::SendInput(1, &Input, sizeof(Input));
		//::keybd_event((BYTE)pck.ks.vkCode, (BYTE)pck.ks.scanCode, (pck.ks.flags&LLKHF_UP) ? KEYEVENTF_KEYUP : 0, pck.ks.dwExtraInfo);
		return true;
	}

	static SIZE& getSize()
	{
		static SIZE sz = {1280, 1024};
		return sz;
	}

	static bool DoMouseInput(const _ST_MOUSEDATA& pck)
	{
		DWORD flags = 0, dwMouseData = 0;
		POINT pt = {0, 0};
		switch (pck.wParam)
		{
		case WM_LBUTTONDBLCLK:
			flags |= MOUSEEVENTF_LEFTDOWN; break;
		case WM_LBUTTONDOWN:
			flags |= MOUSEEVENTF_LEFTDOWN; break;
		case WM_RBUTTONDBLCLK:
			flags |= MOUSEEVENTF_RIGHTDOWN; break;
		case WM_RBUTTONDOWN:
			flags |= MOUSEEVENTF_RIGHTDOWN; break;
		case WM_MBUTTONDBLCLK:
			flags |= MOUSEEVENTF_MIDDLEDOWN; break;
		case WM_MBUTTONDOWN:
			flags |= MOUSEEVENTF_MIDDLEDOWN; break;
		case WM_LBUTTONUP:
			flags |= MOUSEEVENTF_LEFTUP; break;
		case WM_RBUTTONUP:
			flags |= MOUSEEVENTF_RIGHTUP; break;
		case WM_MBUTTONUP:
			flags |= MOUSEEVENTF_MIDDLEUP; break;
		case WM_MOUSEWHEEL:
			flags |= MOUSEEVENTF_WHEEL;
			dwMouseData = (short)HIWORD(pck.ms.mouseData);
		break;
		default:
			flags |= MOUSEEVENTF_MOVE;
			if (!pck.flags) {
				flags |= MOUSEEVENTF_ABSOLUTE;
				pt.x = (pck.ms.pt.x * 65535) / max(1, getSize().cx);
				pt.y = (pck.ms.pt.y * 65535) / max(1, getSize().cy);
				GNUtils::log(L"mouse(%d, %d)", pt.x, pt.y);
			}
		break;
		}
#if 0
		INPUT inp;
		inp.type = INPUT_MOUSE;
		inp.mi.mouseData = dwMouseData;
		inp.mi.dx = pt.x;
		inp.mi.dy = pt.y;
		inp.mi.dwFlags = flags;
		inp.mi.dwExtraInfo = pck.ms.dwExtraInfo;
		inp.mi.time = 0;
		::SendInput(1, &inp, sizeof(inp));
#else
		::mouse_event(flags, pt.x, pt.y, dwMouseData, pck.ms.dwExtraInfo);
#endif
		return true;
	}
};

class GNRemoteHook
{
protected:
	inline static GNRemoteHook& get(GNRemoteHook* _cHook = NULL)
	{
		static GNRemoteHook oHook;
		static GNRemoteHook* cHook = NULL;
		if (_cHook)
			cHook = _cHook;
		if (!cHook)
			return oHook;
		return cHook[0];
	}
public:
	inline GNRemoteHook() : m_global(true) {}
	
	HHOOK		m_hHK, m_hHM;
	HMODULE		m_hMOD;
	bool		m_global;
	
	inline static LRESULT __cdecl keyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		return get().keyboardNext(nCode, wParam, lParam);
	}

	inline static LRESULT __cdecl mouseProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		return get().mouseNext(nCode, wParam, lParam);
	}
	
	inline void SetHook(HHOOK _HK, HHOOK _HM)
	{
		m_hHK = _HK;
		m_hHM = _HM;
	}

	inline bool install(HMODULE hMOD)
	{
#if 0
		m_hMOD = hMOD;
		if (!m_hMOD)
			return false;

		bool (CALLBACK *setHookClient)(GNRemoteHook& hookc);
		setHookClient = (bool (CALLBACK *)(GNRemoteHook& hookc))GetProcAddress(m_hMOD, "setHookClient");
		setHookClient(this[0]);
		LRESULT (CALLBACK *keyHooks)(int nCode, WPARAM wParam, LPARAM lParam);
		keyHooks = (LRESULT (CALLBACK *)(int nCode, WPARAM wParam, LPARAM lParam))GetProcAddress(m_hMOD, "keyboardHooks");
		LRESULT (CALLBACK *mouseHooks)(int nCode, WPARAM wParam, LPARAM lParam);
		mouseHooks = (LRESULT (CALLBACK *)(int nCode, WPARAM wParam, LPARAM lParam))GetProcAddress(m_hMOD, "mouseHooks");

		m_hHK = SetWindowsHookEx(WH_KEYBOARD_LL, keyHooks, m_hMOD, 0);
		if (!m_hHK)
			OutputDebugString(L"Keyboard hook failed.\n");
		m_hHM = SetWindowsHookEx(WH_MOUSE_LL, mouseHooks, m_hMOD, 0);
		if (!m_hHM)
			OutputDebugString(L"Mouse hook failed.\n");
#else
		get(this);
		m_global = true;
		m_hHK = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardProc, NULL, 0);
		if (!m_hHK)
			OutputDebugString(L"Keyboard hook failed.\n");
		m_hHM = SetWindowsHookEx(WH_MOUSE_LL, mouseProc, NULL, 0);
		if (!m_hHM)
			OutputDebugString(L"Mouse hook failed.\n");
#endif
		return true;
	}

	inline bool install_local()
	{
		get(this);
		m_global = false;
		m_hHK = SetWindowsHookEx(WH_KEYBOARD, keyboardProc, NULL, GetCurrentThreadId());
		if (!m_hHK)
			OutputDebugString(L"Keyboard hook failed.\n");
		m_hHM = SetWindowsHookEx(WH_MOUSE, mouseProc, NULL, GetCurrentThreadId());
		if (!m_hHM)
			OutputDebugString(L"Mouse hook failed.\n");
		return true;
	}

	inline bool uninstall()
	{
		while(! UnhookWindowsHookEx(m_hHM)); m_hHM = NULL;
		while(! UnhookWindowsHookEx(m_hHK)); m_hHK = NULL;
		m_hMOD = NULL;
		return true;
	}

	inline LRESULT CALLBACK keyboardNext(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode >= HC_ACTION)
		{
			LRESULT r = m_global ? keyboardLL(nCode, wParam, lParam) : keyboard(nCode, wParam, lParam);
			if (!r || nCode < 0)
				return CallNextHookEx(m_hHK, nCode, wParam, lParam);
			return r;
		}
		return CallNextHookEx(m_hHK, nCode, wParam, lParam);
	}

	inline LRESULT CALLBACK mouseNext(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode >= HC_ACTION)
		{
			LRESULT r = m_global ? mouseLL(nCode, wParam, lParam) : mouse(nCode, wParam, lParam);
			if (!r)
				return CallNextHookEx(m_hHM, nCode, wParam, lParam);
			return r;
		}
		return CallNextHookEx(m_hHM, nCode, wParam, lParam);
	}

	inline virtual LRESULT keyboardLL(int nCode, WPARAM wParam, LPARAM lParam) { return 0; }
	inline virtual LRESULT mouseLL(int nCode, WPARAM wParam, LPARAM lParam) { return 0; }
	inline virtual LRESULT keyboard(int nCode, WPARAM wParam, LPARAM lParam) { return 0; }
	inline virtual LRESULT mouse(int nCode, WPARAM wParam, LPARAM lParam) { return 0; }
};