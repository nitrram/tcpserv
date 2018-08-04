#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace tcpserv
{
	class Connection;

	using SConnection = std::shared_ptr<Connection>;
	using TBytes = std::vector<uint8_t>;

	class Connection
	{
	public:
		Connection(int fd);

		inline int getSocketDescriptor() const;

		inline const TBytes &getBuffer() const;

		inline const TBytes &getUnwritten() const;

		void appendBuffer(const TBytes &extra);

		void appendUnwritten(const TBytes &extra);


	private:
		int m_Fd;
		TBytes m_Buff;
		TBytes m_Unwritten;
	};

	inline int Connection::getSocketDescriptor() const
	{
		return m_Fd;
	}

	inline const TBytes &Connection::getBuffer() const
	{
		return m_Buff;
	}

	inline const TBytes &Connection::getUnwritten() const
	{
		return m_Unwritten;
	}

}
