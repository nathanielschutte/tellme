#pragma once

#include <string>
#include <iostream>

// winsock functions, lib
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (2048)

// forward declare
class CTcpListener;

// callback for data received
typedef void(*MessageReceivedHandler)(CTcpListener* listener, int socketId, std::string clientName, std::string msg);

// callback for client connect
typedef void(*ClientConnectHandler)(CTcpListener* listener, int socketId, std::string clientName);

// callback for client disconnect
typedef void(*ClientDisconnectHandler)(CTcpListener* listener, int socketId, std::string clientName);


class CTcpListener
{

public:

	CTcpListener(std::string ipAddress, int port, MessageReceivedHandler msgHandler);

	~CTcpListener();

	// send a message to specified client
	void sendMsg(int clientSocket, std::string msg);

	// init winsock
	bool init();

	// main processing loop
	void run();

	// cleanup
	void cleanup();


	// add more handlers
	void addClientDisconnectHandler(ClientDisconnectHandler cdHandler) { ClientDisconnect = cdHandler; }

	void addClientConnectHandler(ClientConnectHandler ccHandler) { ClientConnect = ccHandler; }


private:

	SOCKET createSocket();

	SOCKET waitForConnection(SOCKET listener, sockaddr_in* client);

	std::string m_ipAddress;
	int m_port;

	MessageReceivedHandler MessageReceived;
	ClientConnectHandler ClientConnect;
	ClientDisconnectHandler ClientDisconnect;
};