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
	listener->sendMsg(client, joinMsg);

	listener->sendAll(client, clientName + " has joined the server");


	cout << clientName << " has connected" << endl;
}


void ChatServer::ClientDisconnect(CTcpListener* listener, int client, string clientName)
{
	listener->sendAll(client, clientName + " has left the server");

	cout << clientName << " has disconnected" << endl;
}

void ChatServer::processClientCommand(CTcpListener* listener, int client, string msg)
{
	int i = 1;
	while (i < msg.size() && msg.at(i) != ' ') { i++; }
	std::string cmd = msg.substr(1, i - 1);

	// no arg commands
	if (i == msg.size())
	{
		if (cmd == "list")
		{
			listClients(listener, client);
		}
		else if (cmd == "help")
		{

		}
	}
	else
	{
		if (cmd == "name")
		{
			// TODO: actual arg processing
			string username = msg.substr(i + 1);
			if (username.size() > 24)
			{
				listener->sendMsg(client, "OOPS! Name too long (must be < 25 characters)");
			}
			else
			{
				renameClient(listener, client, username);
			}
		}
	}
}

void ChatServer::renameClient(CTcpListener* listener, int client, string name)
{
	string changeMsg = listener->getClientName(client) + " changed name to " + name;

	cout << changeMsg << endl;

	listener->sendAll(client, changeMsg);
	listener->sendMsg(client, "Name changed to " + name);
	listener->getClientInfo(client)->user_id = name;
}


void ChatServer::listClients(CTcpListener* listener, int client)
{
	vector<ClientInfo> client_list = listener->getClientList();

	int clientCount = client_list.size();
	string sendStr = "=== Active online: " + to_string(clientCount) + " ===\r\n";

	for (int i = 0; i < clientCount; i++)
	{
		ClientInfo client_item = client_list[i];
		string name = client_item.user_id;
		if (name == "")
		{
			name = "NONE";
		}
		sendStr += "[" + to_string(i+1) + "] " + name + "\t(" + client_item.host_id + ":" + client_item.port + ")\r\n";
	}

	listener->sendMsg(client, sendStr);
}