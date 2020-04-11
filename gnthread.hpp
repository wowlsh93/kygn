// gnthread header
#pragma once
#ifndef _GNTHREAD_
#define _GNTHREAD_

#include <vector>
#include <windows.h>
#include <process.h>

#ifndef SAFE_FREE
#define SAFE_FREE(X)  { if ((X)) { ::free((X)); (X)=NULL; } }
#endif
#ifndef SAFE_DELETE
#define SAFE_DELETE(X)  { if ((X)) { delete (X); (X)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(X)  { if ((X)) { delete[] (X); (X)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(X)  { if ((X)) { (X)->Release(); } }
#endif

#define	STATIC_INSTANCE(XX)		\
public:	\
	static XX& get() { static XX s_instance; return s_instance; }

class GNLocker
{
public:
	virtual void lock() = 0;
	virtual void unlock() = 0;
};

class GNAutoLocker
{
private:
	GNLocker& m_locker;

public:
	inline GNAutoLocker(GNLocker& locker): m_locker(locker) { m_locker.lock(); }
	inline ~GNAutoLocker() { m_locker.unlock(); }
};

class GNSectionLocker
: public GNLocker
{
private:
	CRITICAL_SECTION	m_cs;

public:
	inline GNSectionLocker()
	{
		InitializeCriticalSection(&m_cs);
	}

	inline ~GNSectionLocker()
	{
		DeleteCriticalSection(&m_cs);
	}

	inline virtual void lock()
	{
		EnterCriticalSection(&m_cs);
	}
	
	inline virtual void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
};

class GNThread
{
private:
	bool	m_stop;
	bool	m_self_destory;
	std::vector<HANDLE>	m_vTHREADS;

public:
	inline GNThread()
	: m_stop(true)
	, m_self_destory(false)
	{}
	inline virtual ~GNThread() {}

	inline static UINT CALLBACK _thr_main(GNThread& This)
	{
		This.main();
		if (This.m_self_destory)
			delete &This;
		return 0;
	}
	
	virtual void main() = 0;

	inline bool isStop()
	{
		return m_stop;
	}
	
	inline void setSelfDestroy(bool bDest)
	{
		m_self_destory = bDest;
	}
	
	inline void start(int nThreads = 1)
	{
		if (! m_stop)
			return;
		m_stop = false;

		for (int i = 0; i < nThreads; i ++)
			m_vTHREADS.push_back((HANDLE)_beginthreadex(0, 0, (UINT(CALLBACK*)(LPVOID))_thr_main, this, 0, 0));
	}

	inline void join()
	{
		if (m_vTHREADS.empty())
			return;

		while ((WaitForMultipleObjects((DWORD)m_vTHREADS.size(), &(m_vTHREADS.at(0)), TRUE, 1000) == WAIT_TIMEOUT) && !m_stop)
			::Sleep(50);

		for (size_t i = 0; i < m_vTHREADS.size(); i ++)
			CloseHandle(m_vTHREADS.at(i));

		m_vTHREADS.clear();
	}
	
	inline void killAll()
	{
		for (size_t i = 0; i < m_vTHREADS.size(); i ++)
		{
			TerminateThread(m_vTHREADS.at(i), 0xFF00);
			CloseHandle(m_vTHREADS.at(i));
		}
	}
	
	inline void stop()
	{
		if (m_stop)
			return;
		m_stop = true;
	}
};

#endif