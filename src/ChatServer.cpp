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
	if (msg.at(0) == '/')
	{
		processClientCommand(listener, client, msg);
	}
	else
	{
		cout << "[" << clientName << "] " << msg << endl;

		listener->sendAll(client, msg);
	}
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

void ChatServer::processClientCommand(CTcpListener* listener, int client, std::string msg)
{
	int i = 1;
	while (i < msg.size() && msg.at(i) != ' ') { i++; }

	// no arg commands
	if (i == msg.size())
	{
		// TODO: support more
	}
	else
	{
		std::string cmd = msg.substr(1, i - 1);

		// TODO: better command identification and execution
		if (cmd == "name")
		{

			// TODO: actual arg processing
			string username = msg.substr(i + 1);
			string changeMsg = listener->getClientName(client) + " changed name to " + username;

			cout << changeMsg << endl;

			listener->sendAll(client, changeMsg);
			listener->sendMsg(client, "Name changed to " + username);
			listener->getClientInfo(client)->user_id = username;
		}
	}
}