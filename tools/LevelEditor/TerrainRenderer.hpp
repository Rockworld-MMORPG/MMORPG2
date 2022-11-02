#pragma once

#include "TerrainTile.hpp"
#include "TextureManager.hpp"
#include <Common/World/Level.hpp>

class TerrainRenderer
{
public:
	auto addLevel(Common::World::Level& level, const TextureManager& textureManager) -> void;
	auto clear() -> void;
	auto update(Common::World::Level& level, const TextureManager& textureManager) -> void;
	auto render(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) -> void;

private:
	TerrainTile m_tile;
	Common::World::Level m_level;
};