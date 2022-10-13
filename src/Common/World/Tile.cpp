#include "Common/World/Tile.hpp"

namespace Common::World
{

	auto operator|(const Tile::TravelMode modeA, const Tile::TravelMode modeB) -> Tile::TravelMode
	{
		return static_cast<Tile::TravelMode>(static_cast<std::uint8_t>(modeA) | static_cast<std::uint8_t>(modeB));
	}

} // namespace Common::World