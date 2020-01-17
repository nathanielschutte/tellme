#include "TcpServer.h"

CTcpListener::CTcpListener(std::string ipAddress, int port, MessageReceivedHandler msgHandler)
	: m_ipAddress(ipAddress), m_port(port), MessageReceived(msgHandler)
{

}


CTcpListener::~CTcpListener()
{
	cleanup();
}

// send a message to specified client
void CTcpListener::sendMsg(int clientSocket, std::string msg)
{
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

// init winsock
bool CTcpListener::init()
{
	WSAData data;
	WORD ver = MAKEWORD(2, 2);

	int wsInit = WSAStartup(ver, &data);

	// TODO: handle error better for caller
	return wsInit == 0;
}

// main processing loop
void CTcpListener::run()
{
	char buf[MAX_BUFFER_SIZE];
	char host[NI_MAXHOST];
	char service[NI_MAXSERV];
	sockaddr_in clientAddr;

	while (true)
	{
		// create listen socket
		SOCKET listener = createSocket();
		if (listener == INVALID_SOCKET)
		{
			std::cerr << "Invalid socket " << errno << std::endl;
			break;
		}

		// wait for connection.  m_client and m_clientSize hold connected client data
		SOCKET clientSock = waitForConnection(listener, &clientAddr);
		if (clientSock != INVALID_SOCKET)

			// close once client has connected so no other client can connect
			closesocket(listener);
		{
			// ---- client connected ----
			ZeroMemory(host, NI_MAXHOST);
			ZeroMemory(service, NI_MAXSERV);

			if (ClientConnect != NULL)
			{
				if (getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
				{
					ClientConnect(this, clientSock, std::string(host));
					std::cout << host << std::endl;
				}
				else
				{
					inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
					ClientConnect(this, clientSock, std::string(host));
				}
			}
			// --------------------------

			int bytesReceived = 0;
			do
			{
				ZeroMemory(buf, MAX_BUFFER_SIZE);

				bytesReceived = recv(clientSock, buf, MAX_BUFFER_SIZE, 0);
				if (bytesReceived > 0)
				{
					if (MessageReceived != NULL)
					{
						
						// send TcpServer, client socket ID, client name, and the message received
						MessageReceived(this, clientSock, host, std::string(buf, 0, bytesReceived));
					}
				}

			} while (bytesReceived > 0);

			// ---- client has disconnected ----

			if (ClientDisconnect != NULL)
			{
				ClientDisconnect(this, clientSock, std::string(host));
			}


			closesocket(clientSock);
			// ---------------------------------
		}
	}
}

// cleanup
void CTcpListener::cleanup()
{
	WSACleanup();
}


// create a socket
// create a socket
SOCKET CTcpListener::createSocket()
{
	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);

	if (listener != INVALID_SOCKET)
	{

		// define server address
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(m_port);
		inet_pton(AF_INET, m_ipAddress.c_str(), &addr.sin_addr); // ip string -> byte

		// bind address to socket
		int bindOk = bind(listener, (sockaddr*)&addr, sizeof(addr));
		if (bindOk != SOCKET_ERROR)
		{
			int listenOk = listen(listener, SOMAXCONN);
			if (listenOk == SOCKET_ERROR)
			{
				return -1;
			}
		}
		else
		{
			return -1;
		}
	}

	return listener;
}

// wait for a connection.  return socket ID and set client sockaddr info
SOCKET CTcpListener::waitForConnection(SOCKET listener, sockaddr_in* client)
{
	int clientSize = sizeof(*client);
	SOCKET clientSock = accept(listener, (sockaddr*) client, &clientSize);
	return clientSock;
}