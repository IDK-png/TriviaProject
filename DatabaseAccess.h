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
	//Rooms Managment 
	std::vector<Room*> GetRooms();
	void CreateROOM(Room room);
	void DeleteROOM(Room room);
	//User Managment
	void DeleteUser(Room, std::string);
	User* GetUSER(int id);
	bool CheckLogin(std::string NAME, std::string PASSWORD);
	User CreateUSER(std::string NAME, std::string PASSWORD);
	void AddUser(int, std::string);
	//Stats Managment
	std::vector<int> getUserStats(User);
	void UpdateUsersStats(User, std::vector<int>);
	//void Create
	//Question Managment
	std::vector <std::string> getQuestion(int id);
	//Game Managment
	void createGame(Room);
	void closeGame(Room);
	void setQuestion(Room, int);
	bool setTimer(Room);
	int getTime(Room);
	int answerCount(Room);
	int getCurrentQuestionID(Room);

	bool open();
	void close();
	void clear();

private:
	std::string DBname;
	sqlite3* DB;
};
