#pragma once

#include "Common/Network/SerialisedComponent.hpp"

namespace Common::Game
{

	/**
	 * \struct WorldEntityName WorldEntityName.hpp <Common/Game/WorldEntityName.hpp>
	 * \brief Data about the name of an entity in the world
	 */
	struct COMMON_API WorldEntityName : Network::SerialisedComponent<WorldEntityName>
	{
		std::string name = "";

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game