#include "Common/Game/WorldPosition.hpp"

namespace Common::Game
{

	auto WorldPosition::serialise(Network::MessageData& data) -> void
	{
		data << instanceID << position.x << position.y;
	}

	auto WorldPosition::deserialise(Network::MessageData& data) -> void
	{
		data >> instanceID >> position.x >> position.y;
	}

} // namespace Common::Game