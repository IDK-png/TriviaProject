#include "Server.h"

#define STRING_SIZE 264
std::mutex mtx;
DatabaseAccess* SQL = new DatabaseAccess("trivia.db");
RoomManager* Rooms = new RoomManager();

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
	if (SQL->open())
	{
		Rooms->UpdateRoomsList(SQL->GetRooms());
		DataManage::getInstance(SQL,Rooms);
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
				std::thread x(&Server::LoginManager, this, clientSocket);
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
	SQL->close();
	delete SQL;
	delete Rooms;
}

/*-----------------------------------------------------------------------------------------------------------------------
* METHODS AND HANDLERS
* Handlers Chain:
LoginManager --> (if user exists) 	 -->					  --> MenuManager --> createMenuManager
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
void Server::LoginManager(SOCKET clientSocket)
{
	HandlerFactory* LoginHandler = new LoginRequestHandlerCreator();
	// Calls A Creator For Login Proccess Requests Handler
	try
	{
		while(socketStillConnected(clientSocket))
		// Continue Get Requests While Client Is Still Connected
		{ 
			JsonRequestPacketDeserializer requestJson(ServerCommunicator::GetString(clientSocket));
			json request_json = requestJson.Deserializer();
			// Get client's Request, and Deserialize JSON
			JsonRequestPacketDeserializer responseJson(LoginHandler->FactoryMethod()->RequestResult(request_json, -1, -1));
			json response_json = responseJson.Deserializer();
			// Get Response to client's Request
			ServerCommunicator::SendJSON(clientSocket, response_json);
			// Send Response To client
			if (response_json["status"] == 112 || response_json["status"] == 111)
				// If User Signed Up, or Login Successfully, Send Client to Menu Manager
			{
				std::vector<std::string> arg = ServerCommunicator::splitFunc(request_json["argument"], "|");
				MenuManager(clientSocket, User(arg[0], SQL->getIndexByName("USERS", arg[0])));
				// Send To Menu Manager The Socket, And Client's User As An "User" Object
			}
		}
		std::cout << "Client Disconnected" << std::endl;
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}

// Handler that makes the Menu proccess for user(Join/Create Room, Get User's Statistics)
void Server::MenuManager(SOCKET clientSocket, User client)
{
	HandlerFactory* MenuHandler = new MenuRequestHandlerCreator();
	// Calls A Creator For Login Proccess Requests Handler
	try
	{
		while (socketStillConnected(clientSocket))
		// Continue Get Requests While Client Is Still Connected
		{
			JsonRequestPacketDeserializer requestJson(ServerCommunicator::GetString(clientSocket));
			json request_json = requestJson.Deserializer();
			// Get client's Request, and Deserialize JSON
			JsonRequestPacketDeserializer responseJson(MenuHandler->FactoryMethod()->RequestResult(request_json, client.GetID(), -1));
			json response_json = responseJson.Deserializer();
			// Get Response to client's Request
			ServerCommunicator::SendJSON(clientSocket, response_json);
			// Send Response To client
			if (response_json["status"] == 333 || response_json["status"] == 334)
				// If User Joined, or Created Room Successfully, Send Client to Waiting Room Manager(createMenuManager)
			{
				std::vector<std::string> arg = ServerCommunicator::splitFunc(request_json["argument"], "|");
				Room f = Rooms->GetRoom(SQL->getIndexByName("ROOMS", arg[0]));
				// Create "Room" Object of Room Client Created/Joined
				createMenuManager(clientSocket, client, Rooms->GetRoom(SQL->getIndexByName("ROOMS",arg[0])));
				// Send To Waiting Room Manager(createMenuManager)
			}
		}
		std::cout << "Client Disconnected" << std::endl;
	}
	catch (const std::exception& e)
	{
		closesocket(clientSocket);
	}
}

// Function That Sends To The Client Updated Status Of His Current Room
// That Means, If Someones Join,Left or Started Game, This Function Will Send Status Update To Client 
void Server::RoomStatusSender(SOCKET clientSocket, Room CreatedRoom, std::atomic<bool>* Flag)
{
	std::string currentUsers = Rooms->GetRoom(CreatedRoom.GetID()).GetUsers();
	// Get Current Users Of The Room
	std::string roomInfo = CreatedRoom.GetName() + "|" + std::to_string(CreatedRoom.GetID()) + "|" + currentUsers;
	ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 335, \"argument\": \"Status Update!|" + roomInfo + "\"}"));
	// Send Current Room Status, The Status Contains Information About What Users In the Room Right Now
	while (Flag->load())
	{
		Room UpdatedRoom = Rooms->GetRoom(CreatedRoom.GetID());
		// Gets The Updated Room From Rooms Manager
		if (UpdatedRoom.GetID() != -1) // Checks If Room Is Not Closed 
		{
			std::string UpdatedListOfUsers = UpdatedRoom.GetUsers();
			// Gets The Users From The Updated Room
			UpdatedListOfUsers.erase(std::remove(UpdatedListOfUsers.begin(), UpdatedListOfUsers.end(), '\n'), UpdatedListOfUsers.cend());
			// Removes All '\n'
			if (currentUsers != UpdatedListOfUsers)
			{
				std::string roomInfo = UpdatedRoom.GetName() + "|" + std::to_string(UpdatedRoom.GetID()) + "|" + UpdatedListOfUsers;
				ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 335, \"argument\": \"Status Update!|" + roomInfo + "\"}"));
				currentUsers = UpdatedListOfUsers;
			}
			if (UpdatedRoom.GetState())
			{
				ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 402, \"argument\": \"Admin Started The Game!\"}"));
				Flag->store(false); 
				// Stop Sending Status
			}
		}
		else
		{
			ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 355, \"argument\": \"Room Closed!\"}"));
			Flag->store(false);
			// Stop Sending Status
		}
	}
}
void Server::createMenuManager(SOCKET clientSocket, User client, Room CreatedRoom)
{
	std::atomic<bool> keepSendStatus(true);
	std::future<void> StatusSending = std::async(std::launch::async, &Server::RoomStatusSender, this, clientSocket, CreatedRoom, &keepSendStatus);
	HandlerFactory* createMenuHandler = new CreateMenuRequestHandlerCreator();
	try
	{
		while (socketStillConnected(clientSocket))
		{
			if (keepSendStatus.load())
			{
				std::string request = ServerCommunicator::GetString(clientSocket);
				std::cout << request << std::endl;
				JsonRequestPacketDeserializer requestJson(request);
				json request_json = requestJson.Deserializer();
				if (request_json["status"] != 352)
					// 352 -> Other Users Joins Started Game
				{
					std::string response = createMenuHandler->FactoryMethod()->RequestResult(request_json, client.GetID(), CreatedRoom.GetID());
					ServerCommunicator::SendString(clientSocket, response);
					JsonRequestPacketDeserializer responseJson(response);
					json response_json = responseJson.Deserializer();
					if (response_json["status"] == 404)
					{
						MenuManager(clientSocket, client);
						break;
					}
					if (response_json["status"] == 400)
					{
						std::future<void> GameMechanic = std::async(std::launch::async, &Server::GameMake, this, CreatedRoom);
						GameManager(clientSocket, client, CreatedRoom);
						break;
					}
				}
			}
			else
			{
				Room UpdatedRoom = Rooms->GetRoom(CreatedRoom.GetID());
				if (UpdatedRoom.GetID() != -1)
				{
					GameManager(clientSocket, client, CreatedRoom);
					break;
				}
				else
				{
					MenuManager(clientSocket, client);
					break;
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}


void Server::GameStatusSender(SOCKET clientSocket, Room GameRoom, std::atomic<bool>* Flag)
{
	std::vector<std::string> currentQuestion = SQL->getQuestion(SQL->getCurrentQuestionID(GameRoom));
	std::string currentTimeLeft = std::to_string(SQL->getTime(GameRoom));
	std::string roomInfo = currentQuestion[0] + "|" + currentQuestion[1] + "|" + currentTimeLeft;
	ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 435, \"argument\": \"Status Update!|" + roomInfo + "\"}"));
	while (Flag->load())
	{
		Room UpdatedRoom = Rooms->GetRoom(GameRoom.GetID());
		std::vector<std::string> UpdatedQuestion = SQL->getQuestion(SQL->getCurrentQuestionID(GameRoom));
		std::string UpdatedTimeLeft = std::to_string(SQL->getTime(GameRoom));
		if (currentTimeLeft != UpdatedTimeLeft)
		{
			currentTimeLeft = UpdatedTimeLeft;
			currentQuestion = UpdatedQuestion;
			roomInfo = currentQuestion[0] + "|" + currentQuestion[1] + "|" + currentTimeLeft;
			ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 435, \"argument\": \"Status Update!|" + roomInfo + "\"}"));
			if (SQL->getCurrentQuestionID(GameRoom) == 10 && SQL->getTime(GameRoom) == 1)
			{
				ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 444, \"argument\": \"Game Finished!\"}"));
				Flag->store(false); //
			}
		}
	}
}


void Server::GameMake(Room GameRoom)
{
	std::cout << "Room \"" + GameRoom.GetName() + "\" Launched Game!" << std::endl;
	Rooms->startGame(GameRoom.GetID());
	SQL->createGame(GameRoom);
	for (int i = 1; i <= 10; i++)
	{
		SQL->setQuestion(GameRoom, i);
		SQL->setTimer(GameRoom);
	}
	Rooms->stopGame(GameRoom.GetID());
	SQL->closeGame(GameRoom);
	SQL->DeleteROOM(GameRoom);
}

void Server::GameManager(SOCKET clientSocket, User client, Room GameRoom)
{
	std::atomic<bool> keepSendStatus(true);
	std::future<void> StatusSending = std::async(std::launch::async, &Server::GameStatusSender, this, clientSocket, GameRoom, &keepSendStatus);
	HandlerFactory* GameRequestHandler = new GameRequestHandlerCreator();
	Rooms->SetPointsRoom(GameRoom.GetID(), client, 0);
	int averageTime = 1;
	std::vector<int> userStats = SQL->getUserStats(client);
	std::vector<int> userStatsBeforeChange = userStats;
	userStats[3]++;
	try
	{
		while (socketStillConnected(clientSocket))
		{
			std::string request = ServerCommunicator::GetString(clientSocket);
			JsonRequestPacketDeserializer requestJson(request);
			json request_json = requestJson.Deserializer();
			if (request_json["status"] != 600)
			{
				std::string response = GameRequestHandler->FactoryMethod()->RequestResult(request_json, client.GetID(), GameRoom.GetID());
				ServerCommunicator::SendString(clientSocket, response);
				JsonRequestPacketDeserializer responseJson(response);
				json response_json = responseJson.Deserializer();
				if (response_json["status"] == 502)
				{
					averageTime = (averageTime + (GameRoom.GetTime() - SQL->getTime(GameRoom))) / 2;
					userStats[0] = (userStats[0] + (GameRoom.GetTime() - SQL->getTime(GameRoom))) / 2;
					userStats[1]++;
					userStats[2]++;
					Rooms->SetPointsRoom(GameRoom.GetID(), client, userStats[1] - userStatsBeforeChange[1]);
					SQL->UpdateUsersStats(client, userStats);
				}
				if (response_json["status"] == 503)
				{
					userStats[2]++;
					SQL->UpdateUsersStats(client, userStats);
				}
			}
			else
			{
				std::string ResultsInfo = std::to_string(userStats[1] - userStatsBeforeChange[1]) + "|" + std::to_string(averageTime) + "|";
				std::vector<User*> xxx = Rooms->GetPointsRoom(GameRoom);
				for (auto i : xxx)
				{
					ResultsInfo = ResultsInfo + i->GetName() + " ";
				}
				ServerCommunicator::SendString(clientSocket, std::string("{\"status\": 601, \"argument\": \"" + ResultsInfo + "\"}"));
				MenuManager(clientSocket, client);
			}
		}
		std::cout << socketStillConnected(clientSocket) << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}