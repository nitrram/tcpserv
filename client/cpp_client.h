#pragma once

class Client
{
public:
	Client(std::string address, int port);

	bool command(std::string data);

	std::string receive();

	void exit();

	inline bool isWorking() const;

private:
	int m_Fd;
	std::string m_Address;
	int m_Port;
	struct sockaddr_in m_Server;
	bool m_Working;
};


inline bool Client::isWorking() const
{
	return m_Working;
}
