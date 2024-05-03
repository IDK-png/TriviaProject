#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <map>
#include <string>
#include <queue>
#include "ServerJSON.hpp"
#include "ServerCommunicator.hpp"
#include "DatabaseAccess.h"

class Server
{
public:
	Server();
	~Server();
	void serve(int port);

private:

	void acceptClient();
	void clientHandler(SOCKET clientSocket);
	void LoginRequestHandler(SOCKET clientSocket);
	void MenuRequestHandler(SOCKET, User);
	SOCKET _serverSocket;
	std::map<std::string, SOCKET> USERS;
};

