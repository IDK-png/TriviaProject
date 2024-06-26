#pragma once
#include "RequestHandlerFactory.h"
#include "ServerCommunicator.hpp"
#include "DatabaseAccess.h"
#include "RoomManager.hpp"

class DatabaseAccess;
class RoomManager;

class DataManage {
public:
    // Deleting copy,assignment to prevent copy creations
    DataManage(const DataManage&) = delete;
    DataManage& operator=(const DataManage&) = delete;

    // Getting singleton Instance 
    static DataManage* getInstance(DatabaseAccess* database = nullptr, RoomManager* rooms = nullptr);

    // Getters
    DatabaseAccess* getDatabase() const;

    RoomManager* getRooms() const;

protected:
    // Приватный конструктор с параметрами
    DataManage(DatabaseAccess* database, RoomManager* rooms) : SQL(database), Rooms(rooms) { }

    ~DataManage() { }

    DatabaseAccess* SQL;
    RoomManager* Rooms;

    static DataManage* instance;
};

class LoginRequestHandler : public RequestHandlerFactory {
public:
    std::string RequestResult(json Request, int clientID, int roomID) override;
};

class MenuRequestHandler : public RequestHandlerFactory {
public:
    std::string RequestResult(json Request, int clientID, int roomID) override;
};

class CreateMenuRequestHandler : public RequestHandlerFactory {
public:
    std::string RequestResult(json Request, int clientID, int roomID) override;
};

class GameRequestHandler : public RequestHandlerFactory {
public:
    std::string RequestResult(json Request, int clientID, int roomID) override;
};