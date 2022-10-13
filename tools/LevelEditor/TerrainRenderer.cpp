#include "TerrainRenderer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>


auto TerrainRenderer::addLevel(Common::World::Level& level) -> void
{
	m_tiles.emplace_back(level);
}

auto TerrainRenderer::clear() -> void
{
	m_tiles.clear();
}

auto TerrainRenderer::render(sf::RenderTarget& renderTarget) -> void
{
	for (auto& tile : m_tiles)
	{
		renderTarget.draw(tile);
	}
}
