#ifndef SERVERCOM_HPP
#define SERVERCOM_HPP
#include "Server.h"
#include "ServerJSON.hpp"

class ServerCommunicator
{
public:
	static void SendString(const SOCKET sc, const std::string message);
	static void SendJSON(const SOCKET sc, const json message);
	static std::string GetString(const SOCKET sc);
	static std::vector<std::string> splitFunc(std::string Text, std::string delimiter);
};



/*
Brief : Returns splited text by delimiter in vector
Example of [std::string Text][std::string delimiter] : "TEST TEXT", " "
Example of return : std::vector<std::string>{"TEST", "TEXT"}
*/
inline std::vector<std::string> ServerCommunicator::splitFunc(std::string Text, std::string delimiter)
{
	std::vector<std::string> tokens;

	size_t pos = 0;
	while ((pos = Text.find(delimiter)) != std::string::npos) {
		std::string token = Text.substr(0, pos);
		tokens.push_back(token);
		Text.erase(0, pos + delimiter.length());
	}
	tokens.push_back(Text);

	return tokens;
}

inline void ServerCommunicator::SendString(const SOCKET sc, const std::string message)
{
	const char* data = message.c_str();

	if (send(sc, data, message.size(), 0) == INVALID_SOCKET)
	{
		throw std::exception("Error while sending message to client");
	}
}

inline void ServerCommunicator::SendJSON(const SOCKET sc, const json message)
{
	SendString(sc, message.dump());
}

inline std::string ServerCommunicator::GetString(const SOCKET sc)
{
	char m[264] = { 0 };
	recv(sc, m, sizeof(m), 0);
	return std::string(m);
}

#endif // SERVERCOM_HPP