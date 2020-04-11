#pragma once

#include <list>
#include <string>

#include "gndatalink.hpp"

typedef std::pair<std::string, GNLINKVALUES>	CACHEDATA;
typedef std::list< CACHEDATA >					CACHELIST;

class GNLRUCache
: public CACHELIST
{
protected:
	size_t m_size;
public:
	inline size_t setMaxSize(size_t _size)
	{
		m_size = _size;
		return m_size;
	}

	inline const GNLRUCache::iterator find(const std::string& key)
	{
		for (iterator cp = begin(); cp != end(); cp ++)
		{
			if (cp->first == key)
				return cp;
		}
		return end();
	}

	inline bool setData(const std::string& key, const GNLINKVALUES& value)
	{
		iterator fp = find(key);
		if (fp != end())
			erase(fp);
		push_front( std::make_pair(key, value) );
		while (size() >= m_size)
			pop_back();
		return true;
	}

	inline bool getData(const std::string& key, GNLINKVALUES& value)
	{
		iterator fp = find(key);
		if (fp == end())
			return false;
		if (fp == begin())
			return true;
		CACHEDATA& val = *fp;
		push_front(val);
		erase(fp);
		return true;
	}
};

class GNRepository
{
protected:
	GNLRUCache		m_cacheData;

public:
	inline size_t setCacheSize(size_t size)
	{
		return m_cacheData.setMaxSize(size);
	}

	inline bool setData(const std::string& key, const GNLINKVALUES& value)
	{
		return m_cacheData.setData(key, value);
	}
	
	inline bool getData(const std::string& key, GNLINKVALUES& value)
	{
		return m_cacheData.getData(key, value);
	}
	
	inline bool rmvData(const std::string& key)
	{
		GNLRUCache::iterator fp = m_cacheData.find(key);
		if (fp == m_cacheData.end())
			return false;
		return false;
	}
};
