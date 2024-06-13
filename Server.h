#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <map>
#include <string>
#include <exception>
#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <future>
#include <atomic>
#include <chrono>
#include <algorithm>

#include "ServerJSON.hpp"
#include "ServerCommunicator.hpp"
#include "DatabaseAccess.h"
#include "Helper.h"
#include "RequestHandlerFactory.h"
#include "Creators.h"
#include "Products.h"

class User; // Initialize User Class For Server.cpp 
class Room; // Initialize Room Class For Server.cpp 

class Server
{
public:
	Server();
	~Server();
	void serve(int port);

private:

	void acceptClient();
	void clientHandler(SOCKET clientSocket);
	void LoginManager(SOCKET clientSocket);
	/*void SignUpRequestHandler(SOCKET clientSocket, std::string, std::string);*/
	void createMenuManager(SOCKET, User, Room);
	void MenuManager(SOCKET, User);
	//void GameRequestHandler(SOCKET, User, Room);
	static bool socketStillConnected(SOCKET);
	void RoomStatusSender(SOCKET,Room, std::atomic<bool>*);
	SOCKET _serverSocket;
	std::map<std::string, SOCKET> USERS;
};

