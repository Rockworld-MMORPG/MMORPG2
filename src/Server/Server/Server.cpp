#include "Server.hpp"
#include "Game/PlayerManager.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/InputState.hpp>
#include <SFML/System/Sleep.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace Server
{

	Server::Server() :
	    networkManager(*this),
	    playerManager(*this)
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

		auto networkThread = std::thread([&]() {
			while (!m_serverShouldExit)
			{
				networkManager.update();
			}
		});

		while (!m_serverShouldExit)
		{
			auto deltaTime = m_clock.restart();

			parseMessages();
			updatePlayers(deltaTime);
			broadcastPlayerPositions();

			std::this_thread::sleep_for((MIN_UPDATE_TIME - m_clock.getElapsedTime()).toDuration());
		}

		networkThread.join();
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
				networkManager.setClientUdpPort(message.header.clientID, udpPort);
			}
			break;
			case Common::Network::MessageType::Disconnect:
			{
				playerManager.destroyPlayer(message.header.clientID);
				entityManager.destroy(message.header.clientID);
				auto data = Common::Network::MessageData();
				data << message.header.clientID;

				networkManager.markForDisconnect(message.header.clientID);

				networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::DestroyEntity, data);
				networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, message.header.clientID, data);
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
				playerManager.createPlayer(message.header.clientID);
				entityManager.addComponent<Common::Input::InputState>(message.header.clientID);
				auto optPlayer = entityManager.getComponent<Player>(message.header.clientID);

				auto data = Common::Network::MessageData();
				data << message.header.clientID;
				optPlayer->get().serialise(data);
				networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
			}
			break;
			case Common::Network::MessageType::GetEntity:
			{
				auto entity = Common::Network::ClientID_t(-1);
				message.data >> entity;

				auto optPlayer = entityManager.getComponent<Player>(Common::Network::ClientID(entity));
				if (optPlayer.has_value())
				{
					auto data = Common::Network::MessageData();
					data << entity;
					optPlayer->get().serialise(data);
					networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, message.header.clientID, data);
				}
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
						auto optInputState = entityManager.getComponent<Common::Input::InputState>(message.header.clientID);
						if (optInputState.has_value())
						{
							optInputState->get().forwards = (action.state == Common::Input::Action::State::Begin);
						}
					}
					break;
					case Common::Input::ActionType::MoveBackward:
					{
						auto optInputState = entityManager.getComponent<Common::Input::InputState>(message.header.clientID);
						if (optInputState.has_value())
						{
							optInputState->get().backwards = (action.state == Common::Input::Action::State::Begin);
						}
					}
					break;
					case Common::Input::ActionType::StrafeLeft:
					{
						auto optInputState = entityManager.getComponent<Common::Input::InputState>(message.header.clientID);
						if (optInputState.has_value())
						{
							optInputState->get().left = (action.state == Common::Input::Action::State::Begin);
						}
					}
					break;
					case Common::Input::ActionType::StrafeRight:
					{
						auto optInputState = entityManager.getComponent<Common::Input::InputState>(message.header.clientID);
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
		auto data = Common::Network::MessageData();

		auto inputView = entityManager.getRegistry().view<Common::Input::InputState>();
		data << static_cast<std::uint32_t>(inputView.size());
		for (const auto entity : inputView)
		{
			auto& input = entityManager.getRegistry().get<Common::Input::InputState>(entity);
			data << static_cast<Common::Network::ClientID_t>(entity) << input;
		}

		if (!inputView.empty())
		{
			networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::InputState, data);
		}
	}

	auto Server::updatePlayers(const sf::Time deltaTime) -> void
	{
		auto fDt = deltaTime.asSeconds();

		auto& registry = entityManager.getRegistry();
		auto view      = registry.view<Player, Common::Input::InputState>();
		for (const auto entity : view)
		{
			auto& playerComponent = registry.get<Player>(entity);
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