#include "Server.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "Game/Player.hpp"
#include "SFML/System/Time.hpp"
#include <Common/Input/Action.hpp>
#include <Common/Input/InputState.hpp>
#include <SFML/System/Sleep.hpp>
#include <spdlog/spdlog.h>
#include <thread>

namespace Server
{

#define SYSTEM_FN(NAME) auto system##NAME(Server& server, const sf::Time deltaTime)->void
#define HANDLER_FN(NAME) auto handler##NAME(Common::Network::Message& message, Server& server)->void

	SYSTEM_FN(PlayerMovement)
	{
		auto fDt = deltaTime.asSeconds();

		auto view = server.registry.view<Game::Player, Common::Input::InputState>();
		for (const auto entity : view)
		{
			auto& playerComponent = server.registry.get<Game::Player>(entity);
			auto& inputComponent  = server.registry.get<Common::Input::InputState>(entity);

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

	SYSTEM_FN(BroadcastMovement)
	{
		for (const auto entity : server.registry.view<Game::Player, Common::Input::InputState>())
		{
			auto& inputState = server.registry.get<Common::Input::InputState>(entity);
			if (inputState.changed)
			{
				auto data = Common::Network::MessageData();
				data << entity << inputState;
				server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::InputState, data);
				inputState.changed = false;
			}
		}
	}

	HANDLER_FN(Connect)
	{
		std::uint16_t udpPort = 0;
		message.data >> udpPort;
		server.networkManager.setClientUdpPort(message.header.entityID, udpPort);
	}

	HANDLER_FN(Disconnect)
	{
		auto data = Common::Network::MessageData();
		data << message.header.entityID;
		server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::DestroyEntity, data);
		server.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, message.header.entityID, data);
		server.networkManager.markForDisconnect(message.header.entityID);
	}

	HANDLER_FN(Terminate)
	{
		spdlog::info("Server has been sent a terminate command");
		server.setShouldExit(true);

		auto data = Common::Network::MessageData();
		server.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Disconnect, data);
	}

	HANDLER_FN(CreateEntity)
	{
		if (server.registry.all_of<Game::Player>(message.header.entityID))
		{
			return;
		}

		spdlog::debug("Creating a player for {}", static_cast<std::uint32_t>(message.header.entityID));

		auto& player = server.registry.emplace<Game::Player>(message.header.entityID);
		server.registry.emplace<Common::Input::InputState>(message.header.entityID);

		auto data = Common::Network::MessageData();
		data << message.header.entityID;
		player.serialise(data);
		server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
	}

	HANDLER_FN(Action)
	{
		auto action = Common::Input::Action();
		message.data >> action;

		if (!server.registry.all_of<Common::Input::InputState>(message.header.entityID))
		{
			return;
		}

		auto& inputState   = server.registry.get<Common::Input::InputState>(message.header.entityID);
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
		}
	}


#undef SYSTEM_FN
#undef HANDLER_FN

	Server::Server() :
	    networkManager(*this)
	{
		m_clock.restart();
		networkManager.init();

		addSystem(systemPlayerMovement, sf::milliseconds(50));
		addSystem(systemBroadcastMovement, sf::milliseconds(50));

		using MT
		    = Common::Network::MessageType;
		addMessageHandler(MT::Connect, handlerConnect);
		addMessageHandler(MT::Disconnect, handlerDisconnect);
		addMessageHandler(MT::Terminate, handlerTerminate);

		addMessageHandler(MT::CreateEntity, handlerCreateEntity);
		addMessageHandler(MT::Action, handlerAction);
	}

	Server::~Server()
	{
		networkManager.shutdown();
	}

	auto Server::addSystem(SystemFunction&& system) -> void
	{
		auto wrapper = SystemWrapper{sf::Time::Zero, sf::Time::Zero, system};
		m_systems.emplace_back(std::move(wrapper));
	}

	auto Server::addSystem(SystemFunction&& system, sf::Time updateInterval) -> void
	{
		auto wrapper = SystemWrapper{updateInterval, sf::Time::Zero, system};
		m_systems.emplace_back(std::move(wrapper));
	}

	auto Server::clearSystems() -> void
	{
		m_systems.clear();
	}

	auto Server::addMessageHandler(Common::Network::MessageType messageType, MessageHandlerFunction&& handlerFunction) -> void
	{
		auto [pair, success] = m_messageHandlers.emplace(messageType, std::forward<MessageHandlerFunction>(handlerFunction));
		if (!success)
		{
			spdlog::debug("Tried to add a message handler but one already exists for {:X}", static_cast<std::uint32_t>(messageType));
		}
	}

	auto Server::clearMessageHandlers(Common::Network::MessageType messageType) -> void
	{
		m_messageHandlers.erase(messageType);
	}

	auto Server::run() -> void
	{
		const auto MAX_UPDATES_PER_SECOND = 20;
		const auto MIN_UPDATE_TIME        = sf::milliseconds(1000 / MAX_UPDATES_PER_SECOND);
		auto timeToNextSystemFire         = sf::Time::Zero;

		while (!m_serverShouldExit)
		{
			auto deltaTime = m_clock.restart();
			parseMessages();

			for (auto& system : m_systems)
			{
				// The system is set to fire at every update, so fire it
				if (system.firingInterval == sf::Time::Zero)
				{
					system.callback(*this, deltaTime);
				}
				// The system is set to fire on an interval
				else
				{
					system.timeToNextFire -= deltaTime;
					// The system is due to fire, so fire the system and reset it's counter
					if (system.timeToNextFire.asMilliseconds() <= 0)
					{
						system.callback(*this, system.firingInterval - system.timeToNextFire);
						system.timeToNextFire = system.firingInterval;
					}
					// The system is not due to fire, so see if it's next to fire and set the timeToNextSystemFire accordingly
					else if (system.timeToNextFire < timeToNextSystemFire)
					{
						timeToNextSystemFire = system.timeToNextFire;
					}
				}
			}

			// Use the socket selector to stall the server until the next system needs to fire
			if (timeToNextSystemFire < MIN_UPDATE_TIME)
			{
				timeToNextSystemFire = MIN_UPDATE_TIME;
			}
			networkManager.update(timeToNextSystemFire);
		}
	}

	auto Server::setShouldExit(bool shouldExit) -> void
	{
		m_serverShouldExit = shouldExit;
	}

	auto Server::parseMessages() -> void
	{
		auto messages = networkManager.getMessages();
		for (auto& message : messages)
		{
			if (m_messageHandlers.contains(message.header.type))
			{
				m_messageHandlers.at(message.header.type)(message, *this);
			}
		}
	}

} // namespace Server