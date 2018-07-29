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

Client::Client(std::string address, int port) :
	m_Fd(-1),
	m_Address(address),
	m_Port(port),
	m_Working(false)
{

	m_Fd = socket(AF_INET , SOCK_STREAM , 0);
	if (m_Fd == -1)
	{
		std::err << "Could not create socket\n";
	}
	else
	{
		if(inet_addr(m_Address.c_str()) == -1)
		{
			struct hostent *he;
			struct in_addr **addr_list;
			if ( (he = gethostbyname( m_Address.c_str() ) ) == NULL)
			{
				perror("gethostbyname");
				std::error << "Failed to resolve hostname\n";
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
			if (connect(m_Fd , (struct sockaddr *)&m_Server , sizeof(m_Server)) < 0)
			{
				perror("connect failed. Error");
				m_Working = false;
			}
		}
	}
}

bool Client::command(std::string data)
{
	if(m_Working && m_Fd != -1)
	{
		if( send(m_Fd , data.c_str() , strlen( data.c_str() ) , 0) < 0)
		{
			std::error << "Send failed : " << data << std::endl;
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
	string reply;
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
