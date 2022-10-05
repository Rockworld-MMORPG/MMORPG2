#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageType.hpp"
#include "EntityManager.hpp"
#include "Game/PlayerManager.hpp"
#include "Network/Message.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/Network/Packet.hpp"
#include "Version.hpp"
#include <SFML/System/Clock.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <thread>

using namespace Server;

auto parseTCPMessage(Message& message, bool& shouldExit) -> void
{
	auto messageType = Common::Network::MessageType::None;
	message.packet >> messageType;

	switch (messageType)
	{
		case Common::Network::MessageType::Connect:
		{
			std::uint16_t udpPort = 0;
			message.packet >> udpPort;
			g_networkManager.setClientUdpPort(message.clientID, udpPort);
			g_playerManager.createPlayer(message.clientID);

			Message message{Common::Network::ClientID(-1), Server::Message::Protocol::UDP};
			message.packet << Common::Network::MessageType::CreateEntity << message.clientID;
			g_networkManager.pushMessage(std::move(message));
		}
		break;
		case Common::Network::MessageType::Disconnect:
		{
			g_playerManager.destroyPlayer(message.clientID);
			g_networkManager.markForDisconnect(message.clientID);

			Message message{Common::Network::ClientID(-1), Server::Message::Protocol::UDP};
			message.packet << Common::Network::MessageType::DestroyEntity << message.clientID;
			g_networkManager.pushMessage(std::move(message));
		}
		break;
		case Common::Network::MessageType::Terminate:
		{
			spdlog::info("Server has been sent a terminate command");
			shouldExit = true;

			Message message{Common::Network::ClientID(-1), Server::Message::Protocol::TCP};
			message.packet << Common::Network::MessageType::Terminate;
			g_networkManager.pushMessage(std::move(message));
		}
		break;
		default:
			spdlog::warn("Received a non-TCP packet over TCP");
	}
}

auto parseUDPMessage(Message& message, const float deltaTime) -> void
{
	auto messageType = Common::Network::MessageType::None;
	message.packet >> messageType;

	switch (messageType)
	{
		case Common::Network::MessageType::Movement:
		{
			sf::Vector2f movementDelta{0.0F, 0.0F};
			message.packet >> movementDelta.x >> movementDelta.y;

			auto optPlayer = g_playerManager.getPlayer(message.clientID);
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
	std::optional<Server::Message> nextMessage;
	while ((nextMessage = g_networkManager.getNextMessage()).has_value())
	{
		auto& message = nextMessage.value();
		switch (message.protocol)
		{
			case Server::Message::Protocol::TCP:
				parseTCPMessage(message, shouldExit);
				break;
			case Server::Message::Protocol::UDP:
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

		auto message = Message(Common::Network::ClientID(-1), Server::Message::Protocol::UDP);
		message.packet << Common::Network::MessageType::Position << entity;
		playerComponent.serialise(message.packet);
		g_networkManager.pushMessage(std::move(message));
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