#pragma once

#include <array>
#include <cstdint>

namespace Common::World
{

	struct Tile
	{
		std::array<std::uint8_t, 3> type;
		enum class TravelMode : std::uint8_t
		{
			None = 0,
			Walk = 1,
			Fly  = 2
		} travelMode;
	};

	auto operator|(Tile::TravelMode modeA, Tile::TravelMode modeB) -> Tile::TravelMode;

} // namespace Common::World