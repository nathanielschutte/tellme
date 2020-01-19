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

	while (true)
	{

		fd_set master;
		FD_ZERO(&master);

		// create listen socket
		SOCKET listener = createSocket();
		if (listener == INVALID_SOCKET)
		{
			ServerError(this, NULL, CTCP_ERROR_INVALID_SOCKET);
			break;
		}

		FD_SET(listener, &master);

		int exitNo = 0;
		int bytesReceived = 0;
		bool running = true;

		// ---- client receive loop ----
		while (running)
		{

			// copy master file descriptor set
			fd_set copy = master;

			int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

			for (int i = 0; i < socketCount; i++)
			{

				SOCKET sock = copy.fd_array[i];

				if (sock == listener)
				{
					sockaddr_in clientAddr;

					SOCKET clientSock = waitForConnection(listener, &clientAddr);

					FD_SET(clientSock, &master);

					std::string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
					send(clientSock, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
				}
				else
				{
					char buf[MAX_BUFFER_SIZE];
					ZeroMemory(buf, MAX_BUFFER_SIZE);

					//char host[NI_MAXHOST];
					//char service[NI_MAXSERV];

					bytesReceived = recv(sock, buf, MAX_BUFFER_SIZE, 0);

					if (bytesReceived == SOCKET_ERROR)
					{
						exitNo = CTCP_ERROR_RECV;
						break;
					}

					if (bytesReceived <= 0)
					{
						closesocket(sock);
						FD_CLR(sock, &master);
					}
					else
					{
						if (buf[0] == '\\')
						{
							std::string cmd = std::string(buf, bytesReceived);
							if (cmd == "\\quit")
							{
								running = false;
								break;
							}

							continue;
						}

						for (unsigned int i = 0; i < master.fd_count; i++)
						{
							SOCKET outSock = master.fd_array[i];
							if (outSock != listener && outSock != sock)
							{
								std::string msg = std::string(buf, bytesReceived);
								std::string strOut = "SOCKET #" + sock;
								strOut = strOut + ": " + msg + "\r\n";

								send(outSock, strOut.c_str(), strOut.size() + 1, 0);
							}
						}
					}
				}
			}

			// ---------------------------------
		}

		FD_CLR(listener, &master);
		closesocket(listener);

		std::string msg = "Server is shutting down.\r\n";

		while (master.fd_count > 0)
		{
			SOCKET sock = master.fd_array[0];

			send(sock, msg.c_str(), msg.size() + 1, 0);

			FD_CLR(sock, &master);
			closesocket(sock);
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