#pragma once

#include "TerrainTile.hpp"
#include <Common/World/Level.hpp>

class TerrainRenderer
{
public:
	auto addLevel(Common::World::Level& level) -> void;
	auto clear() -> void;
	auto render(sf::RenderTarget& renderTarget) -> void;

private:
	std::vector<TerrainTile> m_tiles;
};