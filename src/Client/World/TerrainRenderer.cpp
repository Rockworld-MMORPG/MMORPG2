#include "World/TerrainRenderer.hpp"

namespace Client::World
{

	auto TerrainRenderer::addLevel(std::uint32_t identifier, Common::World::Level& level, Graphics::TextureAtlas& textureAtlas) -> void
	{
		auto tile = TerrainTile(level, textureAtlas);
		m_tiles.emplace(identifier, std::move(tile));
	}

	auto TerrainRenderer::render(sf::RenderTarget& renderTarget, sf::Texture& atlas) -> void
	{
		auto renderStates    = sf::RenderStates::Default;
		renderStates.texture = &atlas;

		for (auto& [id, tile] : m_tiles)
		{
			renderTarget.draw(tile, renderStates);
		}
	}

} // namespace Client::World