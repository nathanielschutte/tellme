#pragma once

#include <string>

// winsock functions, lib
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER_SIZE (2048)

// forward declare
class CTcpListener;

// callback to data received
typedef void(*MessageReceivedHandler)(CTcpListener* listener, int socketId, std::string msg);


class CTcpListener
{

public:

	CTcpListener(std::string ipAddress, int port, MessageReceivedHandler handler);

	~CTcpListener();

	// send a message to specified client
	void sendMsg(int clientSocket, std::string msg);

	// init winsock
	bool init();

	// main processing loop
	void run();

	// cleanup
	void cleanup();


private:

	SOCKET createSocket();

	SOCKET waitForConnection(SOCKET listener);

	std::string m_ipAddress;
	int m_port;
	MessageReceivedHandler MessageReceived;
};