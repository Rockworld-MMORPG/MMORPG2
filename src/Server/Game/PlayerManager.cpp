#include "Game/PlayerManager.hpp"
#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageType.hpp"
#include "EntityManager.hpp"
#include <SFML/Network/Packet.hpp>
#include <functional>

namespace Server
{

	PlayerManager g_playerManager;

	auto Player::serialise(Common::Network::MessageData& data) -> void
	{
		data << position.x << position.y;
	}

	auto Player::deserialise(Common::Network::MessageData& data) -> void
	{
		data >> position.x >> position.y;
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