#include "Server.h"
#include "Helper.h"
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <fstream>

#define STRING_SIZE 264
std::mutex mtx;
DatabaseAccess SQL("trivia.db");
Server::Server()
{

	// this server use TCP. that why SOCK_STREAM & IPPROTO_TCP
	// if the server use UDP we will use: SOCK_DGRAM & IPPROTO_UDP
	_serverSocket = socket(AF_INET,  SOCK_STREAM,  IPPROTO_TCP); 

	if (_serverSocket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__ " - socket");
}

Server::~Server()
{
	try
	{
		// the only use of the destructor should be for freeing 
		// resources that was allocated in the constructor
		closesocket(_serverSocket);
	}
	catch (...) {}
}

void Server::serve(int port)
{
	
	struct sockaddr_in sa = { 0 };
	
	sa.sin_port = htons(port); // port that server will listen for
	sa.sin_family = AF_INET;   // must be AF_INET
	sa.sin_addr.s_addr = INADDR_ANY;    // when there are few ip's for the machine. We will use always "INADDR_ANY"

	// Connects between the socket and the configuration (port and etc..)
	if (bind(_serverSocket, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - bind");
	
	// Start listening for incoming requests of clients
	if (listen(_serverSocket, SOMAXCONN) == SOCKET_ERROR)
		throw std::exception(__FUNCTION__ " - listen");
	std::cout << "Opening db...\n" << port << std::endl;
	if (SQL.open())
	{
		while (true)
		{
			// the main thread is only accepting clients 
			// and add then to the list of handlers
			std::cout << "listening..." << std::endl;
			acceptClient();
		}
	}
	else
	{
		std::cout << "server ne startoval\nidi nahui chmo" << std::endl;
		return;
	}
}


void Server::acceptClient()
{

	// this accepts the client and create a specific socket from server to this client
	// the process will not continue until a client connects to the server
	SOCKET client_socket = accept(_serverSocket, NULL, NULL);
	if (client_socket == INVALID_SOCKET)
		throw std::exception(__FUNCTION__);

	std::cout << "Client accepted. Server and client can speak" << std::endl;
	// the function that handle the conversation with the client
	std::thread t(&Server::clientHandler, this, client_socket);
	t.detach(); // отсоединяем поток, чтобы он работал самостоятельно
}

void Server::clientHandler(SOCKET clientSocket)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	try
	{
		while (true)
		{
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(clientSocket, &readSet);
			
			int readySockets = select(clientSocket, &readSet, nullptr, nullptr, &timeout);

			if (readySockets == 0)
			{
				ServerCommunicator::SendString(clientSocket, "Hello");
				std::thread x(&Server::LoginRequestHandler, this, clientSocket);
				x.join();
			}
			else
			{
				closesocket(clientSocket);
				return;
			}
		}
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
	SQL.close();
}

void Server::MenuRequestHandler(SOCKET clientSocket, User client)
{
	try
	{
		while (true)
		{
			ServerCommunicator::SendString(clientSocket, std::string("Zaebis"));
			std::string message = ServerCommunicator::GetString(clientSocket);
		}
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}

}

// Function that makes the login proccess for user  
void Server::LoginRequestHandler(SOCKET clientSocket)
{
	try
	{
		std::string message = ServerCommunicator::GetString(clientSocket);
		std::cout << message << std::endl;
		JsonRequestPacketDeserializer msg(message);
		json json_msg = msg.Deserializer(); // MFOCHKA|4444
		if (JSONMethods::CheckProtocol(json_msg))
		{
			if (json_msg["status"] == 101 && SQL.getIndexByName("USERS", json_msg["argument"]) == -1)
			{
				//СДЕЛАЙ ПРОВЕРКУ
				std::vector<std::string> arg = splitFunc(json_msg["argument"], "|");
				MenuRequestHandler(clientSocket,SQL.CreateUSER(arg[0], arg[1]));
			}
			if (json_msg["status"] == 101 && SQL.getIndexByName("USERS", json_msg["argument"]) != -1)
			{

			}
		}
		//Сделай проверку отсоединился ли пидорас от сокета
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}