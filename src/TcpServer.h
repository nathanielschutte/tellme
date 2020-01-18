#pragma once

#include <string>
#include <iostream>
#include <thread>

// winsock functions, lib
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (2048)

#define CTCP_ERROR_RECV -1
#define CTCP_ERROR_INVALID_SOCKET -2

// forward declare
class CTcpListener;

// callback for data received
typedef void(*MessageReceivedHandler)(CTcpListener* listener, int socketId, std::string clientName, std::string msg);

// callback for client connect/disconnect
typedef void(*ClientConnectHandler)(CTcpListener* listener, int socketId, std::string clientName);

// callback for error
typedef void(*ServerErrorHandler)(CTcpListener* listener, int socketId, int error);


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
	void addClientDisconnectHandler(ClientConnectHandler cdHandler) { ClientDisconnect = cdHandler; }

	void addClientConnectHandler(ClientConnectHandler ccHandler) { ClientConnect = ccHandler; }

	void addErrorHandler(ServerErrorHandler seHandler) { ServerError = seHandler; }



	// error handling
	std::string errorString(int errorCode);

private:

	SOCKET createSocket();

	SOCKET waitForConnection(SOCKET listener, sockaddr_in* client);

	void runThread();

	std::string m_ipAddress;
	int m_port;

	MessageReceivedHandler MessageReceived;
	ClientConnectHandler ClientConnect;
	ClientConnectHandler ClientDisconnect;
	ServerErrorHandler ServerError;
};