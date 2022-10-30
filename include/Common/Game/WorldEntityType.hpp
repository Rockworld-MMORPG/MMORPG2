#pragma once

#include "Common/Network/SerialisedComponent.hpp"

namespace Common::Game
{

	struct WorldEntityType : Network::SerialisedComponent<WorldEntityType>
	{
		std::uint32_t type = 0;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game