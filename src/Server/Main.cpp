#include "Common/Input/Action.hpp"
#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageHeader.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "EntityManager.hpp"
#include "Game/PlayerManager.hpp"
#include "Input/InputState.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/Network/Packet.hpp"
#include "Version.hpp"
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
			g_networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, data);
		}
		break;
		case Common::Network::MessageType::Terminate:
		{
			spdlog::info("Server has been sent a terminate command");
			shouldExit = true;

			auto data = Common::Network::MessageData();
			g_networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, data);
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
			g_entityManager.addComponent<Input::InputState>(message.header.clientID);

			auto data = Common::Network::MessageData();
			data << message.header.clientID;
			g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
		}
		break;
		case Common::Network::MessageType::Action:
		{
			auto action = Common::Input::Action();
			message.data >> action;

			switch (action.type)
			{
				case Common::Input::ActionType::MoveForward:
				{
					auto optInputState = g_entityManager.getComponent<Input::InputState>(message.header.clientID);
					if (optInputState.has_value())
					{
						optInputState->get().forwards = (action.state == Common::Input::Action::State::Begin);
					}
				}
				break;
				case Common::Input::ActionType::MoveBackward:
				{
					auto optInputState = g_entityManager.getComponent<Input::InputState>(message.header.clientID);
					if (optInputState.has_value())
					{
						optInputState->get().backwards = (action.state == Common::Input::Action::State::Begin);
					}
				}
				break;
				case Common::Input::ActionType::StrafeLeft:
				{
					auto optInputState = g_entityManager.getComponent<Input::InputState>(message.header.clientID);
					if (optInputState.has_value())
					{
						optInputState->get().left = (action.state == Common::Input::Action::State::Begin);
					}
				}
				break;
				case Common::Input::ActionType::StrafeRight:
				{
					auto optInputState = g_entityManager.getComponent<Input::InputState>(message.header.clientID);
					if (optInputState.has_value())
					{
						optInputState->get().right = (action.state == Common::Input::Action::State::Begin);
					}
				}
				break;
				default:
					break;
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
	auto data = Common::Network::MessageData();

	auto playerView = g_entityManager.view<Player>();
	data << static_cast<std::uint32_t>(playerView.size());
	for (const auto entity : playerView)
	{
		auto& playerComponent = g_entityManager.getComponent<Player>(entity);
		data << entity;
		playerComponent.serialise(data);
	}

	g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Position, data);
}

auto updatePlayers(sf::Time deltaTime) -> void
{
	auto fDt = deltaTime.asSeconds();

	auto& registry = g_entityManager.getRegistry();
	auto view      = registry.view<Player, Input::InputState>();
	for (const auto entity : view)
	{
		auto& playerComponent = registry.get<Player>(entity);
		auto& inputComponent  = registry.get<Input::InputState>(entity);

		sf::Vector2f delta{0.0F, 0.0F};
		if (inputComponent.forwards)
		{
			delta.y += 200.0F * fDt;
		}
		if (inputComponent.backwards)
		{
			delta.y -= 200.0F * fDt;
		}
		if (inputComponent.left)
		{
			delta.x -= 200.0F * fDt;
		}
		if (inputComponent.right)
		{
			delta.x += 200.0F * fDt;
		}

		playerComponent.position += delta;
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
		auto deltaTime = clock.restart();

		g_networkManager.update();
		parseMessages(deltaTime.asSeconds(), shouldExit);
		updatePlayers(deltaTime);
		broadcastPlayerPositions();
	}

	spdlog::info("Shutting down network manager");
	g_networkManager.shutdown();

	return 0;
}