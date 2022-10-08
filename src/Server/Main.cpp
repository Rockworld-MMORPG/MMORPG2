#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageHeader.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "EntityManager.hpp"
#include "Game/PlayerManager.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/Network/Packet.hpp"
#include "Version.hpp"
#include "spdlog/sinks/stdout_sinks.h"
#include <SFML/System/Clock.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <thread>

using namespace Server;

auto parseTCPMessage(Common::Network::Message& message, bool& shouldExit) -> void
{
	switch (message.header.type)
	{
		case Common::Network::MessageType::None:
		{
		}
		break;
		case Common::Network::MessageType::Connect:
		{
			std::uint16_t udpPort = 0;
			message.data >> udpPort;
			g_networkManager.setClientUdpPort(message.header.clientID, udpPort);
		}
		break;
		case Common::Network::MessageType::Disconnect:
		{
			g_playerManager.destroyPlayer(message.header.clientID);
			g_networkManager.markForDisconnect(message.header.clientID);

			auto data = Common::Network::MessageData();
			g_networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, Common::Network::ClientID(-1), data);
		}
		break;
		case Common::Network::MessageType::Terminate:
		{
			spdlog::info("Server has been sent a terminate command");
			shouldExit = true;

			auto data = Common::Network::MessageData();
			g_networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, Common::Network::ClientID(-1), data);
		}
		break;
		default:
			spdlog::warn("Received a non-TCP packet over TCP ({:X})", static_cast<std::uint8_t>(message.header.type));
	}
}

auto parseUDPMessage(Common::Network::Message& message, const float deltaTime) -> void
{
	switch (message.header.type)
	{
		case Common::Network::MessageType::CreateEntity:
		{
			g_playerManager.createPlayer(message.header.clientID);

			auto data = Common::Network::MessageData();
			data << message.header.clientID;
			g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, Common::Network::ClientID(-1), data);
		}
		break;
		case Common::Network::MessageType::Movement:
		{
			sf::Vector2f movementDelta{0.0F, 0.0F};
			message.data >> movementDelta.x >> movementDelta.y;

			auto optPlayer = g_playerManager.getPlayer(message.header.clientID);
			if (optPlayer.has_value())
			{
				const float PLAYER_MOVEMENT_SPEED = 200.0F;
				optPlayer.value().get().position += movementDelta * deltaTime * PLAYER_MOVEMENT_SPEED;
			}
		}
		break;
		default:
			break;
	}
}

auto parseMessages(const float deltaTime, bool& shouldExit) -> void
{
	auto messages = g_networkManager.getMessages();
	for (auto& message : messages)
	{
		switch (message.header.protocol)
		{
			case Common::Network::Protocol::TCP:
				parseTCPMessage(message, shouldExit);
				break;
			case Common::Network::Protocol::UDP:
				parseUDPMessage(message, deltaTime);
				break;
		}
	}
}

auto broadcastPlayerPositions() -> void
{
	auto playerView = g_entityManager.view<Player>();
	for (const auto entity : playerView)
	{
		auto& playerComponent = g_entityManager.getComponent<Player>(entity);
		auto data             = Common::Network::MessageData();
		data << entity;
		playerComponent.serialise(data);
		g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Position, Common::Network::ClientID(-1), data);
	}
}

auto main() -> int
{
	spdlog::set_level(spdlog::level::debug);

	spdlog::info("Server version {}.{}.{}", Server::Version::getMajor(), Server::Version::getMinor(), Server::Version::getPatch());
	spdlog::debug("Initialising network manager");
	if (!g_networkManager.init())
	{
		return 1;
	}

	// Begin main loop
	sf::Clock clock;
	clock.restart();
	bool shouldExit = false;

	spdlog::debug("Entering main loop");
	while (!shouldExit)
	{
		float deltaTime = clock.restart().asSeconds();

		g_networkManager.update();
		parseMessages(deltaTime, shouldExit);
		broadcastPlayerPositions();
	}

	spdlog::info("Shutting down network manager");
	g_networkManager.shutdown();

	return 0;
}