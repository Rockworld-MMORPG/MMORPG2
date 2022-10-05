#include "Game/PlayerManager.hpp"
#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageType.hpp"
#include "EntityManager.hpp"
#include <SFML/Network/Packet.hpp>
#include <functional>

namespace Server
{

	PlayerManager g_playerManager;

	auto Player::serialise(sf::Packet& packet) -> void
	{
		packet << position.x << position.y;
	}

	auto PlayerManager::createPlayer(const Common::Network::ClientID clientID) -> void
	{
		g_entityManager.addComponent<Player>(clientID);
	}

	auto PlayerManager::destroyPlayer(const Common::Network::ClientID clientID) -> void
	{
		g_entityManager.removeComponent<Player>(clientID);
	}

	auto PlayerManager::getPlayer(const Common::Network::ClientID clientID) -> std::optional<std::reference_wrapper<Player>>
	{
		return g_entityManager.getComponent<Player>(clientID);
	}

} // namespace Server