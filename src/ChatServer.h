/*
*
* Test implementation of the server class.
* Implements the handlers to form a chat server
*
*/

#pragma once

#include "IServer.h"

// ---- Common test net values ----

#define IP_WILLIAMS "192.168.2.64"
#define IP_QUEENS "10.217.220.39"
#define PORT 51115

// --------------------------------


class ChatServer : public IServer
{
public:

	ChatServer();

	~ChatServer();


	// ---- Handlers ----
	static void MessageReceived(CTcpListener* listener, int client, std::string clientName, std::string msg);

	static void ClientDisconnect(CTcpListener* listener, int client, std::string clientName);

	static void ClientConnect(CTcpListener* listener, int client, std::string clientName);

	static void ServerError(CTcpListener* listener, int client, int error);

	// ------------------

	// ---- Unique functionality ----
	void runChat();


	// ------------------------------

private:

	void clean();
};