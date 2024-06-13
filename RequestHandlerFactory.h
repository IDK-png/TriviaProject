#pragma once
#include "ServerJSON.hpp"

class RequestHandlerFactory
{
public:
    virtual std::string RequestResult(json Request, int clientID, int roomID) = 0;
    virtual ~RequestHandlerFactory() {}
};

class HandlerFactory {
public:
    virtual ~HandlerFactory() {};
    virtual RequestHandlerFactory* FactoryMethod() const = 0;

    std::string HandlerRequestResult(json Request, int clientID, int roomID) const;
};
