#ifndef ROOMMANAGER_HPP
#define ROOMMANAGER_HPP
#include "DatabaseAccess.h"

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
	std::string GetName();
	int GetID();
	bool GetState();
	std::string GetValueForDatabase();
	std::string GetUsers();
	
private:
	int ID; // Room Number
	std::string NAME; // Room Name
	int MAXCOUNT; // Max Player Count
	int TIME; // Max Time For Question
	bool ACTIVE;
	bool STATE; // false(0)-Waiting For Players, true(1)-In Game
	std::vector<User*> UserList; 
};

class RoomManager
{
public:
	Room createRoom(int id, std::string name, int max, int time);
	void deleteRoom(int ID);
	bool getRoomState(std::string Name);
	std::vector<Room*> getRooms();
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

inline std::string Room::GetName()
{
	return NAME;
}

inline bool Room::GetState()
{
	return STATE;
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

inline std::string Room::GetValueForDatabase()
{
	std::string returnString = "" + std::to_string(ID) + ", " + '"' + NAME + '"' + ", " + std::to_string(MAXCOUNT) + ", " + std::to_string(TIME) + ", " + std::to_string(ACTIVE) + ", " + '"' + GetUsers() + '"';
	return returnString;
}

inline Room RoomManager::createRoom(int id, std::string name, int max, int time)
{
	Room newRoom = Room(id, name, max, time);
	RoomsList.push_back(&newRoom);
	return newRoom;
}

inline bool RoomManager::getRoomState(std::string Name)
{
	for (Room* i : RoomsList)
	{
		if (i->GetName().compare(Name))
		{
			return i->GetState();
		}
	}
	return true;
}

inline void RoomManager::deleteRoom(int ID)
{
	int iterator = 0;
	for (auto i : RoomsList)
	{
		if (i->GetID() == ID)
		{
			RoomsList.erase(RoomsList.begin()+iterator);
			return; // Проверь если не синвалидил 
		}
		iterator++;
	}
}

inline std::vector<Room*> RoomManager::getRooms()
{
	return RoomsList;
}
#endif // ROOMMANAGER_HPP