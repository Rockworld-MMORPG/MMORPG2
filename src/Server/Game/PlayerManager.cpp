#include "Game/PlayerManager.hpp"
#include "ECS/EntityManager.hpp"
#include "Server/Server.hpp"
#include <Common/Network/ClientID.hpp>
#include <Common/Network/MessageData.hpp>
#include <Common/Network/MessageType.hpp>
#include <SFML/Network/Packet.hpp>
#include <functional>

namespace Server
{

	auto Player::serialise(Common::Network::MessageData& data) -> void
	{
		data << position.x << position.y;
	}

	auto Player::deserialise(Common::Network::MessageData& data) -> void
	{
		data >> position.x >> position.y;
	}

	PlayerManager::PlayerManager(Server& server) :
	    Manager(server) {}

	auto PlayerManager::createPlayer(const Common::Network::ClientID clientID) -> void
	{
		server.entityManager.addComponent<Player>(clientID);
	}

	auto PlayerManager::destroyPlayer(const Common::Network::ClientID clientID) -> void
	{
		server.entityManager.removeComponent<Player>(clientID);
	}

	auto PlayerManager::getPlayer(const Common::Network::ClientID clientID) -> std::optional<std::reference_wrapper<Player>>
	{
		return server.entityManager.getComponent<Player>(clientID);
	}

	auto PlayerManager::update() -> void
	{
	}

} // namespace Server