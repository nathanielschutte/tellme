/*
*
* Test implementation of the server class.
* Implements the handlers to form a chat server
*
*/

#pragma once

#include "IServer.h"

// ---- Common test net values ----

#define IP_WILLIAMS "192.168.2.64"
#define IP_QUEENS "10.217.220.39"
#define IP_LEHIGH "128.180.235.130"
#define PORT 51115

// --------------------------------

typedef std::vector<std::string> argv_t;
typedef void(*ClientCommandCall)(CTcpListener* listener, int client, argv_t argv);
struct ClientCommand {
	std::string cName;
	ClientCommandCall cExecute;
	int cMinCmd;
	std::string cUsage;
	std::string cDesc;
};
typedef std::vector<ClientCommand> cmds_t;

const int COMMAND_MAX = 20;
const std::string joinMsg = "======= You joined the server ========\r\nPreface commands with forward slash \'/\'.\r\nUse \'/help\' for commands list\r\n======================================\r\n";


// allocate client commands, setable
static cmds_t cmds;

// ClientCommand actual count
static int commandCount;

class ChatServer : public IServer
{
public:

	ChatServer();

	~ChatServer();


	// ---- Handlers ----
	static void MessageReceived(CTcpListener* listener, int client, std::string clientName, std::string msg);

	static void ClientDisconnect(CTcpListener* listener, int client, std::string clientName);

	static void ClientConnect(CTcpListener* listener, int client, std::string clientName);

	static void ServerError(CTcpListener* listener, int client, int error, bool fatal);

	// ------------------


	// ---- Unique functionality ----
	void init();

	void runChat();

	void clean();

	// ------------------------------

private:


	static void processClientCommand(CTcpListener* listener, int client, std::string cmd);

	static void renameClient(CTcpListener* listener, int client, std::string name);

	// ===== command processing =====
	static argv_t parseArguments(std::string msg);

	static void addCommand(std::string name, ClientCommandCall call, int minCmd, std::string usage, std::string desc);

	// ===== command implementations =====
	static void executeName(CTcpListener* listener, int client, argv_t argv);

	static void executeListClients(CTcpListener* listener, int client, argv_t argv);

	static void executeHelpMenu(CTcpListener* listener, int client, argv_t argv);
};