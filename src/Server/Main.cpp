#include "Common/Network/MessageType.hpp"
#include "EntityManager.hpp"
#include "NetworkManager.hpp"
#include "Version.hpp"
#include <SFML/System/Clock.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

using namespace Server;

auto parseTCPMessages(bool& shouldExit) -> void
{
	std::optional<Server::Message> nextMessage;
	while ((nextMessage = g_networkManager.getNextTCPMessage()).has_value())
	{
		auto&& message                           = nextMessage.value();
		Common::Network::MessageType messageType = Common::Network::MessageType::None;
		message.packet >> messageType;

		switch (messageType)
		{
			case Common::Network::MessageType::Connect:
			{
				std::uint16_t udpPort = 0;
				message.packet >> udpPort;
				g_networkManager.setClientUdpPort(message.clientID, udpPort);
			}
			break;
			case Common::Network::MessageType::Disconnect:
			{
				g_networkManager.markForDisconnect(message.clientID);
			}
			break;
			case Common::Network::MessageType::Terminate:
			{
				spdlog::info("Server has been sent a terminate command");
				shouldExit = true;
			}
			break;
			default:
				spdlog::warn("Received a non-TCP packet over TCP");
		}
	}
}

auto parseUDPMessages(const float deltaTime) -> void
{
}

auto broadcastPlayerPositions() -> void
{
}

auto main() -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	spdlog::info("Server version {}.{}.{}", Server::Version::getMajor(), Server::Version::getMinor(), Server::Version::getPatch());
	spdlog::debug("Initialising network manager");
	g_networkManager.init();

	// Begin main loop
	sf::Clock clock;
	clock.restart();
	bool shouldExit = false;

	spdlog::debug("Entering main loop");
	while (!shouldExit)
	{
		float deltaTime = clock.restart().asSeconds();
		g_networkManager.update();

		parseTCPMessages(shouldExit);
		parseUDPMessages(deltaTime);
	}

	spdlog::info("Shutting down network manager");
	g_networkManager.shutdown();

	return 0;
}