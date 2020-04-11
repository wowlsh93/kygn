#pragma once

#include <vector>
#include <windows.h>
#include "gnprofile.hpp"

class GNInput
{
protected:
	bool		m_keys[256];
	bool		m_upds[256];
	bool		m_time[256];
	DWORD		m_lpush[256];
	DWORD		m_push[256];
	DWORD		m_pops[256];
	CPoint		m_op, m_cp;
	HWND		m_hWnd;
	DWORD		m_tick;

public:
	std::vector<WORD>	m_acts;

	inline static GNInput& get() {
		static GNInput s_base;
		return s_base;
	}
	
	inline GNInput()
	: m_hWnd(0)
	{
		m_op = m_cp = CPoint(0, 0);
		memset(m_keys, 0, sizeof(m_keys));
		memset(m_upds, 0, sizeof(m_upds));
		memset(m_time, 0, sizeof(m_time));
		memset(m_lpush, 0, sizeof(m_lpush));
		memset(m_push, 0, sizeof(m_push));
		memset(m_pops, 0, sizeof(m_pops));
	}
	
	inline void setHwnd(HWND _hWnd)
	{
		m_hWnd = _hWnd;
	}

	inline void update(bool doFocus = true)
	{
		m_tick = GetTickCount();
		m_op = m_cp;
		GetCursorPos(&m_cp);

		memcpy(m_upds, m_keys, sizeof(m_upds));

		GUITHREADINFO guiInfo = {sizeof(GUITHREADINFO), };
		DWORD nWndID = ::GetWindowThreadProcessId(m_hWnd, NULL);
		::GetGUIThreadInfo(nWndID, &guiInfo);
		bool bActive = !m_hWnd || (guiInfo.hwndFocus == m_hWnd);

		if (doFocus && !bActive && (GetAsyncKeyState(VK_LBUTTON)&0x8000) && WindowFromPoint(m_cp) == m_hWnd)
		{
			DWORD nTrdID = ::GetCurrentThreadId();
			AttachThreadInput(nTrdID, nWndID, TRUE);
			::SetFocus(m_hWnd);
			AttachThreadInput(nTrdID, nWndID, FALSE);
			bActive = true;
		}

		if (m_hWnd)
			::ScreenToClient(m_hWnd, &m_cp);

		m_acts.clear();
		for (int i = 0; i < 256; ++ i)
		{
			if (i == 0x40 || i == 0xFF) continue;
			m_keys[i] = bActive ? ((GetAsyncKeyState(i)&0x8000)!=0) : false;
			if (isPress(i)) m_acts.push_back(i); else m_time[i] = false;
			if (isDown(i)) { m_lpush[i] = m_push[i]; m_push[i] = m_tick; }
			if (isUp(i)) m_pops[i] = m_tick;
		}
	}

	inline bool isChanged(WORD vk)
	{
		return m_keys[vk] != m_upds[vk];
	}
	inline bool isPress(WORD vk)
	{
		return m_keys[vk];
	}
	inline bool isDown(WORD vk)
	{
		return (isChanged(vk) && isPress(vk));
	}
	inline bool isUp(WORD vk)
	{
		return (isChanged(vk) && !isPress(vk));
	}

	inline bool isAnyKey()
	{
		for (int i = 0; i < 256; ++ i)
			if (isPress(i))
				return true;
		return false;
	}
	
	inline bool isAnyKeyDown()
	{
		for (int i = 0; i < 256; ++ i)
			if (isDown(i))
				return true;
		return false;
	}
	
	inline bool isAnyKeyUp()
	{
		for (int i = 0; i < 256; ++ i)
			if (isUp(i))
				return true;
		return false;
	}

	inline bool isTimePress(WORD vk, DWORD ticks = 300)
	{
		if (!m_time[vk] && isPress(vk) && (m_tick - m_push[vk]) > ticks) {
			m_time[vk] = true;
			return true;
		}
		return false;
	}

	inline bool isDouble(WORD vk, DWORD ticks = 300)
	{
		return isDoubleUp(vk, ticks);
	}
	
	inline bool isDoubleDown(WORD vk, DWORD ticks = 300)
	{
		if (isDown(vk) && (m_tick - m_lpush[vk]) < ticks)
			return true;
		return false;
	}

	inline bool isDoubleUp(WORD vk, DWORD ticks = 300)
	{
		if (isUp(vk) && (m_tick - m_lpush[vk]) < ticks)
			return true;
		return false;
	}
	
	inline POINT mousePos(WORD flags = 0)
	{
		if ((flags>>14)==1)
			return m_op;
		else if ((flags>>14)==2)
			return CPoint(m_cp - m_op);

		return m_cp;
	}
};
