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
	ChatServer::setIpAddress(IP_LEHIGH);
	ChatServer::setPort(PORT);
	ChatServer::makeServer();
}

ChatServer::~ChatServer()
{
	ChatServer::clean();
}


void ChatServer::runChat()
{
	if (!server->init())
	{
		return;
	}

	server->setMessageReceivedHandler(MessageReceived);
	server->setClientConnectHandler(ClientConnect);
	server->setClientDisconnectHandler(ClientDisconnect);

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

	listener->sendAll(client, msg);
}


void ChatServer::ClientConnect(CTcpListener* listener, int client, string clientName)
{
	listener->sendMsg(client, "You joined the server");

	listener->sendAll(client, clientName + " has joined the server");

	cout << clientName << " has connected" << endl;
}


void ChatServer::ClientDisconnect(CTcpListener* listener, int client, string clientName)
{
	listener->sendAll(client, clientName + " has left the server");

	cout << clientName << " has disconnected" << endl;
}