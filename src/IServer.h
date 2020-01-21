/*
*
* Handler method signatures for the CTcpListener class.  Helps structure handler implementation.
* Contains necessary stuff for any server implementation
*
*/

#pragma once

#define DEFAULT_PORT 51115

#include "TcpServer.h"

// ---- handler definitions ----
//static void MessageReceived(CTcpListener* listener, int client, std::string clientName, std::string msg);

//static void ClientDisconnect(CTcpListener* listener, int client, std::string clientName);

//static void ClientConnect(CTcpListener* listener, int client, std::string clientName);

//static void ServerError(CTcpListener* listener, int client, int error);
// -----------------------------

class IServer
{
public:

	void setPort(int port) { m_port = port; }

	void setIpAddress(std::string ip) { m_ipAddress = ip; }

	void makeServer() { server = new CTcpListener(m_ipAddress, m_port); }

	virtual void clean() = 0;

protected:

	
	int m_port = DEFAULT_PORT;
	std::string m_ipAddress;

	CTcpListener* server;
};