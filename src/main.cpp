#include <iostream>
#include <string>

#include "TcpServer.h"

using namespace std;

void MessageReceived(CTcpListener* listener, int client, string clientName, string msg);

void ClientDisconnect(CTcpListener* listener, int client, string clientName);

void ClientConnect(CTcpListener* listener, int client, string clientName);

//void ClientJoined()

void main()
{
	CTcpListener server("10.217.220.39", 51515, MessageReceived);

	if (server.init())
	{
		server.addClientConnectHandler(ClientConnect);
		server.addClientDisconnectHandler(ClientDisconnect);
		server.run();
	}

	server.cleanup();
}


// callback for message received
void MessageReceived(CTcpListener* listener, int client, string clientName, string msg)
{
	// echo to client for now
	listener->sendMsg(client, msg);

	cout << "[" << clientName << "] " << msg << endl;
}

// callback for client connected
void ClientConnect(CTcpListener* listener, int client, string clientName)
{
	cout << clientName << " has connected" << endl;
}


// callback for client disconnected
void ClientDisconnect(CTcpListener* listener, int client, string clientName)
{
	cout << clientName << " has disconnected" << endl;
}