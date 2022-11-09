#include "Server.hpp"
#include <Common/Game.hpp>
#include <nlohmann/json.hpp>

namespace Server
{

#define SYSTEM_FN(NAME) auto system##NAME(Server& server, const sf::Time deltaTime)->void
#define HANDLER_FN(NAME) auto handler##NAME(Common::Network::Message& message, Server& server)->void

	auto writePlayerToDatabase(entt::entity entity, entt::registry& registry, DatabaseManager& databaseManager) -> void
	{
		auto& name     = registry.get<Common::Game::WorldEntityName>(entity);
		auto& position = registry.get<Common::Game::WorldEntityPosition>(entity);
		auto& stats    = registry.get<Common::Game::WorldEntityStats>(entity);

		auto jsSkills = nlohmann::json();
		jsSkills.emplace("melee", 0);
		jsSkills.emplace("ranged", 0);

		auto jsWorldPosition = nlohmann::json();
		jsWorldPosition.emplace("instance", 0);
		jsWorldPosition.emplace("position", std::array<float, 2>{position.position.x, position.position.y});

		auto jsStats = nlohmann::json();
		jsStats.emplace("health", std::array<std::uint32_t, 3>{stats.health.current, stats.health.max, stats.health.regenRate});
		jsStats.emplace("power", std::array<std::uint32_t, 3>{stats.power.current, stats.power.max, stats.power.regenRate});

		auto query = nlohmann::json();
		query.emplace("world_position", jsWorldPosition);
		query.emplace("stats", jsStats);
		query.emplace("skills", jsSkills);
		query.emplace("name", name.name);

		databaseManager.replace("rockworld_testing", "players", nlohmann::json::parse("{\"name\": \"" + name.name + "\"}"), query);
	}

	SYSTEM_FN(DatabaseSync)
	{
		for (const auto entity : server.registry.view<Common::Game::WorldEntityPosition>())
		{
			spdlog::debug("Syncing {} to the database the database", static_cast<std::uint32_t>(entity));
			writePlayerToDatabase(entity, server.registry, server.databaseManager);
		}
	}

	SYSTEM_FN(PlayerMovement)
	{
		auto fDt = deltaTime.asSeconds();

		auto view = server.registry.view<Common::Game::WorldEntityPosition, Common::Input::InputState>();
		for (const auto entity : view)
		{
			auto& worldPositionComponent = server.registry.get<Common::Game::WorldEntityPosition>(entity);
			auto& inputComponent         = server.registry.get<Common::Input::InputState>(entity);

			worldPositionComponent.direction.x = 0;
			worldPositionComponent.direction.y = 0;

			if (inputComponent.forwards)
			{
				worldPositionComponent.position.y -= 200.0F * fDt;
				worldPositionComponent.direction.y = -1;
			}
			if (inputComponent.backwards)
			{
				worldPositionComponent.position.y += 200.0F * fDt;
				worldPositionComponent.direction.y = 1;
			}
			if (inputComponent.left)
			{
				worldPositionComponent.position.x -= 200.0F * fDt;
				worldPositionComponent.direction.x = 1;
			}
			if (inputComponent.right)
			{
				worldPositionComponent.position.x += 200.0F * fDt;
				worldPositionComponent.direction.x = -1;
			}

			if (worldPositionComponent.direction.x == 0 && worldPositionComponent.direction.y == 0)
			{
				worldPositionComponent.direction.x = 1;
			}
		}
	}

	SYSTEM_FN(TickGCDs)
	{
		for (const auto entity : server.registry.view<Common::Game::WorldEntityGCD>())
		{
			auto& gcd = server.registry.get<Common::Game::WorldEntityGCD>(entity);
			gcd.currentTime -= deltaTime;
			if (gcd.currentTime.asMicroseconds() < 0)
			{
				gcd.currentTime = sf::Time::Zero;
			}
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

	SYSTEM_FN(BroadcastHealth)
	{
		for (const auto entity : server.registry.view<Common::Game::WorldEntityStats>())
		{
			auto& stats = server.registry.get<Common::Game::WorldEntityStats>(entity);

			auto data = Common::Network::MessageData();
			stats.serialise(data);
			server.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Server_Stats, data);
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

		if (server.registry.all_of<Common::Game::WorldEntityPosition>(message.header.entityID))
		{
			auto& name     = server.registry.get<Common::Game::WorldEntityName>(message.header.entityID);
			auto& position = server.registry.get<Common::Game::WorldEntityPosition>(message.header.entityID);
			auto& stats    = server.registry.get<Common::Game::WorldEntityStats>(message.header.entityID);

			spdlog::debug("Syncing {} to the database", name.name);
			writePlayerToDatabase(message.header.entityID, server.registry, server.databaseManager);
		}

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

		auto optPlayerData = server.databaseManager.get("rockworld_testing", "players", nlohmann::json::parse("{\"name\": \"" + username + "\"}"));
		if (optPlayerData.has_value())
		{
			spdlog::debug("Player {} already exists on the database - fetching", worldEntityName.name);
			auto jsWorldPosition = optPlayerData->at("world_position");
			auto jsStats         = optPlayerData->at("stats");
			auto jsSkills        = optPlayerData->at("skills");

			auto& worldEntityPosition = server.registry.get<Common::Game::WorldEntityPosition>(message.header.entityID);
			auto& worldEntityStats    = server.registry.get<Common::Game::WorldEntityStats>(message.header.entityID);

			worldEntityPosition.instanceID = jsWorldPosition.at("instance").get<std::uint32_t>();
			worldEntityPosition.position.x = jsWorldPosition.at("position").at(0).get<float>();
			worldEntityPosition.position.y = jsWorldPosition.at("position").at(1).get<float>();

			worldEntityStats.health.current   = jsStats.at("health").at(0).get<std::uint32_t>();
			worldEntityStats.health.max       = jsStats.at("health").at(1).get<std::uint32_t>();
			worldEntityStats.health.regenRate = jsStats.at("health").at(2).get<std::uint32_t>();

			worldEntityStats.power.current   = jsStats.at("power").at(0).get<std::uint32_t>();
			worldEntityStats.power.max       = jsStats.at("power").at(1).get<std::uint32_t>();
			worldEntityStats.power.regenRate = jsStats.at("power").at(2).get<std::uint32_t>();
		}
		else
		{
			spdlog::debug("Player {} does not exist on the database - inserting", worldEntityName.name);
			auto query = nlohmann::json::parse(
			    R"(
			{
				"world_position": {
					"instance": 0,
					"position": [0.0, 0.0]
				},
				"stats": {
					"health": [100000, 100000, 1000],
					"power": [100000, 100000, 1000]
				},
				"skills": {
					"melee": 0,
					"ranged": 0
				}
			}
				)");
			query.emplace("name", username);
			server.databaseManager.insert("rockworld_testing", "players", query);
		}

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
			case Common::Input::ActionType::Attack:
			{
				auto& entityPosition = server.registry.get<Common::Game::WorldEntityPosition>(message.header.entityID);
				auto& entityGCD      = server.registry.get<Common::Game::WorldEntityGCD>(message.header.entityID);

				if (entityGCD.currentTime.asMicroseconds() > 0)
				{
					break;
				}

				entityGCD.currentTime = entityGCD.resetTime;

				const auto MAX_DISTANCE = 1000.0F;
				auto distance           = 0.0F;

				auto position   = entityPosition.position;
				auto fDirection = sf::Vector2f(100.0F * static_cast<float>(entityPosition.direction.x), 100.0F * static_cast<float>(entityPosition.direction.y));
				auto length     = fDirection.lengthSq();

				while (distance < MAX_DISTANCE * MAX_DISTANCE)
				{
					distance += length;
					position += fDirection;

					for (const auto entity : server.registry.view<Common::Game::WorldEntityCollider, Common::Game::WorldEntityPosition>())
					{
						auto& otherPosition = server.registry.get<Common::Game::WorldEntityPosition>(entity);
						auto& otherCollider = server.registry.get<Common::Game::WorldEntityCollider>(entity);

						if ((position.x > otherPosition.position.x) || (position.x < otherPosition.position.x + otherCollider.size.x))
						{
							continue;
						}
						if ((position.y < otherPosition.position.y) || (position.y > otherPosition.position.y + otherCollider.size.y))
						{
							continue;
						}

						if (server.registry.all_of<Common::Game::WorldEntityStats>(entity))
						{
							auto& stats = server.registry.get<Common::Game::WorldEntityStats>(entity);
							stats.health.current -= stats.health.regenRate * 20;
							break;
						}
					}
				}
			}
			break;
			case Common::Input::ActionType::Use:
			{
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
	    databaseManager(),
	    loginManager(databaseManager),
	    networkManager(*this)
	{
		loginManager.createUser("gary", "password");
		loginManager.createUser("dave", "password");

		m_clock.restart();
		networkManager.init();

		addSystem(systemTickGCDs);
		addSystem(systemPlayerMovement, sf::milliseconds(50));
		addSystem(systemBroadcastMovement, sf::milliseconds(50));
		addSystem(systemBroadcastHealth, sf::milliseconds(50));
		addSystem(systemDatabaseSync, sf::seconds(300));

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