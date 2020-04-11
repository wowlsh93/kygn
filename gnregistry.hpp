#pragma once

#include <map>
#include <string>
#include <atlstr.h>
#include <windows.h>

#define		DELETE_MARK	(L"--DELETED--")
class GNRegistry
{
protected:
	HKEY m_hROOT;
	bool m_has_keys;
	bool m_hkcu;
	CString m_str_root;
	std::map<std::wstring, std::wstring> m_reg_map;
	bool	m_modified;

public:
	inline GNRegistry(const CString& root, bool _hkcu = true)
	: m_hROOT(NULL), m_modified(false), m_hkcu(_hkcu)
	{
		m_str_root.Append(root);
		m_str_root.Replace(_T('/'), _T('\\'));

		CString value_ex = m_str_root;
		value_ex.Replace(_T('\\'), _T('/'));

		if (m_reg_map.find((LPCWSTR)value_ex)!=m_reg_map.end())
		{
			std::wstring value = m_reg_map[(LPCWSTR)value_ex];
			m_has_keys = value == DELETE_MARK;
			return;
		}

		m_has_keys = ::RegOpenKey(getRootKey(), m_str_root, &m_hROOT) == ERROR_SUCCESS;
		::RegCloseKey(m_hROOT);
		m_hROOT = NULL;
	}

	inline ~GNRegistry()
	{
		if (m_hROOT)
		{
			RegCloseKey(m_hROOT);
			m_hROOT = NULL;
		}
	}

	inline HKEY getRootKey()
	{
		return m_hkcu ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	}

	inline CString read(const CString& path)
	{
		CString value_ex, value_name, value_item, value_path = m_str_root;
		value_path.Append(_T("/"));
		value_path.Append(path);
		value_path.Replace(_T('/'), _T('\\'));
		value_ex = value_path;
		value_ex.Replace(_T('\\'), _T('/'));

		if (m_reg_map.find((LPCWSTR)value_ex)!=m_reg_map.end())
			return m_reg_map[(LPCWSTR)value_ex].c_str();

		int npos = value_path.ReverseFind(_T('\\'));
		value_name = value_path.Mid(npos+1);
		value_path = value_path.Left(npos);

		if (::RegOpenKey(getRootKey(), value_path, &m_hROOT)==ERROR_SUCCESS)
		{
			DWORD lSize = 0, lType = 0;
			LSTATUS ls = ::RegQueryValueEx(m_hROOT, value_name, NULL, &lType, NULL, &lSize);
			ls = ::RegQueryValueEx(m_hROOT, value_name, NULL, &lType, (LPBYTE)value_item.GetBufferSetLength(lSize), &lSize);
			value_item = value_item;
		}
		else
		{
			value_item.Empty();
		}
		::RegCloseKey(m_hROOT); m_hROOT = NULL;

		m_reg_map[(LPCWSTR)value_ex] = value_item;

		return value_item;
	}

	inline CString write(const CString& path, const CString& value)
	{
		CString value_ex, value_name, value_item, value_path = m_str_root;
		value_path.Append(_T("/"));
		value_path.Append(path);
		value_path.Replace(_T('\\'), _T('/'));
		value_ex = value_path;
		value_ex.Replace(_T('\\'), _T('/'));

		m_reg_map[(LPCTSTR)value_ex] = value;
		m_modified = true;

		return value;
	}

	inline bool remove(const CString& path)
	{
		CString value_ex, value_name, value_item, value_path = m_str_root;
		value_path.Append(_T("/"));
		value_path.Append(path);
		value_ex = value_path;
		value_ex.Replace(_T('/'), _T('\\'));

		for (std::map<std::wstring, std::wstring>::iterator cp = m_reg_map.begin(); cp != m_reg_map.end();)
		{
			std::map<std::wstring, std::wstring>::iterator ep = cp ++;
			std::wstring key = ep->first;
			if (key.substr(0, value_ex.GetLength()) != (LPCWSTR)value_ex)
				continue;
			ep->second = DELETE_MARK;
		}

		m_reg_map[(LPCWSTR)value_ex] = DELETE_MARK;
		m_modified = true;

		return false;
	}

	inline bool hasKey()
	{
		return m_has_keys;
	}

	inline bool save()
	{
		for (std::map<std::wstring, std::wstring>::iterator cp = m_reg_map.begin(); cp != m_reg_map.end(); cp++)
		{
			std::wstring key = cp->first;
			std::wstring value = cp->second;

			if (value==DELETE_MARK)
			{
				reg_remove(key.c_str());
			}
			else
			{
				reg_write(key.c_str(), value.c_str());
			}
		}
		m_modified = false;

		return false;
	}
	inline bool isModified()
	{
		return m_modified;
	}

	inline CString reg_write(const CString& path, const CString& value)
	{
		CString value_name, value_item, value_path;
		value_path.Append(path);
		value_path.Replace(_T('/'), _T('\\'));

		int npos = value_path.ReverseFind(_T('\\'));
		value_name = value_path.Mid(npos+1);
		value_path = value_path.Left(npos);
		HKEY hROOT = NULL;

		if (::RegOpenKey(getRootKey(), value_path, &hROOT)!=ERROR_SUCCESS)
		{
			if (::RegCreateKey(getRootKey(), value_path, &hROOT)!=ERROR_SUCCESS)
			{
				::RegCloseKey(hROOT);
				return _T("");
			}
		}

		value_item = value;

		if (RegSetValueEx(hROOT, value_name, NULL, REG_SZ, (BYTE*)value_item.GetBuffer(), (DWORD)(value_item.GetLength()*2))!=ERROR_SUCCESS)
		value_item.Empty();

		::RegCloseKey(hROOT);
		return value;
	}

	inline bool reg_remove(const CString& path)
	{
		CString value_path;
		value_path.Append(path);
		value_path.Replace(_T('/'), _T('\\'));

		return ::RegDeleteKey(getRootKey(), value_path) == ERROR_SUCCESS;
	}
};

#undef DELETE_MARK