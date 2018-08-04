#include "cpp_connection.h"

namespace tcpserv
{
	Connection::Connection(int fd) :
		m_Fd(fd)
	{
	}

	void Connection::appendBuffer(const TBytes &extra)
	{
		m_Buff.insert(m_Buff.end(), extra.begin(), extra.end());
	}

	void Connection::appendUnwritten(const TBytes &extra)
	{
		m_Unwritten.insert(m_Buff.end(), extra.begin(), extra.end());
	}
}
