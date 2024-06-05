#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <map>
#include <string>
#include <queue>
#include "ServerJSON.hpp"
#include "ServerCommunicator.hpp"
#include "DatabaseAccess.h"

class User;
class Room;

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
	void createMenuRequestHandler(SOCKET, User, Room);
	void SignUpRequestHandler(SOCKET clientSocket, std::string, std::string);
	void MenuRequestHandler(SOCKET, User);
	bool socketStillConnected(SOCKET);
	SOCKET _serverSocket;
	std::map<std::string, SOCKET> USERS;
};

