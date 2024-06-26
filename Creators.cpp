#include "Creators.h"

RequestHandlerFactory* LoginRequestHandlerCreator::FactoryMethod() const
{
	return new LoginRequestHandler();
}

RequestHandlerFactory* MenuRequestHandlerCreator::FactoryMethod() const
{
	return new MenuRequestHandler();
}

RequestHandlerFactory* CreateMenuRequestHandlerCreator::FactoryMethod() const
{
	return new CreateMenuRequestHandler();
}

RequestHandlerFactory* GameRequestHandlerCreator::FactoryMethod() const
{
	return new GameRequestHandler();
}
