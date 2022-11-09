#include "Common/Game/WorldEntityPosition.hpp"

namespace Common::Game
{

	auto WorldEntityPosition::serialise(Network::MessageData& data) -> void
	{
		data << instanceID << position.x << position.y << direction.x << direction.y;
	}

	auto WorldEntityPosition::deserialise(Network::MessageData& data) -> void
	{
		data >> instanceID >> position.x >> position.y >> direction.x >> direction.y;
	}

} // namespace Common::Game