#include "DatabaseAccess.h"

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

std::vector<Room*> DatabaseAccess::GetRooms()
{
	sqlite3_stmt* stmt;
	int rc;
	const char* sql = "SELECT ID, NAME, MAXCOUNT, TIME, ACTIVE, USER_LIST FROM ROOMS;";
	std::vector<Room*> RoomsList; 
	rc = sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL);

	//Room(int id, std::string name, int max, int time) 
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0); 
		std::string name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))); 
		int max = sqlite3_column_int(stmt, 2);
		int time = sqlite3_column_int(stmt, 3);
		Room* Curr = new Room(id, name, max, time);
		//bool active = sqlite3_column_int(stmt, 4) != 0;
		//std::string users = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
		RoomsList.push_back(Curr);
	}

	return RoomsList;
}

User* DatabaseAccess::GetUSER(int id)
{
	return new User(getNameByIndex("USERS",std::to_string(id)),id);
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

/*
Brief : Returns if the given login information is correct
Example of [std::string NAME][std::string PASSWORD] : "User1", "1337"
Example of return : True
*/
bool DatabaseAccess::CheckLogin(std::string NAME, std::string PASSWORD)
{
	std::string query = "SELECT * FROM USERS WHERE NAME = '" + NAME + "' AND PASSWORD = '" + PASSWORD + "'";
	if (getFromSql(query).length() > 2)
	{
		return true;
	}
	return false;
}

/*
Brief : Creates new user in database and returns the user as User object
Example of [std::string NAME][std::string PASSWORD] : "User1", "1337"
Example of return : User("User1","1337)
*/
User DatabaseAccess::CreateUSER(std::string NAME, std::string PASSWORD)
{
	User Client = User(NAME, IdCheck("USERS", 0));
	std::string name = '"' + Client.GetName() + '"';
	std::string USERSquery = "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (" + std::to_string(Client.GetID()) + ',' + name + ',' + '"' + PASSWORD + '"' + ");";
	std::string STATSquery = "INSERT INTO STATS (ID, NAME, AVG_TIME, CORRECT, TOTAL, GAMES) VALUES (" + std::to_string(Client.GetID()) + ',' + name + ',' + "0,0,0,0);";
	sendToSql(USERSquery);
	sendToSql(STATSquery);
	return Client;
}

/*
Brief : Creates new room in database
Example of [Room room] : {Some Room Object}
Example of return : NULL
*/
void DatabaseAccess::CreateROOM(Room room)
{
	std::string RoomsQuery = "INSERT INTO ROOMS (ID, NAME, MAXCOUNT, TIME, ACTIVE, USER_LIST) VALUES (" + room.GetValueForDatabase() + ");";
	sendToSql(RoomsQuery);
}

/*
Brief : Adds user to specific room in SQL
Example of [Room room] : {Some Room Object}
Example of return : NULL
*/
void DatabaseAccess::AddUser(int id, std::string user)
{
	std::string CurrentUsers = '"' + getFromSql("SELECT USER_LIST FROM ROOMS WHERE ID=" + std::to_string(id) + ";") + user + ',' + '"';
	std::string RoomsQuery = "UPDATE ROOMS SET USER_LIST=" + CurrentUsers + " WHERE ID =" + std::to_string(id) + ";";
	//std::cout << RoomsQuery << std::endl;
	sendToSql(RoomsQuery);
}

std::vector<int> DatabaseAccess::getUserStats(User client)
{
	std::string AverageTimeQuery = "SELECT AVG_TIME FROM STATS WHERE ID = " + std::to_string(client.GetID());
	std::string CorrectQuery = "SELECT CORRECT FROM STATS WHERE ID = " + std::to_string(client.GetID());
	std::string TotalQuery = "SELECT TOTAL FROM STATS WHERE ID = " + std::to_string(client.GetID());
	std::string GamesQuery = "SELECT GAMES FROM STATS WHERE ID = " + std::to_string(client.GetID());
	std::vector<int> ReturnVector;
	ReturnVector.push_back(std::stoi(getFromSql(AverageTimeQuery)));
	ReturnVector.push_back(std::stoi(getFromSql(CorrectQuery)));
	ReturnVector.push_back(std::stoi(getFromSql(TotalQuery)));
	ReturnVector.push_back(std::stoi(getFromSql(GamesQuery)));
	return ReturnVector;
}

void DatabaseAccess::UpdateUsersStats(User client, std::vector<int> updatedVector)
{
	std::string StatsQuery = "UPDATE STATS SET AVG_TIME=" + std::to_string(updatedVector[0]) + ", CORRECT=" + std::to_string(updatedVector[1]) +", TOTAL=" + std::to_string(updatedVector[2]) +", GAMES=" + std::to_string(updatedVector[3]) + " WHERE ID=" + std::to_string(client.GetID()) + ";";
	std::cout << StatsQuery << std::endl;
	sendToSql(StatsQuery);
}

std::vector<std::string> DatabaseAccess::getQuestion(int id)
{
	std::string QuestionQuery = "SELECT QUESTION FROM QUESTIONS WHERE ID = " + std::to_string(id);
	std::string AnswersQuery = "SELECT ANSWERS FROM QUESTIONS WHERE ID = " + std::to_string(id);
	std::string RightAnswerNumQuery = "SELECT RIGHT_ANSWER FROM QUESTIONS WHERE ID = " + std::to_string(id);
	std::vector<std::string> ReturnVector;
	ReturnVector.push_back(getFromSql(QuestionQuery));
	ReturnVector.push_back(getFromSql(AnswersQuery));
	ReturnVector.push_back(getFromSql(RightAnswerNumQuery));
	return ReturnVector;
}

void DatabaseAccess::createGame(Room GameRoom)
{
	std::string RoomsQuery = "INSERT INTO GAMES (ID, CURRENT_QUESTION_ID, TIME_LEFT, QUESTIONS_ANSWERD, USERS_ANSWERD) VALUES (" + std::to_string(GameRoom.GetID()) + ",0,0,0,0" + ");";
	sendToSql(RoomsQuery);
}

void DatabaseAccess::closeGame(Room GameRoom)
{
	std::string CloseQuery = "DELETE FROM GAMES WHERE ID = " + std::to_string(GameRoom.GetID()) + ";";
	sendToSql(CloseQuery);
}

void DatabaseAccess::setQuestion(Room GameRoom, int QuestionID)
{
	std::string RoomsQuery = "UPDATE GAMES SET CURRENT_QUESTION_ID=" + std::to_string(QuestionID) + " WHERE ID = " + std::to_string(GameRoom.GetID()) + ";";
	sendToSql(RoomsQuery);
}

bool DatabaseAccess::setTimer(Room GameRoom)
{
	for (int i = GameRoom.GetTime(); i > 0; i--)
	{
		std::string TimeQuery = "UPDATE GAMES SET TIME_LEFT=" + std::to_string(i) + " WHERE ID =" + std::to_string(GameRoom.GetID()) + ";";
		sendToSql(TimeQuery);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return true;
}

int DatabaseAccess::getTime(Room GameRoom)
{
	std::string TimeQuery = "SELECT TIME_LEFT FROM GAMES WHERE ID = " + std::to_string(GameRoom.GetID());
	return stoi(getFromSql(TimeQuery));
}

int DatabaseAccess::answerCount(Room GameRoom)
{
	std::string answerCountQuery = "SELECT USERS_ANSWERD FROM GAMES WHERE ID = " + std::to_string(GameRoom.GetID());
	return stoi(getFromSql(answerCountQuery));
}

int DatabaseAccess::getCurrentQuestionID(Room GameRoom)
{
	std::string CurrentQuestionQuery = "SELECT CURRENT_QUESTION_ID FROM GAMES WHERE ID = " + std::to_string(GameRoom.GetID());
	try
	{
		return stoi(getFromSql(CurrentQuestionQuery));
	}
	catch (const std::exception& e)
	{
		return -1;
	}
}

void DatabaseAccess::DeleteUser(Room ROOM, std::string user)
{
	//std::string CurrentUsers = 
	//std::string RoomsQuery = "UPDATE ROOMS SET USER_LIST=" + CurrentUsers + " WHERE ID =" + std::to_string(id) + ";";
	////std::cout << RoomsQuery << std::endl;
	//sendToSql(RoomsQuery);
}
void DatabaseAccess::DeleteROOM(Room room)
{
	std::string RoomsQuery = "DELETE FROM ROOMS WHERE ID=" + std::to_string(room.GetID()) + ";";
	sendToSql(RoomsQuery);
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
	std::string GAMESquery = "CREATE TABLE IF NOT EXISTS GAMES (ID INT PRIMARY KEY NOT NULL, CURRENT_QUESTION_ID INT NOT NULL, TIME_LEFT INT NOT NULL, QUESTIONS_ANSWERD INT NOT NULL, USERS_ANSWERD INT NOT NULL);";
	//std::string QUESTIONSaddQuery = "INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('1', 'Which planet is known as the Red Planet?', 'Earth|Venus|Mars|Jupiter', '3'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('2', 'What is the largest mammal in the world?', 'Elephant|Blue Whale|Giraffe|Great White Shark', '2'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('3', 'Who wrote the novel \"1984\"?', 'Aldous Huxley|George Orwell|Ernest Hemingway|F. Scott Fitzgerald', '2'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('4', 'What is the main ingredient in guacamole?', 'Tomato|Avocado|Onion|Pepper', '2'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('5', 'Which element has the atomic number 1?', 'Helium|Oxygen|Hydrogen|Nitrogen', '3'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('6', 'What is the hardest natural substance on Earth?', 'Gold|Iron|Diamond|Quartz', '3'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('7', 'Who developed the theory of relativity?', 'Isaac Newton|Albert Einstein|Galileo Galilei|Nikola Tesla', '2'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('8', 'What is the largest ocean on Earth?', 'Atlantic Ocean|Indian Ocean|Arctic Ocean|Pacific Ocean', '4'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('9', 'Who is known as the Father of Computers?', 'Charles Babbage|Alan Turing|John von Neumann|Steve Jobs', '1'); INSERT INTO \"QUESTIONS\" (\"ID\", \"QUESTION\", \"ANSWERS\", \"RIGHT_ANSWER\") VALUES ('10', 'Which gas do plants absorb from the atmosphere?', 'Oxygen|Carbon Dioxide|Nitrogen|Hydrogen', '2');";

	rc = sqlite3_exec(DB, USERSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, ROOMSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, STATSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, QUESTIONSquery.c_str(), 0, 0, &errMsg);
	sqlite3_free(errMsg);
	//rc = sqlite3_exec(DB, QUESTIONSaddQuery.c_str(), 0, 0, &errMsg);
	//sqlite3_free(errMsg);
	rc = sqlite3_exec(DB, GAMESquery.c_str(), 0, 0, &errMsg);
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