/*
*
* Test implementation of the server class.
* Implements the handlers to form a chat server
*
*/

#include "ChatServer.h"
#include <iostream>

using namespace std;


ChatServer::ChatServer()
{
	ChatServer::setIpAddress(IP_WILLIAMS);
	ChatServer::setPort(PORT);
	ChatServer::makeServer();
}

ChatServer::~ChatServer()
{
	ChatServer::clean();
}


void ChatServer::runChat()
{
	if (! server->init())
	{
		return;
	}

	server->addMessageReceivedHandler(MessageReceived);
	server->addClientConnectHandler(ClientConnect);
	server->addClientDisconnectHandler(ClientDisconnect);
	server->addErrorHandler(ServerError);

	server->run();

	server->cleanup();
}

void ChatServer::clean()
{
	server->cleanup();
	delete server;
}


void ChatServer::MessageReceived(CTcpListener* listener, int client, string clientName, string msg)
{
	cout << "[" << clientName << "] " << msg << endl;

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
}