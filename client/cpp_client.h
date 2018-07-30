#pragma once

#include <netinet/in.h>
#include <string>

class Client
{
public:

	/** Heavy stuff - TODO separate establishing connection, creation of the socket etc. from the constructor...
	 *
	 */
	Client(std::string address, int port);

	/** Controlling method
	 *
	 * \param data User can provide whatever input, but only defined commands are evaluated
	 * - such as "cpu" and "mem". If one of the defined command is sent, the return value is true; in
	 * other case, return value is false.
	 *
	 * \return True if the command was recognized, false otherwise.
	 */
	bool command(std::string data);

	/** Closes the connection and bails out.
	 *
	 */
	void exit();


	/** Reading bytes from the socket, the client is bind to.
	 *
	 */
	std::string receive();

protected:

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
