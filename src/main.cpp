/*
* 
* Call whichever implementation
*
*/

#include "ChatServer.h"

int main()
{
	ChatServer* chat = new ChatServer();

	chat->runChat();

	delete chat;
}