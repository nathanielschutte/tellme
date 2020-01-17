#include "TcpServer.h"

CTcpListener::CTcpListener(std::string ipAddress, int port, MessageReceivedHandler handler)
	: m_ipAddress(ipAddress), m_port(port), MessageReceived(handler)
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

	while (true)
	{
		// create listen socket
		SOCKET listener = createSocket();
		if (listener == INVALID_SOCKET)
		{
			break;
		}

		// wait for connection
		SOCKET client = waitForConnection(listener);
		if (client != INVALID_SOCKET)
		{
			// close once client has connected so no other client can connect
			closesocket(listener);

			int bytesReceived = 0;
			do
			{
				ZeroMemory(buf, MAX_BUFFER_SIZE);

				bytesReceived = recv(client, buf, MAX_BUFFER_SIZE, 0);
				if (bytesReceived > 0)
				{
					if (MessageReceived != NULL)
					{
						std::string msg = std::string(buf, 0, bytesReceived);

						if (!msg.empty())
						{
							MessageReceived(this, client, msg);
						}
					}
				}

			} while (bytesReceived > 0);

			closesocket(client);
		}
	}
}

// cleanup
void CTcpListener::cleanup()
{
	WSACleanup();
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

// wait for a connection
SOCKET CTcpListener::waitForConnection(SOCKET listener)
{
	SOCKET client = accept(listener, NULL, NULL);
	return client;
}