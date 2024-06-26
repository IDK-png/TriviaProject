#ifndef ROOMMANAGER_HPP
#define ROOMMANAGER_HPP
#include "DatabaseAccess.h"
#include "ServerCommunicator.hpp"
#include <algorithm>
#include <map>

class User
{
public:
	User(std::string, int);
	int GetID();
	std::string GetName();
private:
	int ID;
	std::string NAME;
};

class Room
{
public:
	Room(int id, std::string name, int max, int time);
	Room(const Room&);
	std::string GetName();
	int GetID();
	bool GetState();
	int GetCount();
	int CurrentUsersCount();
	int GetTime();
	std::string GetValueForDatabase();
	std::string GetUsers();
	void StartGame();
	void StopGame();
	int GetAdminID();
	void UserAdd(User*);
	void RemoveUser(User*); 
	void SetPoints(User, int);
	std::vector<User*> SortedVecUserPoints();
private:
	int ID; // Room Number
	std::string NAME; // Room Name
	int MAXCOUNT; // Max Player Count
	int TIME; // Max Time For Question
	bool ACTIVE;
	bool STATE; // false(0)-Waiting For Players, true(1)-In Game
	std::vector<User*> UserList; 
	std::vector<int> UserPoints;
};

class RoomManager
{
public:
    RoomManager();
	Room createRoom(int id, std::string name, int max, int time);
	void deleteRoom(int ID);
	void UpdateRoomsList(std::vector<Room*>);
	bool getRoomState(std::string Name);
	Room GetRoom(int id);
	bool AddUserToRoom(int id, User userToAdd); 
	bool RemoveUserFromRoom(int id, User userToRemove);
	void updateRoom(Room*);
	std::vector<Room*> getRooms();
	void startGame(int ID);
	void stopGame(int ID);
	void SetPointsRoom(int, User, int);
	std::vector<User*> GetPointsRoom(Room);
private:
	std::vector<Room*> RoomsList;
};


inline User::User(std::string Name, int Id) : NAME(Name), ID(Id)
{

}

inline int User::GetID()
{
	return ID;
}
inline std::string User::GetName()
{
	return NAME;
}

inline Room::Room(int id, std::string name, int max, int time) : ID(id), NAME(name), MAXCOUNT(max), TIME(time), ACTIVE(true), STATE(false)
{
}

inline void Room::UserAdd(User* usr)
{
	UserList.push_back(usr);
	UserPoints.push_back(0);
}

inline void Room::RemoveUser(User* x)
{
	int iter = 0;
	for (auto i : UserList)
	{
		if (i->GetID() == x->GetID())
		{
			UserList.erase(UserList.begin() + iter);
			UserPoints.erase(UserPoints.begin() + iter);
		}
		iter++;
	}
}

inline int Room::GetAdminID()
{
	return UserList[0]->GetID();
}

inline Room::Room(const Room& room)
{
	ID = room.ID;
	NAME = room.NAME;
	MAXCOUNT = room.MAXCOUNT;
	TIME = room.TIME;
	ACTIVE = room.ACTIVE;
	STATE = room.STATE;
	UserList = room.UserList;
}

inline std::string Room::GetName()
{
	return NAME;
}

inline int Room::GetCount()
{
	return MAXCOUNT;
}

inline int Room::CurrentUsersCount()
{
	int i = 0;
	for (auto x : UserList)
	{
		i++;
	}
	return i;
}

inline bool Room::GetState()
{
	return STATE;
}

inline int Room::GetTime()
{
	return TIME;
}

inline int Room::GetID()
{
	return ID;
}
inline std::string Room::GetUsers()
{
	std::string returnString = "";
	for (User* i : UserList)
	{
		returnString += i->GetName() + ",";
	}
	return returnString;
}

inline void Room::StartGame()
{
	this->STATE = 1;
}

inline void Room::StopGame()
{
	this->STATE = 0;
}

inline void Room::SetPoints(User client, int Points)
{
	int iter = 0;
	for (auto i : UserList)
	{
		if (i->GetID() == client.GetID())
		{
			UserPoints[iter] = Points;
		}
		iter++;
	}
}

inline std::vector<User*> Room::SortedVecUserPoints() 
{
	std::multimap< int, User*> nameScoreMultimap;
	for (size_t i = 0; i < UserList.size(); ++i) {
		nameScoreMultimap.insert({ UserPoints[i] , UserList[i] });
	}

	std::vector<User*> sortedNames;
	for (const auto& pair : nameScoreMultimap) {
		sortedNames.push_back(pair.second);
		std::cout << pair.second->GetName() << std::endl;
	}

	return sortedNames;
}
inline RoomManager::RoomManager() : RoomsList()
{

}

inline Room RoomManager::GetRoom(int id)
{
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == id)
			return *i;
	}
	return Room(-1,"none",-1,-1); //Room(int id, std::string name, int max, int time);
}

inline void RoomManager::UpdateRoomsList(std::vector<Room*> x)
{
	RoomsList = x;
	std::cout << "HERE, Size: " << RoomsList.size() << std::endl;
	for (Room* a : RoomsList) // Find Room In RoomsList
	{
		std::cout << a->GetName() << std::endl;
	}
}

inline bool RoomManager::AddUserToRoom(int id, User userToAdd)
{
	for (Room* i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == id)
		{
			if (i->CurrentUsersCount() <= i->GetCount())
			{
				User* addUser = new User(userToAdd.GetName(), userToAdd.GetID());
				i->UserAdd(addUser);
				return true;
			}
		}
	}
	return false;
}

inline bool RoomManager::RemoveUserFromRoom(int id, User userToRemove)
{
	for (Room* i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == id)
		{
			i->RemoveUser(&userToRemove);
			return true;
		}
	}
	return false;
}

inline std::string Room::GetValueForDatabase()
{
	std::string returnString = "" + std::to_string(ID) + ", " + '"' + NAME + '"' + ", " + std::to_string(MAXCOUNT) + ", " + std::to_string(TIME) + ", " + std::to_string(ACTIVE) + ", " + '"' + GetUsers() + '"';
	return returnString;
}

inline Room RoomManager::createRoom(int id, std::string name, int max, int time)
{
	Room* newRoom = new Room(id, name, max, time);
	RoomsList.push_back(newRoom);
	return *newRoom;
}

inline bool RoomManager::getRoomState(std::string Name)
{
	for (Room* i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetName().compare(Name))
		{
			//std::cout << "Names: " << i->GetUsers() << std::endl;
			return i->GetState();
		}
	}
	return true;
}

inline void RoomManager::deleteRoom(int ID)
{
	int iterator = 0;
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == ID)
		{
			RoomsList.erase(RoomsList.begin()+iterator);
			return; // Проверь если не с инвалидил 
		}
		iterator++;
	}
}

inline void RoomManager::updateRoom(Room* room)
{
	int iterator = 0;
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == room->GetID() && i->GetName() == room->GetName())
		{
			RoomsList[iterator] = room;
			return; // Проверь если не с инвалидил 
		}
		iterator++;
	}
}

inline void RoomManager::startGame(int ID) // Room(int id, std::string name, int max, int time);
{
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == ID)
		{
			i->StartGame();
			return; 
		}
	}
}

inline void RoomManager::stopGame(int ID) // Room(int id, std::string name, int max, int time);
{
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == ID)
		{
			i->StopGame();
			return;
		}
	}
}

inline std::vector<User*> RoomManager::GetPointsRoom(Room GameRoom)
{
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == GameRoom.GetID())
		{
			return i->SortedVecUserPoints();
		}
	}
    return std::vector<User*>();
}

inline std::vector<Room*> RoomManager::getRooms()
{
	return RoomsList;
}
//void SetPointsRoom(int ID, User, int);
inline void RoomManager::SetPointsRoom(int ID, User client, int Points)
{
	for (auto i : RoomsList) // Find Room In RoomsList
	{
		if (i->GetID() == ID)
		{
			i->SetPoints(client, Points);
			i->SortedVecUserPoints();
			return;
		}
	}
}

#endif // ROOMMANAGER_HPP