#include "cpp_client.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <string>

Client::Client(std::string address, int port) :
	m_Fd(-1),
	m_Address(address),
	m_Port(port),
	m_Working(false)
{

	//TODO move the whole code into a separate method !keep it simple!
	m_Fd = socket(AF_INET , SOCK_STREAM , 0);
	if (m_Fd == -1)
	{
		std::cerr << "Could not create socket\n";
	}
	else
	{
		if(inet_addr(m_Address.c_str()) ==	INADDR_NONE)
		{
			struct hostent *he;
			struct in_addr **addr_list;
			if ( (he = gethostbyname( m_Address.c_str() ) ) == NULL)
			{
				std::cerr << "Failed to resolve hostname\n";
			}
			else {
				addr_list = (struct in_addr **) he->h_addr_list;
				for(int i = 0; addr_list[i] != NULL; i++)
				{
					m_Server.sin_addr = *addr_list[i];
					break;
				}
				m_Working = true;
			}
		}
		else
		{
			m_Server.sin_addr.s_addr = inet_addr( address.c_str() );
			m_Working = true;
		}

		if(m_Working)
		{
			m_Server.sin_family = AF_INET;
			m_Server.sin_port = htons( port );

			/*
			//	non-blocking
			if( (arg = fcntl(m_Fd, F_GETFL, NULL)) < 0) {
				std::cerr << "Error: " << strerror(errno) << std::endl;
				m_Working = false;
			}
			arg |= O_NONBLOCK;
			if( fcntl(soc, F_SETFL, arg) < 0) {
				std::cerr << "Error: " << strerror(errno) << std::endl;
				m_Working = false;
			}
			*/

			//TODO this must be non-blocking
			if (connect(m_Fd , (struct sockaddr *)&m_Server , sizeof(m_Server)) < 0)
			{
				/*
				if (errno == EINPROGRESS) {
					//TODO check on select(m_Fd) with timeout
					}
				*/
				std::cerr << "connect failed. Error\n";
				m_Working = false;
			}
		}
	}
}

Client::~Client()
{
	exit();
}

bool Client::command(std::string data)
{
	if(m_Working && m_Fd != -1)
	{
		if( send(m_Fd , data.c_str() , strlen( data.c_str() ) , 0) < 0)
		{
			std::cerr << "Send failed : " << data << std::endl;
			return false;
		}
	}
	else {
		return false;
	}

	return true;
}

std::string Client::receive()
{
	char buffer[1] = {};
	std::string reply;
	while (buffer[0] != '\n') {
		if( recv(m_Fd , buffer , sizeof(buffer) , 0) < 0)
		{
			std::cerr << "receive failed!\n";
			return "";
		}
		reply += buffer[0];
	}
	return reply;
}


void Client::exit()
{
	close(m_Fd);
}
