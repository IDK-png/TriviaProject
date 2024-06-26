#include "Products.h"

DataManage* DataManage::instance = nullptr;
DataManage* DataManage::getInstance(DatabaseAccess* database, RoomManager* rooms)
{
    if (instance == nullptr) {
        instance = new DataManage(database, rooms);
    }
    return instance;
}

DatabaseAccess* DataManage::getDatabase() const
{
    return SQL;
}

RoomManager* DataManage::getRooms() const
{
    return Rooms;
}

std::string LoginRequestHandler::RequestResult(json Request, int clientID, int roomID)
{
    DataManage* Data = DataManage::getInstance();
    if (JSONMethods::CheckProtocol(Request))
    {
        std::vector<std::string> arg = ServerCommunicator::splitFunc(Request["argument"], "|");
        if (Request["status"] == 101)
        {
            if (Data->getDatabase()->getIndexByName("USERS", arg[0]) != -1)
            {
                if (Data->getDatabase()->CheckLogin(arg[0], arg[1]))
                { // If the Username And Password correct
                    return "{\"status\": 111, \"argument\": \"User Logined!\"}";
                }
                else
                { // If the Username in database but password not correct
                    return "{\"status\": 113, \"argument\": \"Wrong Password!\"}";
                }
            }
            else
            {
                return "{\"status\": 141, \"argument\": \"No User Found\"}";
            }
        }
        if (Request["status"] == 102)
        {
            if (Data->getDatabase()->getIndexByName("USERS", arg[0]) != -1)
            {   // If username in database and client trying to create another one 
                return "{\"status\": 142, \"argument\": \"User Exists Already!\"}";
            }
            else
            {   // If there is no username in database then create 
                Data->getDatabase()->CreateUSER(arg[0], arg[1]);
                return "{\"status\": 112, \"argument\": \"User Created!\"}";
            }
        }
    }
    return "{\"status\": 144, \"argument\": \"Bad Request!\"}";
}

std::string MenuRequestHandler::RequestResult(json Request, int clientID, int roomID)
{
	DataManage* Data = DataManage::getInstance();
	RoomManager* RoomsData = Data->getRooms();
	DatabaseAccess* SQL = Data->getDatabase();
	User* client = SQL->GetUSER(clientID);
	if (Request["status"] == 200)
	{
		std::vector<int> userStats = SQL->getUserStats(*client);
		std::string StatsInfo = std::to_string(userStats[0]) + "|" + std::to_string(userStats[1]) + "|" + std::to_string(userStats[2]) + "|" + std::to_string(userStats[3]);
		return "{\"status\": 177, \"argument\": \"" + StatsInfo + "\"}";
	}
	if (Request["status"] == 201) // If User Creates Room
	{
		std::vector<std::string> secondMessage = ServerCommunicator::splitFunc(Request["argument"], "|"); // Splits message
		try
		{
			Room newCreatedRoom = RoomsData->createRoom(SQL->IdCheck("ROOMS", 0), secondMessage[0], stoi(secondMessage[1]), stoi(secondMessage[2]));
			User* userToRoom = new User(client->GetName(), client->GetID());
			if (RoomsData->AddUserToRoom(newCreatedRoom.GetID(), *client))
			{
				SQL->CreateROOM(newCreatedRoom); // Create Room In DB
				SQL->AddUser(newCreatedRoom.GetID(), client->GetName()); // Add User In DB
				return "{\"status\": 333, \"argument\": \"Room Created!\"}";
			}
		}
		catch (const std::exception& e)
		{
			return "{\"status\": 314, \"argument\": \"Not Correct Info!\"}";
		}
	}
	if (Request["status"] == 211) // If User Joins Room
	{
		std::vector<std::string> secondMessage = ServerCommunicator::splitFunc(Request["argument"], "|"); // Splits message
		try
		{
			Room FoundRoom = RoomsData->GetRoom(stoi(secondMessage[1])); // Find Room The User Want To Join
			if (FoundRoom.GetName() == secondMessage[0] && !FoundRoom.GetState())
			{
				if (RoomsData->AddUserToRoom(stoi(secondMessage[1]), *client))
				{
					SQL->AddUser(stoi(secondMessage[1]), client->GetName());
					return "{\"status\": 334, \"argument\": \"Joined To Room!\"}";
				}
				else
				{
					return "{\"status\": 314, \"argument\": \"Room Is Full!\"}";
				}
			}
			else
			{
				return "{\"status\": 314, \"argument\": \"Not Correct Info!\"}";
			}
		}
		catch (const std::exception& e)
		{
			return "{\"status\": 314, \"argument\": \"Not Correct Info!\"}";
		}
	}
}

std::string CreateMenuRequestHandler::RequestResult(json Request, int clientID, int roomID)
{
	DataManage* Data = DataManage::getInstance();
	RoomManager* RoomsData = Data->getRooms();
	DatabaseAccess* SQL = Data->getDatabase();
	User* client = SQL->GetUSER(clientID);
	Room clientRoom = RoomsData->GetRoom(roomID);
	if (Request["status"] == 345)
	{
		if (RoomsData->GetRoom(clientRoom.GetID()).GetAdminID() == client->GetID()) // If The User Who Leaved Is Admin
		{
			//SQL->DeleteROOM(clientRoom);
			RoomsData->deleteRoom(clientRoom.GetID());
			return "{\"status\": 404, \"argument\": \"Left Room!\"}";
		}
		else
		{
			// SQL Remove User
			RoomsData->RemoveUserFromRoom(clientRoom.GetID(), *client);
			return "{\"status\": 404, \"argument\": \"Left Room!\"}";
		}
	}
	if (Request["status"] == 346)
	{
		if (RoomsData->GetRoom(clientRoom.GetID()).GetAdminID() == client->GetID())
		{
			RoomsData->startGame(clientRoom.GetID());
			return "{\"status\": 400, \"argument\": \"Game Starting!\"}";
		}
		else
		{
			return "{\"status\": 401, \"argument\": \"Not Admin, Game Not Starting!\"}";
		}
	}
}

std::string GameRequestHandler::RequestResult(json Request, int clientID, int roomID)
{
	DataManage* Data = DataManage::getInstance();
	RoomManager* RoomsData = Data->getRooms();
	DatabaseAccess* SQL = Data->getDatabase();
	User* client = SQL->GetUSER(clientID);
	Room clientRoom = RoomsData->GetRoom(roomID);
	if (Request["status"] == 501)
	{
		std::vector <std::string> currentQuestion = SQL->getQuestion(SQL->getCurrentQuestionID(clientRoom));
		if (std::stoi(currentQuestion[2]) == std::stoi(std::string(Request["argument"])))
		{
			return "{\"status\": 502, \"argument\": \"Right Answer!\"}";
		}
		else
		{
			return "{\"status\": 503, \"argument\": \"Wrong Answer!\"}";
		}
	}
}
