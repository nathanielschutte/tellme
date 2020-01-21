#pragma once

#include <string>
#include <iostream>
#include <thread>
#include <time.h>
#include <vector>

// winsock functions, lib
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (2048)

// TODO: error code enum when expanded
#define CTCP_ERROR_RECV -1
#define CTCP_ERROR_INVALID_SOCKET -2

// forward declare
class CTcpListener;

// callback for data received
typedef void(*MessageReceivedHandler)(CTcpListener* listener, int socketId, std::string clientName, std::string msg);

// callback for client connect/disconnect
typedef void(*ClientConnectHandler)(CTcpListener* listener, int socketId, std::string clientName);

// callback for error
typedef void(*ServerErrorHandler)(CTcpListener* listener, int socketId, int error, bool fatal);


struct ClientInfo
{
	int sock;
	std::string port;
	std::string host_id;
	std::string user_id;
	
};

class CTcpListener
{

public:

	CTcpListener(std::string ipAddress, int port);

	~CTcpListener();

	// send a message to specified client
	void sendMsg(int clientSocket, std::string msg);

	// send a message to all other clients
	void sendAll(int clientSocket, std::string msg);

	// init winsock
	bool init();

	// main processing loop
	void run();

	// cleanup
	void cleanup();


	// add more handlers
	void setMessageReceivedHandler(MessageReceivedHandler msgHandler) { MessageReceived = msgHandler; }

	void setClientDisconnectHandler(ClientConnectHandler cdHandler) { ClientDisconnect = cdHandler; }

	void setClientConnectHandler(ClientConnectHandler ccHandler) { ClientConnect = ccHandler; }

	void setErrorHandler(ServerErrorHandler seHandler) { ServerError = seHandler; }


	// client info stuff
	std::string getClientName(int clientSock);




	// error handling
	std::string errorString(int errorCode);

private:

	std::string m_ipAddress;
	int m_port;
	int m_recvs;
	fd_set m_master;
	SOCKET m_listen;

	std::vector<ClientInfo> m_client_list;

	void runThread();

	// ---- helper net functions ----
	SOCKET createSocket();

	SOCKET waitForConnection(SOCKET listener, sockaddr_in* client);

	void addClientInfo(int clientSock, std::string port, std::string host);

	void deleteClientInfo(int clientSock);

	void popClientInfo(int clientSock);

	char* stripMsg(char* msg, bool front_space);

	// ------------------------------

	void subStrCpy(char* dest, const char* src, int begin, int end);


	// ---- default handlers ----
	static void DefaultServerError(CTcpListener* listener, int client, int error, bool fatal);

	// --------------------------

	MessageReceivedHandler MessageReceived;
	ClientConnectHandler ClientConnect;
	ClientConnectHandler ClientDisconnect;
	ServerErrorHandler ServerError;
};