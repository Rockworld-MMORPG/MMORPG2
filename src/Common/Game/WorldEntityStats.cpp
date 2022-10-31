#include "Common/Game/WorldEntityStats.hpp"

namespace Common::Game
{

	auto StatBlock::serialise(Network::MessageData& data) -> void
	{
		data << max << current << regenRate;
	}

	auto StatBlock::deserialise(Network::MessageData& data) -> void
	{
		data >> max >> current >> regenRate;
	}

	auto WorldEntityStats::serialise(Network::MessageData& data) -> void
	{
		health.serialise(data);
		magic.serialise(data);
	}

	auto WorldEntityStats::deserialise(Network::MessageData& data) -> void
	{
		health.deserialise(data);
		magic.deserialise(data);
	}

} // namespace Common::Game