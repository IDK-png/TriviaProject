#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include "RoomManager.hpp"
#include "sqlite3/sqlite3.h"

class DatabaseAccess
{
public:
	DatabaseAccess(std::string);
	void sendToSql(const std::string command);
	std::string getFromSql(const std::string command);
	int IdCheck(std::string TABLE, int start);
	int getIndexByName(std::string TABLE, std::string NAME);
	std::string getNameByIndex(std::string TABLE, std::string ID);

	User CreateUSER(std::string NAME, std::string PASSWORD);
	void CreateROOM(Room room);



	bool open();
	void close();
	void clear();

private:
	std::string DBname;
	sqlite3* DB;
};
