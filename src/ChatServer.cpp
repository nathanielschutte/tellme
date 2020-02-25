/*
*
* Test implementation of the server class.
* Implements the handlers to form a chat server
*
*/

#include "ChatServer.h"
#include <iostream>

using namespace std;

ChatServer::ChatServer() {
	init();
}

ChatServer::~ChatServer() {
	clean();
}

void ChatServer::init() {
	setIpAddress(IP_LEHIGH);
	setPort(PORT);
	makeServer();

	// messing with C practices
	commandCount = 0;

	addCommand("list", executeListClients, 0, "", "list all clients online");
	addCommand("name", executeName, 1, "[username]", "change username");
	addCommand("help", executeHelpMenu, 0, "", "list all commands");

	// -------------------
}

void ChatServer::addCommand(string name, ClientCommandCall call, int minCmd, string usage, string desc) {
	if (commandCount >= COMMAND_MAX) {
		return;
	}
	else {
		ClientCommand cmd;
		cmd.cName = name;
		cmd.cExecute = call;
		cmd.cMinCmd = minCmd;
		cmd.cUsage = usage;
		cmd.cDesc = desc;
		cmds.push_back(cmd);
		commandCount++;
	}
}


void ChatServer::runChat()
{
	if (!server->init())
	{
		return;
	}

	server->setMessageReceivedHandler(MessageReceived);
	server->setClientConnectHandler(ClientConnect);
	server->setClientDisconnectHandler(ClientDisconnect);

	server->run();

	server->cleanup();
}

void ChatServer::clean()
{
	server->cleanup();
	delete server;
}


void ChatServer::MessageReceived(CTcpListener* listener, int client, string clientName, string msg)
{
	if (msg.at(0) == '/')
	{
		processClientCommand(listener, client, msg);
	}
	else
	{
		cout << "[" << clientName << "] " << msg << endl;

		listener->sendAll(client, "[" + clientName + "] " + msg);
	}
}


void ChatServer::ClientConnect(CTcpListener* listener, int client, string clientName)
{
	listener->sendMsg(client, joinMsg);

	listener->sendAll(client, clientName + " has joined the server");


	cout << clientName << " has connected" << endl;
}


void ChatServer::ClientDisconnect(CTcpListener* listener, int client, string clientName)
{
	listener->sendAll(client, clientName + " has left the server");

	cout << clientName << " has disconnected" << endl;
}

void ChatServer::processClientCommand(CTcpListener* listener, int client, string msg)
{
	int i = 1;
	while (i < msg.size() && msg.at(i) == ' ') { i++; }
	int j = i;
	while (i < msg.size() && msg.at(i) != ' ') { i++; }
	std::string cmd = msg.substr(j, i - 1);

	argv_t argv;
	int argc;
	if (i < msg.size()) {
		argv = parseArguments(msg.substr(i + 1));
		argc = argv.size();
	}
	else {
		argc = 0;
	}

	bool execSuccess = false;
	for (i = 0; i < commandCount; i++) {
		ClientCommand clientCmd = cmds[i];
		if (clientCmd.cName == cmd) {
			execSuccess = true;
			
			// not enough arguments provides, do not execute
			if (clientCmd.cMinCmd > argc) {
				listener->sendMsg(client, "Not enough arguments! Usage: /" + clientCmd.cName + " " + clientCmd.cUsage);
				
				break;
			}
			
			// command found, execute if not null
			ClientCommandCall call = clientCmd.cExecute;
			if (call != NULL) {
				call(listener, client, argv);
			}
			break;
		}
	}

	// command not found
	if (!execSuccess) {
		listener->sendMsg(client, "\'" + cmd + "\' command not found");
	}
}

argv_t ChatServer::parseArguments(string msg) {
	int i = 0;
	int begin;
	argv_t res;
	while (i < msg.size()) {
		begin = i;
		while (i < msg.size() && msg.at(i) != ' ') { i++; }
		res.push_back(msg.substr(begin, i - begin));
		i++;
	}
	return res;
}


void ChatServer::renameClient(CTcpListener* listener, int client, string name)
{
	string changeMsg = listener->getClientName(client) + " changed name to " + name;

	cout << changeMsg << endl;

	listener->sendAll(client, changeMsg);
	listener->sendMsg(client, "Name changed to " + name);
	listener->getClientInfo(client)->user_id = name;
}



// ========== Command Definitions ==========

void ChatServer::executeListClients(CTcpListener* listener, int client, argv_t argv)
{
	vector<ClientInfo> client_list = listener->getClientList();

	int clientCount = client_list.size();
	string sendStr = "\n\r=== Active online: " + to_string(clientCount) + " ===\r\n";

	for (int i = 0; i < clientCount; i++)
	{
		ClientInfo client_item = client_list[i];
		string name = client_item.user_id;
		if (name == "")
		{
			name = "NONE";
		}
		sendStr += "[" + to_string(i+1) + "] " + name + "\t(" + client_item.host_id + ":" + client_item.port + ")\r\n";
	}

	listener->sendMsg(client, sendStr);
}

void ChatServer::executeName(CTcpListener* listener, int client, argv_t argv) {
	string username = argv[0];
	if (username.size() > 24)
	{
		listener->sendMsg(client, "OOPS! Name too long (must be < 25 characters)");
	}
	else
	{
		renameClient(listener, client, username);
	}
}

void ChatServer::executeHelpMenu(CTcpListener* listener, int client, argv_t argv) {
	string menu = "";
	for (int i = 0; i < commandCount; i++) {
		ClientCommand c = cmds[i];
		menu += " - \'" + c.cName + "\' " + c.cDesc + "\r\n   Usage: /" +c.cName + " " + c.cUsage + "\r\n";
		if (i < commandCount - 1) {
			menu += "\n";
		}
	}
	listener->sendMsg(client, menu);
}

