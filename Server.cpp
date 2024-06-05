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
RoomManager Rooms = RoomManager();
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
		Rooms.UpdateRoomsList(SQL.GetRooms());
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
		std::cout << "server ne startoval\n" << std::endl;
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

/*-----------------------------------------------------------------------------------------------------------------------
* METHODS AND HANDLERS
* Handlers Chain:
LoginRequestHandler --> (if user exists) 	 -->					  --> MenuRequestHandler --> createMenuRequestHandler
					--> (if user not exists) --> SignUpRequestHandler
-----------------------------------------------------------------------------------------------------------------------*/
bool Server::socketStillConnected(SOCKET socket_fd) // Checks if the socket connection is still valid
{
	char error[sizeof(int)];
	int len = sizeof(error);
	int retval = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, error, &len);
	if (retval != 0)
	{
		return false;
	}
	return true;
}

// Handler that makes the login proccess for user  
void Server::LoginRequestHandler(SOCKET clientSocket)
{
	try
	{
		while(socketStillConnected(clientSocket))
		{ 
			std::string message = ServerCommunicator::GetString(clientSocket);
			std::cout << message << std::endl;
			JsonRequestPacketDeserializer msg(message);
			json json_msg = msg.Deserializer(); // MFOCHKA|4444
			if (JSONMethods::CheckProtocol(json_msg))
			{
				std::cout << "secondMessage: " << socketStillConnected(clientSocket) << std::endl;
				if (json_msg["status"] == 101)
				{
					std::vector<std::string> arg = splitFunc(json_msg["argument"], "|");
					std::cout << arg[0] + arg[1] << std::endl;
					if (SQL.CheckLogin(arg[0], arg[1]))
					{
						MenuRequestHandler(clientSocket, User(arg[0], SQL.getIndexByName("USERS", arg[0]))); 
						break;
					}
					else
					{
						SignUpRequestHandler(clientSocket, arg[0], arg[1]);
					}
				}
				else
				{
					ServerCommunicator::SendString(clientSocket, std::string("Error!\n"));
				}
			}
		}
		std::cout << "Client Disconnected" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

// Handler that makes the signup proccess for user  
void Server::SignUpRequestHandler(SOCKET clientSocket, std::string NAME, std::string PASSWORD)
{
	ServerCommunicator::SendString(clientSocket, "{\"status\": 141, \"argument\": \"No User Found\"}");
	while (socketStillConnected(clientSocket))
	{
		try {
			std::string secondMessage = ServerCommunicator::GetString(clientSocket);
			std::cout << secondMessage << std::endl;
			JsonRequestPacketDeserializer secMsg(secondMessage);
			json json_msgg = secMsg.Deserializer();
			if (json_msgg["status"] == 102)
			{
				MenuRequestHandler(clientSocket, SQL.CreateUSER(NAME, PASSWORD));
				break;
			}
			else
			{
				ServerCommunicator::SendString(clientSocket, "{\"status\": 144, \"argument\": \"Bad Request!\"}");
			}
		}
		catch (const std::exception x)
		{

		}
	}
	std::cout << "Client Disconnected" << std::endl;
}

void Server::MenuRequestHandler(SOCKET clientSocket, User client)
{
	ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 111, \"argument\": \"User Logined\"}"));
	try
	{
		while (socketStillConnected(clientSocket))
		{
			std::string message = ServerCommunicator::GetString(clientSocket);
			JsonRequestPacketDeserializer msg(message);
			json json_msg = msg.Deserializer();
			if (JSONMethods::CheckProtocol(json_msg))
			{
				if (json_msg["status"] == 201) // If User Creates Room
				{
					std::vector<std::string> secondMessage = splitFunc(json_msg["argument"], "|"); // Splits message
					try
					{
						Room newCreatedRoom = Rooms.createRoom(SQL.IdCheck("ROOMS", 0), secondMessage[0], stoi(secondMessage[1]), stoi(secondMessage[2]));
						User* userToRoom = new User(client.GetName(), client.GetID());
						newCreatedRoom.UserAdd(userToRoom);
						SQL.CreateROOM(newCreatedRoom);
						ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 333, \"argument\": \"Room Created!\"}"));
						createMenuRequestHandler(clientSocket, client, newCreatedRoom);
					}
					catch (const std::exception& e)
					{
						ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 314, \"argument\": \"Not Correct Info!\"}"));
					}
				}
				if (json_msg["status"] == 211) // If User Joins Room
				{
					std::vector<std::string> secondMessage = splitFunc(json_msg["argument"], "|"); // Splits message
					try
					{
						Room FoundRoom = Rooms.GetRoom(stoi(secondMessage[1])); // Find Room The User Want To Join
						if (FoundRoom.GetName() == secondMessage[0])
						{
							if (Rooms.AddUserToRoom(stoi(secondMessage[1]), client))
							{
								SQL.AddUser(stoi(secondMessage[1]), client.GetName());
								ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 334, \"argument\": \"Joined To Room!\"}"));
								createMenuRequestHandler(clientSocket, client, FoundRoom);
							}
							else
							{
								ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 314, \"argument\": \"Room Is Full!\"}"));
							}
						}
						else
						{
							ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 314, \"argument\": \"Not Correct Info!\"}"));
						}
					}
					catch (const std::exception& e)
					{
						ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 314, \"argument\": \"Not Correct Info!\"}"));
					}
				}
			}
		}
		std::cout << "Client Disconnected" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		closesocket(clientSocket);
	}
}

void Server::createMenuRequestHandler(SOCKET clientSocket, User Admin, Room CreatedRoom)
{
	std::string currentUsers = Rooms.GetRoom(CreatedRoom.GetID()).GetUsers();
	std::string roomInfo = CreatedRoom.GetName() + "|" + std::to_string(CreatedRoom.GetID()) + "|" + currentUsers;
	ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 335, \"argument\": \"Status Update!|" + roomInfo +  "\"}"));
	//std::cout << std::string("{\"status\": 334, \"argument\": \"Joined Room!|" + roomInfo + "\"}") << std::endl;
	try
	{
		while (socketStillConnected(clientSocket))
		{
			std::string message = ServerCommunicator::GetString(clientSocket);
			Room UpdatedRoom = Rooms.GetRoom(CreatedRoom.GetID());
			std::string UpdatedListOfUsers = UpdatedRoom.GetUsers();
			if (currentUsers != UpdatedListOfUsers)
			{
				std::string roomInfo = UpdatedRoom.GetName() + "|" + std::to_string(UpdatedRoom.GetID()) + "|" + UpdatedListOfUsers;
				std::cout << std::string("{\"status\": 335, \"argument\": \"Status Update!|" + roomInfo + "\"}") << std::endl;
				currentUsers = UpdatedListOfUsers;
			}
		}
	}
	catch (const std::exception& e)
	{
		Room UserDisconnected = Rooms.GetRoom(CreatedRoom.GetID());
		UserDisconnected.RemoveUser(&Admin);
		Rooms.updateRoom(&UserDisconnected);
	}
}

RoomManager X()
{
	return RoomManager();
}
