#pragma once

#include "Common/Network/Message.hpp"
#include "Common/Network/MessageType.hpp"
#include "Network/NetworkManager.hpp"
#include "entt/entity/fwd.hpp"
#include <SFML/System/Clock.hpp>
#include <entt/entity/registry.hpp>

namespace Server
{

	class Manager;

	using MessageHandlerFunction = std::function<void(Common::Network::Message&, Server&)>;
	using SystemFunction         = std::function<void(Server& server, sf::Time deltaTime)>;

	class Server
	{
	public:
		Server();
		~Server();

		auto run() -> void;
		auto setShouldExit(bool shouldExit) -> void;

		auto addSystem(SystemFunction&& system) -> void;
		auto addSystem(SystemFunction&& system, sf::Time updateInterval) -> void;
		auto clearSystems() -> void;

		auto addMessageHandler(Common::Network::MessageType messageType, MessageHandlerFunction&& handlerFunction) -> void;
		auto clearMessageHandlers(Common::Network::MessageType messageType) -> void;

		NetworkManager networkManager;
		entt::registry registry;

	private:
		auto parseMessages() -> void;

		struct SystemWrapper
		{
			sf::Time firingInterval;
			sf::Time timeToNextFire;
			SystemFunction callback;
		};

		std::vector<SystemWrapper> m_systems;
		std::unordered_map<Common::Network::MessageType, MessageHandlerFunction> m_messageHandlers;

		bool m_serverShouldExit = false;
		sf::Clock m_clock;
	};

} // namespace Server