#include "Common/Game/WorldEntityName.hpp"

namespace Common::Game
{

	auto WorldEntityName::serialise(Network::MessageData& data) -> void
	{
		data << name;
	}

	auto WorldEntityName::deserialise(Network::MessageData& data) -> void
	{
		data >> name;
	}

} // namespace Common::Game