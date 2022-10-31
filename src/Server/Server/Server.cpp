#include "Server.hpp"
#include <Common/Game.hpp>

namespace Server
{

#define SYSTEM_FN(NAME) auto system##NAME(Server& server, const sf::Time deltaTime)->void
#define HANDLER_FN(NAME) auto handler##NAME(Common::Network::Message& message, Server& server)->void

	SYSTEM_FN(PlayerMovement)
	{
		auto fDt = deltaTime.asSeconds();

		auto view = server.registry.view<Common::Game::WorldEntityPosition, Common::Input::InputState>();
		for (const auto entity : view)
		{
			auto& worldPositionComponent = server.registry.get<Common::Game::WorldEntityPosition>(entity);
			auto& inputComponent         = server.registry.get<Common::Input::InputState>(entity);

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

			worldPositionComponent.position += delta;
		}
	}

	SYSTEM_FN(BroadcastMovement)
	{
		for (const auto entity : server.registry.view<Common::Game::WorldEntityPosition, Common::Input::InputState>())
		{
			auto& inputState = server.registry.get<Common::Input::InputState>(entity);
			if (inputState.changed)
			{
				auto data = Common::Network::MessageData();
				data << entity << inputState;
				server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Server_InputState, data);
				inputState.changed = false;
			}
		}
	}

	HANDLER_FN(Connect)
	{
		auto udpPort = std::uint16_t(0);
		message.data >> udpPort;
		server.networkManager.setClientUdpPort(message.header.entityID, udpPort);
	}

	HANDLER_FN(Disconnect)
	{
		auto data = Common::Network::MessageData();
		data << message.header.entityID;
		server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Server_DestroyEntity, data);
		server.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Server_Disconnect, message.header.entityID, data);
		server.networkManager.markForDisconnect(message.header.entityID);

		if (server.registry.all_of<Login::UserData>(message.header.entityID))
		{
			server.loginManager.logout(server.registry.get<Login::UserData>(message.header.entityID).username);
		}
	}

	HANDLER_FN(Command)
	{
		auto command = std::string(static_cast<char*>(message.data.data()), message.data.size());
		server.commandShell.parseMessage(command);
	}

	HANDLER_FN(Spawn)
	{
		if (server.registry.all_of<Common::Game::WorldEntityPosition>(message.header.entityID))
		{
			return;
		}

		auto username = server.registry.get<Login::UserData>(message.header.entityID).username;

		spdlog::debug("Creating a player for {}", username);

		server.registry.emplace_or_replace<Common::Input::InputState>(message.header.entityID);
		Common::Game::createWorldEntity(server.registry, message.header.entityID, {});
		auto& worldEntityName = server.registry.get<Common::Game::WorldEntityName>(message.header.entityID);
		worldEntityName.name  = username;

		auto data = Common::Network::MessageData();
		data << message.header.entityID;
		Common::Game::serialiseWorldEntity(server.registry, message.header.entityID, data);
		server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Server_CreateEntity, data);
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
			default:
				break;
		}
	}

	HANDLER_FN(GetWorldState)
	{
		auto tileIdentifier = std::uint32_t(0);
		message.data >> tileIdentifier;

		// Emplace the count at position 0, then serialise entities into it
		auto data       = Common::Network::MessageData();
		auto entityList = std::vector<entt::entity>();

		for (const auto entity : server.registry.view<Common::Game::WorldEntityPosition>())
		{
			auto& worldPositionComponent = server.registry.get<Common::Game::WorldEntityPosition>(entity);
			if (worldPositionComponent.instanceID == tileIdentifier)
			{
				entityList.emplace_back(entity);
			}
		}

		data << std::uint32_t(entityList.size());
		for (const auto entity : entityList)
		{
			data << entity;
			Common::Game::serialiseWorldEntity(server.registry, entity, data);
		}

		server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Server_WorldState, data);
	}

	HANDLER_FN(Authenticate)
	{
		auto username = std::string();
		auto password = std::string();

		message.data >> username >> password;

		auto data = Common::Network::MessageData();

		auto loggedInResult = server.loginManager.isLoggedIn(username);
		if (loggedInResult)
		{
			data << static_cast<std::uint8_t>(Login::AuthenticationResult::AlreadyLoggedIn);
		}
		else
		{
			auto authResult = server.loginManager.authenticate(username, password);
			data << static_cast<std::uint8_t>(authResult);

			if (authResult == Login::AuthenticationResult::Valid)
			{
				server.registry.emplace<Login::UserData>(message.header.entityID, Login::UserData{username});
				server.loginManager.login(username);
			}
		}

		server.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Server_Authenticate, message.header.entityID, data);
	}


#undef SYSTEM_FN
#undef HANDLER_FN

	Server::Server(const std::filesystem::path& executableDirectory) :
	    databaseManager("database.db"),
	    loginManager(databaseManager),
	    networkManager(*this)
	{
		databaseManager.createTables();
		loginManager.createUser("admin", "password");

		m_clock.restart();
		networkManager.init();

		addSystem(systemPlayerMovement, sf::milliseconds(50));
		addSystem(systemBroadcastMovement, sf::milliseconds(50));

		using MT
		    = Common::Network::MessageType;
		addMessageHandler(MT::Client_Connect, handlerConnect);
		addMessageHandler(MT::Client_Disconnect, handlerDisconnect);
		addMessageHandler(MT::Client_Authenticate, handlerAuthenticate);

		addMessageHandler(MT::Command, handlerCommand);

		addMessageHandler(MT::Client_Spawn, handlerSpawn);
		addMessageHandler(MT::Client_Action, handlerAction);
		addMessageHandler(MT::Client_GetWorldState, handlerGetWorldState);

		commandShell.registerCommand("terminate", [&](std::vector<std::string> tokens) {
			m_serverShouldExit = true;
			return;
		});
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