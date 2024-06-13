#pragma once
#include "RequestHandlerFactory.h"
#include "Products.h"
class LoginRequestHandlerCreator : public HandlerFactory
{
public:
    RequestHandlerFactory* FactoryMethod() const override;
};

class MenuRequestHandlerCreator : public HandlerFactory
{
public:
    RequestHandlerFactory* FactoryMethod() const override;
};

class CreateMenuRequestHandlerCreator : public HandlerFactory
{
public:
    RequestHandlerFactory* FactoryMethod() const override;
};
