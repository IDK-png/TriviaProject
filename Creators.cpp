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
