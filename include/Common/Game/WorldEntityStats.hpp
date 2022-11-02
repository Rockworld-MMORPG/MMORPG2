#pragma once

#include "Common/Network/SerialisedComponent.hpp"

namespace Common::Game
{
	/**
	 * \struct StatBlock WorldEntityStats.hpp <Common/Game/WorldEntityStats.hpp>
	 * \brief Represents a statistic that can change and regenerate
	 */
	struct COMMON_API StatBlock : Network::SerialisedComponent<StatBlock>
	{
		std::uint32_t max       = 100'000;
		std::uint32_t current   = 0;
		std::uint32_t regenRate = 1'000;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

	/**
	 * \struct WorldEntityStats WorldEntityStats.hpp <Common/Game/WorldEntityStats.hpp>
	 * \brief Data about the stats of an entity that exists within the world space
	 */
	struct COMMON_API WorldEntityStats : Network::SerialisedComponent<WorldEntityStats>
	{
		StatBlock health;
		StatBlock magic;

		auto serialise(Network::MessageData& data) -> void;
		auto deserialise(Network::MessageData& data) -> void;
	};

} // namespace Common::Game