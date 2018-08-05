#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <array>
#include <functional>

namespace tcpserv {
	class Connection;

	using THandler = std::function<int(void *)>;
	using SConnection = std::shared_ptr<Connection>;
	using TBytes = std::vector<uint8_t>;

	class Connection {
	public:
		Connection(int fd);

		inline int getSocketDescriptor() const;

		inline const TBytes &getBuffer() const;

		inline const TBytes &getUnwritten() const;

		void appendBuffer(const TBytes &extra);

		void appendUnwritten(const TBytes &extra);

		void resetBuffer();

		void resetUnwritten();

		inline int getTimerFd() const;

		inline void setTimerFd(int timerFd);

		inline const std::array<int, 10> &getStagedData() const;

		inline void setStagedData(std::array<int, 10> map);

		inline void setStagedHandler(THandler handler);

		inline void callStage(void *data);

	private:
		int m_Fd;
		TBytes m_Buff;
		TBytes m_Unwritten;

		int m_Timer_fd;
		std::array<int, 10> m_StagedData;
		THandler m_StagedHandler;
	};

	inline int Connection::getSocketDescriptor() const {
		return m_Fd;
	}

	inline const TBytes &Connection::getBuffer() const {
		return m_Buff;
	}

	inline const TBytes &Connection::getUnwritten() const {
		return m_Unwritten;
	}

	inline int Connection::getTimerFd() const {
		return m_Timer_fd;
	}

	inline void Connection::setTimerFd(int timerFd)	{
		m_Timer_fd = timerFd;
	}

	inline const std::array<int, 10> &Connection::getStagedData() const	{
		return m_StagedData;
	}

	inline void Connection::setStagedData(std::array<int, 10> map){
		m_StagedData = map;
	}

	inline void Connection::setStagedHandler(THandler handler) {
		m_StagedHandler = handler;
	}

	inline void Connection::callStage(void *data) {
		m_StagedHandler(data);
	}
}
