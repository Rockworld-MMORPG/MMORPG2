#pragma once

#include "ECS/EntityManager.hpp"
#include "Game/PlayerManager.hpp"
#include "Network/NetworkManager.hpp"
#include <SFML/System/Clock.hpp>

namespace Server
{

	class Manager;

	class Server
	{
	public:
		Server();
		~Server();

		auto run() -> void;
		auto addManager(std::unique_ptr<Manager>&& manager) -> void;
		auto addManager(std::unique_ptr<Manager>&& manager, sf::Time updateInterval) -> void;

		NetworkManager networkManager;
		PlayerManager playerManager;
		EntityManager entityManager;

	private:
		auto parseTCPMessage(Common::Network::Message& message) -> void;
		auto parseUDPMessage(Common::Network::Message& message) -> void;
		auto parseMessages() -> void;
		auto broadcastPlayerPositions() -> void;
		auto updatePlayers(sf::Time deltaTime) -> void;

		std::vector<std::pair<sf::Time, std::unique_ptr<Manager>>> m_managers;

		bool m_serverShouldExit = false;
		sf::Clock m_clock;
	};

} // namespace Server