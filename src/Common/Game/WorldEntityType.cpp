#include "Common/Game/WorldEntityType.hpp"

namespace Common::Game
{

	auto WorldEntityType::serialise(Network::MessageData& data) -> void
	{
		data << type;
	}

	auto WorldEntityType::deserialise(Network::MessageData& data) -> void
	{
		data >> type;
	}

} // namespace Common::Game