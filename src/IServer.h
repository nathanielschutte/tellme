#pragma once

#include "TcpServer.h"

class ChatServer
{
public:

	void runChat();

	ChatServer();

	~ChatServer();

	void static MessageReceived(CTcpListener* listener, int client, std::string clientName, std::string msg);

	void static ClientDisconnect(CTcpListener* listener, int client, std::string clientName);

	void static ClientConnect(CTcpListener* listener, int client, std::string clientName);

	void static ServerError(CTcpListener* listener, int client, int error);
};

