#pragma once

#include "Common/Network/SerialisedComponent.hpp"

namespace Common::Game
{

	/**
	 * \struct WorldEntityType WorldEntityType.hpp <Common/Game/WorldEntityType.hpp>
	 * \brief Data about the type of an entity that exists within the world space
	 */
	struct COMMON_API WorldEntityType : Network::SerialisedComponent<WorldEntityType>
	{
		std::uint32_t type = 0;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game