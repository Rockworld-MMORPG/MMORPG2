#include "Server.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "Game/Player.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/InputState.hpp>
#include <SFML/System/Sleep.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace Server
{

	Server::Server() :
	    networkManager(*this)
	{
		m_clock.restart();
		networkManager.init();
	}

	Server::~Server()
	{
		networkManager.shutdown();
	}

	auto Server::run() -> void
	{
		const auto MAX_UPDATES_PER_SECOND = 10;
		const auto MIN_UPDATE_TIME        = sf::milliseconds(1000 / MAX_UPDATES_PER_SECOND);

		while (!m_serverShouldExit)
		{
			auto deltaTime = m_clock.restart();

			networkManager.update();
			parseMessages();
			updatePlayers(deltaTime);
			broadcastPlayerPositions();

			std::this_thread::sleep_for((MIN_UPDATE_TIME - m_clock.getElapsedTime()).toDuration());
		}
	}

	auto Server::parseTCPMessage(Common::Network::Message& message) -> void
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
				networkManager.setClientUdpPort(message.header.entityID, udpPort);
			}
			break;
			case Common::Network::MessageType::Disconnect:
			{
				auto data = Common::Network::MessageData();
				data << message.header.entityID;
				networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::DestroyEntity, data);
				networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, message.header.entityID, data);
				networkManager.markForDisconnect(message.header.entityID);
			}
			break;
			case Common::Network::MessageType::Terminate:
			{
				spdlog::info("Server has been sent a terminate command");
				m_serverShouldExit = true;

				auto data = Common::Network::MessageData();
				networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, data);
			}
			break;
			default:
				spdlog::warn("Received a non-TCP packet over TCP ({:X})", static_cast<std::uint8_t>(message.header.type));
		}
	}

	auto Server::parseUDPMessage(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::CreateEntity:
			{
				if (registry.all_of<Game::Player>(message.header.entityID))
				{
					break;
				}

				spdlog::debug("Creating a player for {}", static_cast<std::uint32_t>(message.header.entityID));

				auto& player = registry.emplace<Game::Player>(message.header.entityID);
				registry.emplace<Common::Input::InputState>(message.header.entityID);

				auto data = Common::Network::MessageData();
				data << message.header.entityID;
				player.serialise(data);
				networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
			}
			break;
			case Common::Network::MessageType::GetEntity:
			{
				// auto clientID = Common::Network::ClientID(-1);
				// message.data >> clientID;

				// auto iterator = m_clientEntityMap.find(clientID);
				// if (!registry.all_of<Game::Player>(iterator->second))
				// {
				// 	break;
				// }

				// auto player = registry.get<Game::Player>(iterator->second);
				// auto data   = Common::Network::MessageData();
				// data << clientID;
				// player.serialise(data);
				// networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, message.header.clientID, data);
			}
			break;
			case Common::Network::MessageType::Action:
			{
				auto action = Common::Input::Action();
				message.data >> action;

				if (!registry.all_of<Common::Input::InputState>(message.header.entityID))
				{
					break;
				}

				auto& inputState   = registry.get<Common::Input::InputState>(message.header.entityID);
				inputState.changed = true;
				switch (action.type)
				{
					case Common::Input::ActionType::MoveForward:
					{
						inputState.forwards = (action.state == Common::Input::Action::State::Begin);
					}
					break;
					case Common::Input::ActionType::MoveBackward:
					{
						inputState.backwards = (action.state == Common::Input::Action::State::Begin);
					}
					break;
					case Common::Input::ActionType::StrafeLeft:
					{
						inputState.left = (action.state == Common::Input::Action::State::Begin);
					}
					break;
					case Common::Input::ActionType::StrafeRight:
					{
						inputState.right = (action.state == Common::Input::Action::State::Begin);
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

	auto Server::parseMessages() -> void
	{
		auto messages = networkManager.getMessages();
		for (auto& message : messages)
		{
			switch (message.header.protocol)
			{
				case Common::Network::Protocol::TCP:
					parseTCPMessage(message);
					break;
				case Common::Network::Protocol::UDP:
					parseUDPMessage(message);
					break;
			}
		}
	}

	auto Server::broadcastPlayerPositions() -> void
	{
		for (const auto entity : registry.view<Game::Player, Common::Input::InputState>())
		{
			auto& inputState = registry.get<Common::Input::InputState>(entity);
			if (inputState.changed)
			{
				auto data = Common::Network::MessageData();
				data << entity << inputState;
				networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::InputState, data);
				inputState.changed = false;
			}
		}
	}

	auto Server::updatePlayers(const sf::Time deltaTime) -> void
	{
		auto fDt = deltaTime.asSeconds();

		auto view = registry.view<Game::Player, Common::Input::InputState>();
		for (const auto entity : view)
		{
			auto& playerComponent = registry.get<Game::Player>(entity);
			auto& inputComponent  = registry.get<Common::Input::InputState>(entity);

			sf::Vector2f delta{0.0F, 0.0F};
			if (inputComponent.forwards)
			{
				delta.y -= 200.0F * fDt;
			}
			if (inputComponent.backwards)
			{
				delta.y += 200.0F * fDt;
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

} // namespace Server