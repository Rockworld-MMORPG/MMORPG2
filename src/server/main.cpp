#include "NetworkManager.hpp"
#include "SFML/Window/WindowStyle.hpp"
#include "common/message.hpp"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <unordered_map>

struct Player
{
	sf::Vector2f position{0.0F, 0.0F};
};

using PlayerMap = std::unordered_map<Server::ClientID, Player>;

auto parseTCPMessages(Server::NetworkManager& networkManager, PlayerMap& playerMap, bool& shouldExit) -> void
{
	std::optional<Server::Message> message;
	while ((message = networkManager.getNextTCPMessage()).has_value())
	{
		auto messageType = Common::Message::None;
		message->data >> messageType;

		switch (messageType)
		{
			case Common::Message::Connect:
			{
				std::uint16_t udpPort = 0;
				message->data >> udpPort;

				networkManager.setClientUdpPort(message->clientID, udpPort);

				if (!playerMap.contains(message->clientID))
				{
					playerMap.emplace(message->clientID, Player());
					Server::Message outboundMessage{};
					outboundMessage.clientID = message->clientID;
					outboundMessage.data << static_cast<std::uint32_t>(Common::Message::Position) << 0.0F << 0.0F;
					networkManager.pushUDPMessage(std::move(outboundMessage));
				}
			}
			break;
			case Common::Message::Disconnect:
			{
				std::cout << "Received disconnect from client " << message->clientID << "\n";
				auto iterator = playerMap.find(message->clientID);
				if (iterator != playerMap.end())
				{
					playerMap.erase(iterator);
				}
				networkManager.closeConnection(message->clientID);
			}
			break;
			case Common::Message::Terminate:
			{
				shouldExit = true;
			}
			default:
				break;
		}
	}
}

auto parseUDPMessages(Server::NetworkManager& networkManager, PlayerMap& playerMap, const float deltaTime) -> void
{
	std::optional<Server::Message> message;
	while ((message = networkManager.getNextUDPMessage()).has_value())
	{
		auto messageType = Common::Message::None;
		message->data >> messageType;

		switch (messageType)
		{
			case Common::Message::Movement:
			{
				sf::Vector2f movementDelta{0.0F, 0.0F};
				message->data >> movementDelta.x >> movementDelta.y;

				auto iterator = playerMap.find(message->clientID);
				if (iterator == playerMap.end())
				{
					playerMap.emplace(message->clientID, Player());
				}
				auto&& position = playerMap[message->clientID].position;
				position += movementDelta * deltaTime;

				Server::Message outboundMessage{};
				outboundMessage.clientID = message->clientID;
				outboundMessage.data << static_cast<std::uint32_t>(Common::Message::Position) << position.x << position.y;
				networkManager.pushUDPMessage(std::move(outboundMessage));
			}
			break;
			default:
				break;
		}
	}
}

auto main() -> int
{
	Server::NetworkManager networkManager;
	PlayerMap playerMap;

	// Begin main loop
	sf::Clock clock;
	clock.restart();
	bool shouldExit = false;

	while (!shouldExit)
	{
		float deltaTime = clock.restart().asSeconds();
		networkManager.update();

		parseTCPMessages(networkManager, playerMap, shouldExit);
		parseUDPMessages(networkManager, playerMap, deltaTime);
	}

	return 0;
}