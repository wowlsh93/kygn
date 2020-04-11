// gntrayicon header

#pragma once
#ifndef _GNTRAYICON_
#define _GNTRAYICON_

#include <string>
#include <windows.h>
#include <shellapi.h>
class GNTrayIcon
{
private:
	std::wstring	m_tooltip;
	HICON			m_hIcon;
	HWND			m_hWnd;
	UINT			m_uID;
	UINT			m_msg;
	UINT			m_rwm;
public:
	inline GNTrayIcon(UINT _uID, const std::wstring& _tooltip = L"GNTrayIcon")
	: m_hWnd(NULL)
	, m_uID(_uID)
	, m_tooltip(_tooltip)
	, m_hIcon(NULL)
	, m_msg(WM_USER + m_uID)
	, m_rwm(::RegisterWindowMessage(_T("TaskbarCreated")))
	{
	}

	inline ~GNTrayIcon()
	{
		if (m_hIcon)
			DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}
	
	inline void setHWnd(HWND _hWnd)
	{
		m_hWnd = _hWnd;
	}
	
	inline void setToolTip(const std::wstring& _tooltip)
	{
		m_tooltip = _tooltip;
	}
	
	inline void setIcon(HICON _hIcon, bool _bCopy = true)
	{
		if (_bCopy)
			m_hIcon = CopyIcon(_hIcon);
		else
			m_hIcon = _hIcon;
	}

	inline bool add()
	{
		NOTIFYICONDATA nid = {};

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = m_uID;
		nid.uVersion = NOTIFYICON_VERSION;
		nid.uCallbackMessage = m_msg;
		nid.hIcon = m_hIcon ? m_hIcon : LoadIcon(NULL, IDI_APPLICATION);
		wcscpy_s(nid.szTip, m_tooltip.c_str());
		nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

		return Shell_NotifyIcon(NIM_ADD, &nid) != 0;
	}
	
	inline bool update()
	{
		NOTIFYICONDATA nid = {};

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = m_uID;
		nid.hIcon = m_hIcon ? m_hIcon : LoadIcon(NULL, IDI_APPLICATION);
		wcscpy_s(nid.szTip, m_tooltip.c_str());
		nid.uFlags = NIF_ICON | NIF_TIP;
		return Shell_NotifyIcon(NIM_MODIFY, &nid) != 0;
	}
	
	
	inline bool remove()
	{
		NOTIFYICONDATA nid = {};

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = m_hWnd;
		nid.uID = m_uID;
		return Shell_NotifyIcon(NIM_DELETE, &nid) != 0;
	}
	
	inline bool isNotifyMessage(UINT message)
	{
		return (message == m_msg);
	}
	
	inline bool OnMsg(UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (isNotifyMessage(message))
		{
			return true;
		}
		else if (message == m_rwm)
		{
			add();
		}
		return false;
	}
};

/*	Usage of Tray icon

CMyDLG::CMyDLG(CWnd* pParent)
: CDialog(CMyDLG::IDD, pParent)
, m_tray(10, L"MYDLG Tooltip")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_tray.setIcon(m_hIcon);
}

void CMyDLG::OnDestroy()
{
	m_tray.remove();
	CDialog::OnDestroy();
}

BOOL CMyDLG::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_tray.setHWnd(GetSafeHwnd());
	m_tray.setIcon(m_hIcon);

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	return TRUE;
}

BOOL CMyDLG::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (m_tray.OnMsg(message, wParam, lParam))
	{
		switch (lParam)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_LBUTTONDBLCLK:
			break;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_RBUTTONDBLCLK:
			break;
		}
	}
	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

*/

#endif
