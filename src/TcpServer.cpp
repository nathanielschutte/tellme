#include "TcpServer.h"

CTcpListener::CTcpListener(std::string ipAddress, int port)
	: m_ipAddress(ipAddress), m_port(port)
{
	m_recvs = 0;

	setErrorHandler(DefaultServerError);
}


CTcpListener::~CTcpListener()
{
	cleanup();
}

// send a message to specified client
void CTcpListener::sendMsg(int clientSocket, std::string msg)
{
	msg = msg + "\r\n";
	send(clientSocket, msg.c_str(), msg.size() + 1, 0);
}

void CTcpListener::sendAll(int clientSocket, std::string msg)
{
	for (unsigned int i = 0; i < m_master.fd_count; i++)
	{
		SOCKET outSock = m_master.fd_array[i];
		if (outSock != m_listen && outSock != clientSocket)
		{
			sendMsg(outSock, msg);
		}
	}
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
}


// process inside thread
void CTcpListener::runThread()
{
	while (true)
	{
		FD_ZERO(&m_master);

		// ---- create listen socket ----
		m_listen = createSocket();
		if (m_listen == INVALID_SOCKET)
		{
			ServerError(this, NULL, CTCP_ERROR_INVALID_SOCKET, true);
			break;
		}
		// ------------------------------

		FD_SET(m_listen, &m_master);

		int bytesReceived = 0;
		bool running = true;

		// ---- client receive loop ----
		while (running)
		{

			// copy master file descriptor set
			fd_set copy = m_master;
			int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

			for (int i = 0; i < socketCount; i++)
			{
				SOCKET sock = copy.fd_array[i];

				// ---- client joined ----
				if (sock == m_listen)
				{
					sockaddr_in clientAddr;

					SOCKET clientSock = waitForConnection(m_listen, &clientAddr);

					char host[NI_MAXHOST];
					char service[NI_MAXHOST];
					ZeroMemory(host, NI_MAXHOST);

					if (getnameinfo((sockaddr*)&clientAddr, sizeof(clientAddr), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
					{
						addClientInfo(clientSock, service, std::string(host));
					}
					else
					{
						inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
						addClientInfo(clientSock, std::to_string(ntohs(clientAddr.sin_port)), std::string(host));
					}

					if (ClientConnect != NULL)
					{
						ClientConnect(this, clientSock, std::string(host));
					}

					FD_SET(clientSock, &m_master);

				}
				// -----------------------

				// ---- receive message from client ----
				else
				{
					char buf[MAX_BUFFER_SIZE];
					ZeroMemory(buf, MAX_BUFFER_SIZE);

					bytesReceived = recv(sock, buf, MAX_BUFFER_SIZE, 0);

					if (bytesReceived == SOCKET_ERROR)
					{
						ServerError(this, sock, CTCP_ERROR_RECV, false);
						break;
					}

					if (bytesReceived <= 0)
					{
						ClientDisconnect(this, sock, getClientName(sock));

						FD_CLR(sock, &m_master);
						deleteClientInfo(sock);
						closesocket(sock);
					}
					else
					{
						std::string msg = std::string(buf, bytesReceived);
						if (msg != "\r\n" && stripMsg(msg, true))
						{
							MessageReceived(this, sock, getClientName(sock), msg);
						}
					}
				} // -----------------------------------
			}

			// ---------------------------------
		}

		FD_CLR(m_listen, &m_master);
		closesocket(m_listen);
	}
}



// cleanup
void CTcpListener::cleanup()
{
	//ServerCleanup();
	while (m_master.fd_count > 0)
	{
		SOCKET sock = m_master.fd_array[0];
		FD_CLR(sock, &m_master); // clear in set
		deleteClientInfo(sock); // clear in info vector
		closesocket(sock); // close socket
	}

	WSACleanup();
}


// get client name from socket number, default to hostname if no name given
std::string CTcpListener::getClientName(int clientSock)
{
	std::string name = "";

	for (unsigned int i = 0; i < m_client_list.size(); i++)
	{
		ClientInfo check = m_client_list[i];
		if (clientSock == check.sock)
		{
			if (check.user_id.compare("") != 0)
			{
				name = check.user_id;
			}
			else {
				name = check.host_id;
			}
		}
	}

	return name;
}


// get socket number for client with name (user or host name)
int CTcpListener::getClientSocket(std::string name)
{
	int socket = -1;

	for (unsigned int i = 0; i < m_client_list.size(); i++)
	{
		ClientInfo check = m_client_list[i];
		if (name == check.user_id || name == check.host_id)
		{
			socket = check.sock;
		}
	}

	return socket;
}


// return clientinfo data, settable
ClientInfo* CTcpListener::getClientInfo(int clientSock)
{
	for (unsigned int i = 0; i < m_client_list.size(); i++)
	{
		ClientInfo check = m_client_list[i];
		if (clientSock == check.sock)
		{
			return &m_client_list[i];
		}
	}
}

// get the full list
std::vector<ClientInfo> CTcpListener::getClientList()
{
	return m_client_list;
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


// add client info
void CTcpListener::addClientInfo(int clientSock, std::string port, std::string host)
{
	ClientInfo new_client;
	new_client.sock = clientSock;
	new_client.host_id = host;
	new_client.user_id = "";
	m_client_list.push_back(new_client);
}


// remove specified client info
void CTcpListener::deleteClientInfo(int clientSock)
{
	for (unsigned int i = 0; i < m_client_list.size(); i++)
	{
		if (clientSock == m_client_list[i].sock)
		{
			m_client_list.erase(m_client_list.begin() + i);
			break;
		}
	}
}


// remove client info as stack
void CTcpListener::popClientInfo(int clientSock)
{
	m_client_list.pop_back();
}


// rid trailing spaces and newline characters, optionally rid leading spaces (front_space)
bool CTcpListener::stripMsg(std::string& msg, bool front_space)
{
	if (msg == "\r\n" || msg == "\n")
	{
		return false;
	}

	int msgSize = msg.size();
	int begin = 0;
	int end = msgSize - 1;

	if (front_space)
	{
		while (begin < msgSize && msg.at(begin) == ' ')
		{
			begin++;
		}
		if (begin >= msgSize)
		{
			return false;
		}
	}

	bool trash = true;

	for (int i = end; i >= begin; i--)
	{
		if (msg.at(i) == '\r' || msg.at(i) == '\n' || msg.at(i) == ' ')
		{
			end--;
		}
		else
		{
			trash = false;
			break;
		}
	}

	if (trash) {
		return false;
	}

	msg = msg.substr(begin, end - begin + 1);

	return true;
}


// substring
void CTcpListener::subStrCpy(char* dest, const char* src, int begin, int end) 
{
	// TODO: better checks and error handling (this is rushed)
	if (begin > end || begin < 0 || end >= strlen(src) 
		|| end >= strlen(dest) || (begin - end + 1) > strlen(dest))
	{
		return;
	}

	int k = 0;
	for (int i = begin; i <= end; i++)
	{
		dest[k] = src[i];
		k++;
	}
}



// ---- static default handler definitions ----

void CTcpListener::DefaultServerError(CTcpListener* listener, int client, int error, bool fatal)
{
	std::cout << "SERVER ERROR: " << listener->errorString(error) << std::endl;

	if (fatal)
	{
		system("exit");
	}
}
// --------------------------------------------