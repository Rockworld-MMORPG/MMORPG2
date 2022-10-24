#include "TerrainRenderer.hpp"
#include "Common/World/Level.hpp"
#include "TerrainTile.hpp"
#include <SFML/Graphics/RenderTarget.hpp>


auto TerrainRenderer::addLevel(Common::World::Level& level, const TextureManager& textureManager) -> void
{
	update(level, textureManager);
}

auto TerrainRenderer::clear() -> void
{
	m_level = Common::World::Level();
}

auto TerrainRenderer::update(Common::World::Level& level, const TextureManager& textureManager) -> void
{
	m_level = level;
	m_tile  = TerrainTile(m_level, textureManager);
}

auto TerrainRenderer::render(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) -> void
{
	renderTarget.draw(m_tile, renderStates);
}
