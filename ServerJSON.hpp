#ifndef SERVERJSON_HPP
#define SERVERJSON_HPP

#include "./nlohmann/json.hpp"
using json = nlohmann::json;

struct JSONResponse {
	int status;
	std::string message;
};

class JsonResponsePacketSerializer
{
public:
	//Constructors
	//_______________________________________________________________
	// Default constructor
	JsonResponsePacketSerializer();
	// Constructor that initializes the ResponseStruct with given arguments
	JsonResponsePacketSerializer(int status, std::string message);
	//_______________________________________________________________
	
	//Methods
	//_______________________________________________________________
	json Serializer();
	//_______________________________________________________________
private:
	JSONResponse ResponseStruct;
};

inline JsonResponsePacketSerializer::JsonResponsePacketSerializer() : ResponseStruct{ 0 , "" } { }

inline JsonResponsePacketSerializer::JsonResponsePacketSerializer(int status, std::string message) : ResponseStruct{ status , message } { }

inline json JsonResponsePacketSerializer::Serializer()
{
	return json{ {"status", ResponseStruct.status},{"message", ResponseStruct.message} };
}
//_______________________________________________________________
class JsonRequestPacketDeserializer
{
public:
	//Constructors
	//_______________________________________________________________
	// Default constructor
	JsonRequestPacketDeserializer();
	// Constructor that initializes the RequestMessage with given argument
	JsonRequestPacketDeserializer(std::string message);
	//_______________________________________________________________

	//Methods
	//_______________________________________________________________
	json Deserializer();
	//_______________________________________________________________
private:
	std::string RequestMessage;
};

inline JsonRequestPacketDeserializer::JsonRequestPacketDeserializer() : RequestMessage("") { }

inline JsonRequestPacketDeserializer::JsonRequestPacketDeserializer(std::string message) : RequestMessage(message) { }

inline json JsonRequestPacketDeserializer::Deserializer()
{
	return json::parse(RequestMessage);
}

class JSONMethods
{
public:
	static bool CheckProtocol(json);
};

inline bool JSONMethods::CheckProtocol(json object) {
	if (object.contains("status") && object.contains("argument"))
	{
		return true;
	}
	return false;
}
#endif // SERVERJSON_HPP