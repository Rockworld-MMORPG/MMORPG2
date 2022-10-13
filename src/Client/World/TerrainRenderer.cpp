#include "World/TerrainRenderer.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace Client::World
{

	auto TerrainRenderer::addLevel(Common::World::Level& level) -> void
	{
		m_tiles.emplace_back(level);
	}

	auto TerrainRenderer::render(sf::RenderTarget& renderTarget) -> void
	{
		for (auto& tile : m_tiles)
		{
			renderTarget.draw(tile);
		}
	}

} // namespace Client::World