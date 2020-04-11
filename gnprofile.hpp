#pragma once

#include <atlstr.h>

class GNProfile
{
public:
	inline GNProfile(void)
	: m_bProfile(true)
	, m_bUsed(false)
	{
	}

	bool			m_bProfile;
	bool			m_bUsed;
	double			s, e, t;

	inline void Begin()
	{
		m_bUsed = true;
		t = s = current();
	}

	inline double End()
	{
		if (! m_bUsed)
			return current();
		e = current();
		return e - s;
	}

	inline void SetModeProfile( bool modeProfile )
	{
		m_bProfile = modeProfile;
	}

	inline double Step(const wchar_t* msg)
	{
		if (m_bProfile )
		{
			double dx = End();
			double dt = e - t;
			if (msg)
			{
				CString msgs;
				msgs.Format(L"GNProfile [Timer]: [%f / %f] %s\r\n", dt, dx, msg);
				OutputDebugString(msgs);
			}
			t = e;
			return dt;
		}
		return 0.0;
	}

	inline void End(const wchar_t* msg )
	{
		if ( msg && m_bProfile )
		{
			CString msgs;
			msgs.Format(L"GNProfile [Timer]: [%f] %s\r\n", End(), msg);
			OutputDebugString(msgs);
		}
	}

	inline static double current()
	{
		static bool bFirst = true;
		static LARGE_INTEGER q;
		if (bFirst) {bFirst=false;QueryPerformanceFrequency(&q);}
		LARGE_INTEGER c;
		QueryPerformanceCounter(&c);
		return ((double)c.QuadPart)/q.QuadPart;
	}
};

class GNMProfile
{
protected:
	bool m_bUsed;
	double s, e;
public:
	inline GNMProfile()
	: m_bUsed(false)
	{
	}
	
	inline void Begin()
	{
		m_bUsed = true;
		s = (double)memSize();
	}
	
	inline double End()
	{
		m_bUsed = false;
		e = (double)memSize();
		return (e-s);
	}

	inline void End( wchar_t* msg )
	{
		if ( msg )
		{
			CString msgs;
			double ee = End();
			msgs.Format(L"GNMProfile [Memory]: [%0.0f / %0.2f] %s\r\n", ee, e/1024.0f, msg);
			OutputDebugString(msgs);
		}
	}
	
	static inline size_t memSize()
	{
		size_t x = 0;
		
		MEMORY_BASIC_INFORMATION memInfo;
		
		size_t wAddr = 0;
		memset(&memInfo, 0, sizeof(memInfo));
		
		while (wAddr < (size_t)0x80000000 && VirtualQuery((LPCVOID)wAddr, &memInfo, sizeof(memInfo)))
		{
			if (memInfo.State == MEM_COMMIT && memInfo.Type == MEM_PRIVATE)
				x += memInfo.RegionSize;
			wAddr += memInfo.RegionSize;
		}
		return x;
	}
};

class GNTimeout
{
protected:
	double	m_s_time;
public:
	inline GNTimeout() : m_s_time(0.0) {}
	inline bool isTimeout(double _sec)
	{
		double e_time = GNProfile::current();
		if ((e_time - m_s_time) > _sec)
		{
			m_s_time = e_time;
			return true;
		}
		return false;
	}
};
