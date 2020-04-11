#pragma once

#include <map>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <winsock2.h>
#include "gnthread.hpp"
#include "gnnetutils.hpp"

#pragma comment(lib, "ws2_32.lib")

class GNNetStarter
{
public:
	inline GNNetStarter()
	{
		WSADATA ws;
		WSAStartup(MAKEWORD(2,2), &ws);
	}
	inline virtual ~GNNetStarter()
	{
		WSACleanup();
	}
};

class GNNetSession;
class GNPacket
: public OVERLAPPED
{
public:
	inline GNPacket(): m_sess(0), m_size(0) {}
	inline void Reset()
	{
		OVERLAPPED::hEvent = NULL;
		OVERLAPPED::Pointer = NULL;
		OVERLAPPED::Internal = OVERLAPPED::InternalHigh = 0L;
		OVERLAPPED::Offset  = OVERLAPPED::OffsetHigh = 0L;
		m_size = 0;
	}
	GNNetSession*	m_sess;
	DWORD			m_size;
	std::string		m_data;
};

class GNNetSession
{
public:
	SOCKET			m_sock;
	SOCKADDR_IN		m_addr;
	std::string		m_recvs;

	GNSectionLocker	m_locker;
	GNSectionLocker	m_send_locker;
	GNPacket		m_recv1;
	GNPacket		m_recv2;

	GNPacket		m_send1;
	GNPacket		m_send2;
	GNPacket*		m_send0;
	bool			m_sends;
	DWORD			m_endtm;
	LPARAM			m_context;

public:
//생성자 초기화
	inline GNNetSession() 
	: m_send0(&m_send1)
	, m_sends(false)
	, m_sock(INVALID_SOCKET)
	, m_endtm(0)
	, m_context(0)
	{
		m_recv1.Reset();
		m_recv1.m_sess = this;
		m_recv2.Reset();
		m_recv2.m_sess = this;
		m_send1.Reset();
		m_send1.m_sess = this;
		m_send2.Reset();
		m_send2.m_sess = this;
	}

	inline bool write(const std::string& data)
	{
		GNAutoLocker locker(m_send_locker);
		if (m_send0->m_data.size() > 64 * 1024 * 1024)
			return false;
		m_send0->m_data.append(data);
		if (!m_sends)
		{
			DWORD dwTrans;

			m_sends = true;
			GNPacket* sends = m_send0;
			m_send0 = (m_send0 == &m_send1) ? &m_send2 : &m_send1;
			sends->Reset();
			if (!WriteFile((HANDLE)m_sock, (LPCVOID)sends->m_data.c_str(), (DWORD)sends->m_data.size(), &dwTrans, sends))
			{
				if (GetLastError() != ERROR_IO_PENDING)
				{
					::closesocket(m_sock);
					return false;
				}
			}
		}
		return true;
	}
};

class GNNetBase;
class GNNetListener
{
public:
	virtual const std::string getName() = 0;
	virtual bool onConnect(GNNetSession& session) = 0;
	virtual bool onData(GNNetSession& session, GNPacket& packet) = 0;
	virtual bool onError(GNNetSession& session) = 0;
	virtual bool onClose(GNNetSession& session) = 0;
	virtual bool onDestroy(GNNetSession& session) = 0;
	virtual bool onListenerAdded(GNNetBase& netbase) = 0;
	virtual bool onListenerRemoved(GNNetBase& netbase) = 0;
	virtual bool onCallback(GNNetSession& session, const std::string& cmd="", WPARAM wParam = 0, LPARAM lParam = 0) = 0;
};

class GNNetListenerImpl
: public GNNetListener
{
public:
	virtual const std::string getName() = 0;
	inline virtual bool onConnect(GNNetSession& session) { return false; }
	inline virtual bool onData(GNNetSession& session, GNPacket& packet) { return false; }
	inline virtual bool onError(GNNetSession& session) { return false; }
	inline virtual bool onClose(GNNetSession& session) { return false; }
	inline virtual bool onDestroy(GNNetSession& session) { return false; }
	inline virtual bool onListenerAdded(GNNetBase& netbase) { return true; }
	inline virtual bool onListenerRemoved(GNNetBase& netbase) { return true; }
	inline virtual bool onCallback(GNNetSession& session, const std::string& cmd, WPARAM wParam, LPARAM lParam) { return false; }
};

class GNNetBase
: public GNNetStarter
, public GNNetUtils
{
protected:
	LONG						m_refc;
	HANDLE						m_hIOCP;
	SOCKET						m_sock;
	SOCKADDR_IN					m_addr;
	bool						m_bStop;
	GNSectionLocker				m_locker;

	std::list<GNNetSession*>	m_closes;
	std::list<GNNetSession*>	m_sessions;
	std::list<GNNetListener*>	m_listeners;
	std::vector<HANDLE>			m_threads;

	virtual void connector() = 0;

	inline void worker()
	{
		DWORD		dwTrans;
		ULONG_PTR	pKey;
		GNPacket	*pOV;

		while (! isStop())
		{
			if (GetQueuedCompletionStatus(m_hIOCP, &dwTrans, &pKey, (LPOVERLAPPED*)&pOV, 32))
			{
				if (dwTrans == 0 && pKey == 0 && pOV == 0)
					return;

				if (pOV)
				{
					GNNetSession* sess = pOV->m_sess;
					if (dwTrans == 0)
					{
						if ((pOV == &sess->m_recv1) || (pOV == &sess->m_recv2))
							OnClose(*pOV->m_sess);
					}
					else
					if (pOV == &sess->m_recv1)
					{
						bool error = false;
						GNAutoLocker locker(sess->m_locker);
						sess->m_recv1.m_size = dwTrans;
						sess->m_recv2.Reset();
						if (! ReadFile((HANDLE)sess->m_sock, (LPVOID)sess->m_recv2.m_data.c_str(), (DWORD)sess->m_recv2.m_data.size(), &dwTrans, &sess->m_recv2))
						{
							if (GetLastError() != ERROR_IO_PENDING)
								error = true;
						}
						OnData(*sess, sess->m_recv1);
						if (error)
							OnClose(*sess);

					}
					else
					if (pOV == &sess->m_recv2)
					{
						bool error = false;
						GNAutoLocker locker(sess->m_locker);
						sess->m_recv2.m_size = dwTrans;
						sess->m_recv1.Reset();
						if (! ReadFile((HANDLE)sess->m_sock, (LPVOID)sess->m_recv1.m_data.c_str(), (DWORD)sess->m_recv1.m_data.size(), &dwTrans, &sess->m_recv1))
						{
							if (GetLastError() != ERROR_IO_PENDING)
								error = true;
						}
						OnData(*sess, sess->m_recv2);
						if (error)
							OnClose(*sess);
					}
					else
					if (pOV == &sess->m_send1)
					{
						GNAutoLocker locker(sess->m_send_locker);
						sess->m_send1.m_data.clear();
						if (! sess->m_send2.m_data.empty())
						{
							sess->m_send0 = &sess->m_send1;
							sess->m_send2.Reset();
							if (! WriteFile((HANDLE)sess->m_sock, (LPVOID)sess->m_send2.m_data.c_str(), (DWORD)sess->m_send2.m_data.size(), &dwTrans, &sess->m_send2))
							{
								if (GetLastError() != ERROR_IO_PENDING)
								{
									::closesocket(sess->m_sock);
									sess->m_sock = INVALID_SOCKET;
								}
							}
						}
						else
						{
							sess->m_sends = false;
						}
					}
					else
					if (pOV == &sess->m_send2)
					{
						GNAutoLocker locker(sess->m_send_locker);
						sess->m_send2.m_data.clear();
						if (! sess->m_send1.m_data.empty())
						{
							sess->m_send0 = &sess->m_send2;
							sess->m_send1.Reset();
							if (! WriteFile((HANDLE)sess->m_sock, (LPVOID)sess->m_send1.m_data.c_str(), (DWORD)sess->m_send1.m_data.size(), &dwTrans, &sess->m_send1))
							{
								if (GetLastError() != ERROR_IO_PENDING)
								{
									::closesocket(sess->m_sock);
									sess->m_sock = INVALID_SOCKET;
								}
							}
						}
						else
						{
							sess->m_sends = false;
						}
					}
				}
				else
				{
					break;
				}
			}
			else
			if (pOV)
			{
				GNNetSession* sess = pOV->m_sess;
				if ((pOV == &sess->m_recv1) || (pOV == &sess->m_recv2))
				{
					OnError(*sess);
					OnClose(*sess);
				}
			}
		}
	}	

	inline void closing()
	{
		std::list<GNNetSession*> closes;
		std::list<GNNetSession*> saves;
		
		size_t proc_size = m_closes.size() >> 3;
		if (proc_size < 8) proc_size = 8;

		while (! m_closes.empty() && (closes.size() < proc_size))
		{
			GNAutoLocker locker(m_locker);
			closes.push_back(m_closes.front());
			m_closes.pop_front();
		}

		std::list<GNNetSession*>::iterator cp;
		
		for (cp = closes.begin(); cp != closes.end(); cp ++)
		{
			GNNetSession* sess = *cp;
			if (GetTickCount() > sess->m_endtm + 15000)
			{
				OnDestroy(*sess);
				delete sess;
				continue;
			}
			saves.push_back(sess);
		}

		if (! saves.empty())
		{
			GNAutoLocker locker(m_locker);
			for (cp = saves.begin(); cp != saves.end(); cp ++)
				m_closes.push_back(*cp);
		}
	}
	
	inline bool AddConnect(SOCKET sock, SOCKADDR_IN& addr)
	{
		int opt = 1;
		::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&opt, sizeof(opt));

		GNNetSession* psess = new GNNetSession();
		psess->m_addr = addr;
		psess->m_sock = sock;
		psess->m_recv1.m_data.resize(8*1024L);
		psess->m_recv2.m_data.resize(8*1024L);

		DWORD dwTrans;
		CreateIoCompletionPort((HANDLE)psess->m_sock, m_hIOCP, (ULONG_PTR)psess, 0);

		if (psess)
			OnConnect(*psess);

		if (!ReadFile((HANDLE)psess->m_sock, (LPVOID)psess->m_recv1.m_data.c_str(), (DWORD)psess->m_recv1.m_data.size(), &dwTrans, &psess->m_recv1))
		{
			//ERROR_IO_PENDING >>> 997L
			if (GetLastError() != ERROR_IO_PENDING)
			{
				::PostQueuedCompletionStatus(m_hIOCP, 0, (ULONG_PTR)psess, &psess->m_recv1);
				return false;
			}
		}

		return true;
	}

	inline void OnConnect(GNNetSession& sess)
	{
		for (std::list<GNNetListener*>::iterator cp = m_listeners.begin(); cp != m_listeners.end(); cp ++)
			(*cp)->onConnect(sess);
		GNAutoLocker locker(m_locker);
		m_sessions.push_back(&sess);
	}	

	inline void OnClose(GNNetSession& sess)
	{
		for (std::list<GNNetListener*>::iterator cp = m_listeners.begin(); cp != m_listeners.end(); cp ++)
			(*cp)->onClose(sess);
		if (sess.m_sock != INVALID_SOCKET)
			::closesocket(sess.m_sock);
		sess.m_sock = INVALID_SOCKET;
		sess.m_endtm = GetTickCount();
		GNAutoLocker locker(m_locker);
		std::list<GNNetSession*>::iterator fp = std::find(m_sessions.begin(), m_sessions.end(), &sess);
		if (fp != m_sessions.end())
			m_sessions.erase(fp);
		m_closes.push_back(&sess);
	}
	
	inline void OnError(GNNetSession& sess)
	{
		for (std::list<GNNetListener*>::iterator cp = m_listeners.begin(); cp != m_listeners.end(); cp ++)
			(*cp)->onError(sess);
	}	

	inline void OnDestroy(GNNetSession& sess)
	{
		for (std::list<GNNetListener*>::iterator cp = m_listeners.begin(); cp != m_listeners.end(); cp ++)
			(*cp)->onDestroy(sess);
		GNAutoLocker locker(m_locker);
		std::list<GNNetSession*>::iterator fp = std::find(m_closes.begin(), m_closes.end(), &sess);
		if (fp != m_closes.end())
			m_closes.erase(fp);
	}
	
	inline void OnData(GNNetSession& sess, GNPacket& data)
	{
		for (std::list<GNNetListener*>::iterator cp = m_listeners.begin(); cp != m_listeners.end(); cp ++)
			(*cp)->onData(sess, data);
	}	

	inline static UINT CALLBACK _connector(GNNetBase& This)
	{
		InterlockedIncrement(&This.m_refc);
		try
		{
			This.connector();
		}
		catch(...)
		{
			OutputDebugStringW(L"Error on connector\n");
		}
		InterlockedDecrement(&This.m_refc);
		return 0;
	}

	inline static UINT CALLBACK _worker(GNNetBase& This)
	{
		InterlockedIncrement(&This.m_refc);
		try
		{
			This.worker();
		}
		catch(...)
		{
			OutputDebugStringW(L"Error on worker\n");
		}
		InterlockedDecrement(&This.m_refc);
		return 0;
	}
	

public:
	inline GNNetBase(void)
	: m_bStop(true)
	, m_refc(0L)
	, m_sock(INVALID_SOCKET)
	{
	}
	
	inline virtual ~GNNetBase(void)
	{
		destroy();
	}
	
	inline void destroy()
	{
		GNAutoLocker locker(m_locker);
		std::list<GNNetSession*>::iterator cp;
		std::list<GNNetSession*> sesss;

		sesss = m_sessions;
		m_sessions.clear();

		for (cp = sesss.begin(); cp != sesss.end(); cp ++)
			OnClose(*(*cp));

		sesss = m_closes;
		m_closes.clear();

		for (cp = sesss.begin(); cp != sesss.end(); cp ++)
		{
			GNNetSession* sess = *cp;
			if (sess)
			{
				OnDestroy(*sess);
				delete sess;
			}
		}
	}

	inline bool addListener(GNNetListener* _listener)
	{
		GNAutoLocker locker(m_locker);
		std::list<GNNetListener*>::iterator fp = std::find(m_listeners.begin(), m_listeners.end(), _listener);
		if (fp == m_listeners.end())
		{
			m_listeners.push_back(_listener);
			_listener->onListenerAdded(*this);
			return true;
		}
		return false;
	}
	
	inline bool removeListener(GNNetListener* _listener)
	{
		GNAutoLocker locker(m_locker);
		std::list<GNNetListener*>::iterator fp = std::find(m_listeners.begin(), m_listeners.end(), _listener);
		if (fp != m_listeners.end())
		{
			m_listeners.erase(fp);
			_listener->onListenerRemoved(*this);
			return true;
		}
		return false;
	}

	inline virtual bool isStop()
	{
		return m_bStop;
	}

	inline virtual void SetStop(bool _bStop)
	{
		m_bStop = _bStop;
	}
	
	inline virtual bool startup(SOCKADDR_IN& addr)
	{
		if (! isStop())
			return false;
		SetStop(false);
		InterlockedIncrement(&m_refc);
		m_addr = addr;
		m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
		m_threads.reserve(5);
		m_threads.push_back((HANDLE)_beginthreadex(0, 0, (UINT (CALLBACK*)(LPVOID))_connector, this, 0, 0));
		for(int i = 0; i < 4; i++)
			m_threads.push_back((HANDLE)_beginthreadex(0, 0, (UINT (CALLBACK*)(LPVOID))_worker, this, 0, 0));
		return true;
	}
	
	inline bool cleanup()
	{
		if (isStop())
			return false;
		SetStop(true);
		InterlockedDecrement(&m_refc);
		::closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		for(size_t i=0; i < m_threads.size(); i++)
			PostQueuedCompletionStatus(m_hIOCP, 0, 0, 0);

		while (WaitForMultipleObjects((DWORD)m_threads.size(), &m_threads[0], TRUE, 1000) == WAIT_TIMEOUT)
			m_hIOCP = m_hIOCP;

		for(size_t i=0; i < m_threads.size(); i++)
			CloseHandle(m_threads[i]);
		m_threads.clear();
		CloseHandle(m_hIOCP);
		m_hIOCP = 0;
		destroy();
		return true;
	}

	inline SOCKET try_connect(SOCKADDR_IN& addr, int mills = 0)
	{
		SOCKET sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		if (sock == INVALID_SOCKET)
			return sock;

		if (GNNetUtils::connect_timeout(sock, addr, mills) == SOCKET_ERROR) {
			closesocket(sock);
			sock = INVALID_SOCKET;
		}
		return sock;
	}
};

class GNClient
: public GNNetBase
{
protected:
	inline virtual void connector()
	{
		while (! isStop())
		{
			closing();
			::Sleep(16);
		}
	}	
	
public:
	inline bool connect(SOCKADDR_IN& addr, int mills = 0)
	{
		SOCKET sock = try_connect(addr, mills);
		if (sock == INVALID_SOCKET)
			return false;
		return AddConnect(sock, addr);
	}	
};

class GNServer
: public GNNetBase
{
protected:
	inline virtual void connector()
	{
		m_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		if (m_sock == INVALID_SOCKET)
		{
			::MessageBoxW(NULL, L"Can not create new socket !", L"GNServer", MB_ICONSTOP | MB_OK);
			return;
		}
		
		if (bind(m_sock, (SOCKADDR *)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
		{
			::MessageBoxW(NULL, L"Can not bind socket. Address already used !", L"GNServer", MB_ICONSTOP | MB_OK);
			return;
		}
		
		if (listen(m_sock, 5) == SOCKET_ERROR)
		{
			::MessageBoxW(NULL, L"Can not listen socket. Server failed !", L"GNServer", MB_ICONSTOP | MB_OK);
			return;
		}
		
		while (! isStop())
		{
			closing();

			SOCKADDR_IN aaddr;
			int alen = sizeof(aaddr);
			SOCKET asock = accept(m_sock, (sockaddr*)&aaddr, &alen);
			if (asock == INVALID_SOCKET)
			{
				::Sleep(1);
				continue;
			}
			AddConnect(asock, aaddr);

			::Sleep(50);
		}
	}	

	inline bool broadcast(const std::string& data, GNNetSession* psess = NULL)
	{
		if (psess)
			return psess->write(data);

		std::list<GNNetSession*> sessions;
		m_locker.lock();
		sessions = m_sessions;
		m_locker.unlock();

		for (std::list<GNNetSession*>::iterator cp = sessions.begin(); cp != sessions.end(); cp ++)
			(*cp)->write(data);
		
		return true;
	}
};

/**********************************************

#include "gnsoftech/gnnet.hpp"
#include "gnsoftech/gnprotocol.hpp"

class KYSERVER
: public GNServer
, public GNNetListenerImpl
, public GNTextProtocol
{
public:
	KYSERVER()
	{
		addListener(this);
	}

	~KYSERVER()
	{
		removeListener(this);
		cleanup();
	}

	const std::string getName() { return "KYSERVER"; }

	virtual bool onData(GNNetSession& sess, GNPacket& pck)
	{
		GNTextProtocol::appendData(sess, pck.m_data.c_str(). pck.m_size);

		std::string cmd, dat;
		while (recvPacket(sess, cmd, dat))
		{
			if (cmd == "aa")
			{
			}
			else if (cmd=="bb")
			{
				sendPacket(sess, "cc", "abcd");
			}
		}

		return true;
	}
};

***************************************************/