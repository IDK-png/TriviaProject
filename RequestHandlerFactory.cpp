#include "RequestHandlerFactory.h"

std::string HandlerFactory::HandlerRequestResult(json Request, int clientID, int roomID) const
{
    RequestHandlerFactory* product = this->FactoryMethod();
    std::string result = product->RequestResult(Request, clientID, roomID);
    delete product;
    return result;
}
