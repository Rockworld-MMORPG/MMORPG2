#include "Game/Player.hpp"
#include <Common/Network/MessageData.hpp>

namespace Server::Game
{

	auto Player::serialise(Common::Network::MessageData& data) -> void
	{
		data << position.x << position.y;
	}

	auto Player::deserialise(Common::Network::MessageData& data) -> void
	{
		data >> position.x >> position.y;
	}

} // namespace Server::Game