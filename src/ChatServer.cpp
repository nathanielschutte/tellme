/*
*
* Test implementation of the server class.
* Implements the handlers to form a chat server
*
*/

#include <iostream>
#include "IServer.h"

// ---- Common test net values ----

#define IP_WILLIAMS "192.168.2.64"
#define IP_QUEENS "10.217.220.39"
#define PORT 51111

// --------------------------------
using namespace std;

ChatServer::ChatServer()
{

}

ChatServer::~ChatServer()
{

}

void ChatServer::runChat()
{
	CTcpListener server(IP_WILLIAMS, PORT, MessageReceived);

	if (server.init())
	{
		server.addClientConnectHandler(ClientConnect);
		server.addClientDisconnectHandler(ClientDisconnect);
		server.addErrorHandler(ServerError);

		server.run();
	}

	server.cleanup();
}


void ChatServer::MessageReceived(CTcpListener* listener, int client, string clientName, string msg)
{
	// display client message to server
	cout << "[" << clientName << "] " << msg << endl;

	// echo
	listener->sendMsg(client, msg);
}


void ChatServer::ClientConnect(CTcpListener* listener, int client, string clientName)
{
	cout << clientName << " has connected" << endl;
}


void ChatServer::ClientDisconnect(CTcpListener* listener, int client, string clientName)
{
	cout << clientName << " has disconnected" << endl;
}

void ChatServer::ServerError(CTcpListener* listener, int client, int error)
{
	cerr << "SERVER ERROR: " << listener->errorString(error) << endl;
	exit(error);
}