#pragma once

#include "gnnet.hpp"

class GNProtocol
{
protected:
	static bool appendData(GNNetSession& sess, const char *data, size_t size)
	{
		GNAutoLocker locker(sess.m_locker);
		sess.m_recvs.append(data, size);
		return true;
	}
};

class GNTextProtocol
: public GNProtocol
{
protected:
	static bool recvPacket(GNNetSession& sess, std::string& cmd, std::string& data)
	{
		size_t st = sess.m_recvs.find("syn::");
		if (st == std::string::npos)
			return false;

		size_t cm = sess.m_recvs.find(".", st+5);

		cmd = sess.m_recvs.substr(st+5, cm-st-5);
		if (cmd.empty() || cmd.size() > 16)
		{
			// has problem. clip syn
			sess.m_recvs.erase(0, st+5);
			return false;
		}

		size_t sz = sess.m_recvs.find(".", cm+1);
		std::string size = sess.m_recvs.substr(cm+1, sz-cm-1);
		size_t dt = atoi(size.c_str());
		size_t ed = sess.m_recvs.find("::end.", sz+1+dt);
		if (ed == std::string::npos)
			return false;		

		if (dt)
			data = sess.m_recvs.substr(sz+1, dt);

		sess.m_recvs.erase(0, ed+6);
		return true;
	}

	static bool sendPacket(GNNetSession& sess, const std::string& cmd, const std::string& data)
	{
		char buff[24]; _itoa_s<24>((int)data.size(), buff, 10);
		std::string s; s.append("syn::").append(cmd).append(".").append(buff).append(".").append(data).append("::end.");
		return sess.write(s);
	}

public:
	static bool makePacket(const std::string& cmd, const std::string& dat, std::string& pck)
	{
		char buff[24]; _itoa_s<24>((int)dat.size(), buff, 10);
		pck.clear();
		pck.append("syn::").append(cmd).append(".").append(buff).append(".").append(dat).append("::end.");
		return true;
	}
};

class GNBinaryProtocol
: public GNProtocol
{
public:
#pragma pack(push, 1)
	struct PCK_HEADER
	{
		unsigned int opcode;
		unsigned int length;
	};
#pragma pack(pop)
protected:
	static bool recvPacket(GNNetSession& sess, PCK_HEADER*& _hdr, std::string& data)
	{
		if (sess.m_recvs.size() < sizeof(PCK_HEADER))
		{
			_hdr = NULL;
			return false;
		}

		_hdr = (PCK_HEADER*)sess.m_recvs.c_str();
		if (sess.m_recvs.size() < (UINT)_hdr->length)
		{
			return false;
		}

		data.clear();
		data.reserve(_hdr->length);
		data.append(sess.m_recvs.c_str(), _hdr->length);
		sess.m_recvs.erase(0, _hdr->length);
		_hdr = (PCK_HEADER*)data.c_str();
		return true;
	}

	static bool sendPacket(GNNetSession& sess, PCK_HEADER* _hdr)
	{
		std::string s;
		s.append((const char *)_hdr, _hdr->length);
		return sess.write(s);
	}

public:
	static bool makePacket(const std::string& cmd, const std::string& dat, std::string& pck)
	{
		pck.clear();
		pck.resize(sizeof(PCK_HEADER) + dat.size());
		PCK_HEADER *_hdr = (PCK_HEADER*)pck.c_str();
		memcpy((void*)&_hdr->opcode, cmd.c_str(), sizeof(_hdr->opcode));
		_hdr->length = (int)pck.size();
		memcpy((void*)(pck.c_str()+sizeof(PCK_HEADER)), dat.c_str(), dat.size());
		return true;
	}

	static bool makePacket(unsigned int opcode, const std::string& dat, std::string& pck)
	{
		pck.clear();
		pck.resize(sizeof(PCK_HEADER) + dat.size());
		PCK_HEADER *_hdr = (PCK_HEADER*)pck.c_str();
		_hdr->opcode = opcode;
		_hdr->length = (unsigned int)pck.size();
		memcpy((void*)(pck.c_str()+sizeof(PCK_HEADER)), dat.c_str(), dat.size());
		return true;
	}
};
