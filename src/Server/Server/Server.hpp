#pragma once

#include "Network/NetworkManager.hpp"
#include "Shell/CommandShell.hpp"
#include "entt/entity/fwd.hpp"
#include <Common/Network.hpp>
#include <SFML/System/Clock.hpp>
#include <entt/entity/registry.hpp>

namespace Server
{

	class Manager;

	using MessageHandlerFunction = std::function<void(Common::Network::Message&, Server&)>;
	using SystemFunction         = std::function<void(Server& server, sf::Time deltaTime)>;

	/**
	 * \class Server::Server Server.hpp "Server/Server.hpp"
	 * \brief Manages and updates the global state of the server
	 */
	class Server
	{
	public:
		/**
		 * \brief Construct a new Server object
		 *
		 */
		Server();
		/**
		 * \brief Destroy the Server object
		 *
		 */
		~Server();

		/**
		 * \brief Run the server's main loop until set to exit
		 *
		 */
		auto run() -> void;

		/**
		 * \brief Sets whether or not the server should stop running the main loop
		 *
		 */
		auto setShouldExit(bool shouldExit) -> void;

		/**
		 * \brief Attach a system to the server, to be updated every cycle of the main loop
		 *
		 * \param system The system to attach to the server
		 */
		auto addSystem(SystemFunction&& system) -> void;

		/**
		 * \brief Attach a system to the server, to be updated at a specific interval
		 *
		 * \param system The system to attach to the server
		 * \param updateInterval How often the system should be updated
		 */
		auto addSystem(SystemFunction&& system, sf::Time updateInterval) -> void;

		/**
		 * \brief Clear all systems from the server
		 *
		 */
		auto clearSystems() -> void;

		/**
		 * \brief Register a message handler with the server
		 *
		 * \param messageType The type of message the handler should accept
		 * \param handlerFunction The function to be called when a message of the given type is received
		 */
		auto addMessageHandler(Common::Network::MessageType messageType, MessageHandlerFunction&& handlerFunction) -> void;

		/**
		 * \brief Clear all message handlers of a certain type
		 *
		 * \param messageType The type of message to clear the handlers for
		 */
		auto clearMessageHandlers(Common::Network::MessageType messageType) -> void;

		CommandShell commandShell;
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