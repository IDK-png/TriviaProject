﻿#include "DatabaseAccess.h"

DatabaseAccess::DatabaseAccess(std::string name) : DBname(name)
{

}

/*
Brief : sends command line to SQLite3 interface and returns nothing
Example of [const std::string command] : CREATE TABLE IF NOT EXISTS USERS (ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL);
Example of return : NULL
*/
void DatabaseAccess::sendToSql(const std::string command)
{
	char* errMsg = 0; int rc;

	rc = sqlite3_exec(DB, command.c_str(), 0, 0, &errMsg);

	sqlite3_free(errMsg);

}

/*
Brief : sends command line to SQLite3 interface and returns the response of the SQLite3 interface
Example of [const std::string command] : "SELECT ID FROM ALBUMS WHERE NAME=\"TEST\""
Example of return : "102"
*/
std::string DatabaseAccess::getFromSql(const std::string command)
{
	char* errMsg = 0; int rc;
	std::string result;

	sqlite3_stmt* stmt;
	rc = sqlite3_prepare_v2(DB, command.c_str(), -1, &stmt, NULL);

	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int columns = sqlite3_column_count(stmt);
		for (int i = 0; i < columns; i++) {
			result += reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
			result += " ";
		}
		result += "\n";
	}

	//std::cout << result << std::endl;
	sqlite3_finalize(stmt);

	return result;
}

/*
Brief : Returns the first available ID in range of n,n+100
Example of [std::string TABLE][int start=0] : "ALBUMS", 100
Example of return : 101
*/
int DatabaseAccess::IdCheck(std::string TABLE, int start=0)
{
	for (int i = 0; i < start+1000; i++)
	{
		if (getFromSql("SELECT * FROM " + TABLE + " WHERE ID= " + std::to_string(i) + ';').length() == 0)
		{
			return i;
		}
	}
	return -1;
}

/*
Brief : Returns the ID column value of found record by NAME
Example of [std::string TABLE][std::string NAME] : "ALBUMS", "TESTalbum"
Example of return : 102
*/
int DatabaseAccess::getIndexByName(std::string TABLE, std::string NAME)
{
	std::string result = getFromSql("SELECT ID FROM " + TABLE + " WHERE NAME= " + '"' + NAME + '"' + ';');
	if (result.size() > 0)
	{
		return std::stoi(result);
	}
	else
	{
		return -1;
	}
}

/*
Brief : Returns the NAME column value of found record by ID
Example of [std::string TABLE][std::string ID] : "ALBUMS", "102"
Example of return : "TESTalbum"
*/
std::string DatabaseAccess::getNameByIndex(std::string TABLE, std::string ID)
{
	std::string result = getFromSql("SELECT NAME FROM " + TABLE + " WHERE ID= " + ID);
	if (result.size() > 0)
	{
		return result;
	}
	else
	{
		return "NONE";
	}
}

User DatabaseAccess::CreateUSER(std::string NAME, std::string PASSWORD)
{
	User Client = User(NAME, IdCheck("USERS", 0));
	std::string name = '"' + Client.GetName() + '"';
	std::string USERSquery = "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (" + std::to_string(Client.GetID()) + ',' + name + ',' + '"' + PASSWORD + '"' + ");";
	sendToSql(USERSquery);
	return Client;
}

void DatabaseAccess::CreateROOM(Room room)
{
	std::string ALBUMSquery = "INSERT INTO USERS (ID, NAME, MAXCOUNT, TIME, ACTIVE, USER_LIST) VALUES (" + room.GetValueForDatabase() + ");";
	sendToSql(ALBUMSquery);
}

/*
Brief : Opens database
Example of [no arguments] : NULL
Example of return : NULL
*/
bool DatabaseAccess::open()
{
	int rc; char* errMsg = 0;
	std::string name = "trivia.db";

	this->DBname = name;
	rc = sqlite3_open(DBname.c_str(),&DB);

	if (rc)
	{
		fprintf(stderr, "Can't open database: %s\nPlease restart the program!.\n", sqlite3_errmsg(DB));
		return false;
	}

	std::string USERSquery = "CREATE TABLE IF NOT EXISTS USERS (ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, PASSWORD TEXT NOT NULL);";
	std::string ROOMSquery = "CREATE TABLE IF NOT EXISTS ROOMS (ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, MAXCOUNT INT NOT NULL, TIME INT NOT NULL, ACTIVE BOOLEAN, USER_LIST TEXT NOT NULL);";
	std::string STATSquery = "CREATE TABLE IF NOT EXISTS STATS (ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, AVG_TIME INT NOT NULL, CORRECT INT NOT NULL, TOTAL INT NOT NULL, GAMES INT NOT NULL);";
	std::string QUESTIONSquery = "CREATE TABLE IF NOT EXISTS QUESTIONS (ID INT PRIMARY KEY NOT NULL, QUESTION TEXT NOT NULL, ANSWERS TEXT NOT NULL, RIGHT_ANSWER INT NOT NULL);";
	rc = sqlite3_exec(DB, USERSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, ROOMSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, STATSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, QUESTIONSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);

	return true;
}

/*
Brief : Closes current database
Example of [no arguments] : NULL
Example of return : NULL
*/
void DatabaseAccess::close()
{
	sqlite3_close(DB);
	if (DB != SQLITE_OK)
	{
		fprintf(stderr, "Database closed successfully!\n");
	}
}

/*
Brief : Deletes all records in database
Example of [no arguments] : NULL
Example of return : NULL
*/
void DatabaseAccess::clear()
{
	char* errMsg = 0; int rc = 0;
	const char* ALBUMSquery = "DELETE FROM USERS"; // Замени тут TableName на другое, не позорься 
	const char* USERquery = "DELETE FROM ROOMS";

	rc = sqlite3_exec(DB, ALBUMSquery, 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, USERquery, 0, 0, &errMsg);
	sqlite3_free(errMsg);
}