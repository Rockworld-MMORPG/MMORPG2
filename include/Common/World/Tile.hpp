#pragma once

#include "Common/Export.hpp"
#include <array>
#include <cstdint>

namespace Common::World
{

	/**
	 * \struct Tile Tile.hpp <Common/World/Tile.hpp>
	 * \brief Data representing a type of tile in the world space
	 */
	struct Tile
	{
		enum class TravelMode : std::uint8_t
		{
			None = 0,
			Walk = 1,
			Fly  = 2
		} travelMode;
	};

	COMMON_API auto operator|(Tile::TravelMode modeA, Tile::TravelMode modeB) -> Tile::TravelMode;

} // namespace Common::World