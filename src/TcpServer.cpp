#include "TcpServer.h"

CTcpListener::CTcpListener(std::string ipAddress, int port)
	: m_ipAddress(ipAddress), m_port(port)
{
	m_recvs = 0;
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


// TODO: allow external kill of thread/server

// main processing loop
// TODO: run as thread
void CTcpListener::run()
{
	runThread();
	cleanup();
}


// process inside thread
void CTcpListener::runThread()
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
			ServerError(this, NULL, CTCP_ERROR_INVALID_SOCKET);
			break;
		}

		// wait for connection.  m_client and m_clientSize hold connected client data
		SOCKET clientSock = waitForConnection(listener, &clientAddr);
		if (clientSock != INVALID_SOCKET)
		{
			// close once client has connected so no other client can connect
			closesocket(listener);
		
			// ---- client connected ----
			ZeroMemory(host, NI_MAXHOST);
			ZeroMemory(service, NI_MAXSERV);

			if (ClientConnect != NULL)
			{
				if (getnameinfo((sockaddr*) &clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
				{
					ClientConnect(this, clientSock, std::string(host));
				}
				else
				{
					inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
					ClientConnect(this, clientSock, std::string(host));
				}
			}
			// --------------------------

			int exitNo = 0;
			int bytesReceived = 0;


			// ---- client receive loop ----
			while (true)
			{
				ZeroMemory(buf, MAX_BUFFER_SIZE);

				bytesReceived = recv(clientSock, buf, MAX_BUFFER_SIZE, 0);
				m_recvs++;
				if (bytesReceived == SOCKET_ERROR)
				{
					exitNo = CTCP_ERROR_RECV;
					break;
				}

				if (bytesReceived == 0)
				{
					exitNo = 0;
					break;
				}

				if (MessageReceived != NULL)
				{
					std::string msg = std::string(buf, 0, bytesReceived);

					if (msg.compare("\r\n") != 0)
					{
						MessageReceived(this, clientSock, host, msg);
					}
				}
			}
			// -----------------------------


			// ---- client has disconnected ----

			if (exitNo == 0 && ClientDisconnect != NULL)
			{
				ClientDisconnect(this, clientSock, std::string(host));
			}
			if (exitNo < 0)
			{
				ServerError(this, clientSock, exitNo);
			}

			closesocket(clientSock);
			// ---------------------------------
		}
		else
		{
			ServerError(this, clientSock, CTCP_ERROR_INVALID_SOCKET);
		}
	}
}

// cleanup
void CTcpListener::cleanup()
{
	WSACleanup();
}


// generate readable error strings from CTCP codes
std::string CTcpListener::errorString(int errorCode)
{
	std::string err = "code: " + std::to_string(errorCode);

	switch (errorCode)
	{
	case CTCP_ERROR_INVALID_SOCKET: err = "Invalid socket";
		break;
	case CTCP_ERROR_RECV: err = "Error receiving message from client";
		break;
	}

	return err;
}


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