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
#define IP_LEHIGH "128.180.235.130"
#define PORT 51115

const std::string joinMsg = "======= You joined the server ========\r\nPreface commands with forward slash \'/\'.\r\nUse \'/help\' for commands list\r\n======================================\r\n";

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

	static void ServerError(CTcpListener* listener, int client, int error, bool fatal);

	// ------------------


	// ---- Unique functionality ----
	void runChat();

	// ------------------------------

private:

	void clean();

	static void processClientCommand(CTcpListener* listener, int client, std::string cmd);

	static void renameClient(CTcpListener* listener, int client, std::string name);

	static void listClients(CTcpListener* listener, int client);
};