#pragma once

#include "World/TerrainTile.hpp"
#include <Common/World/Level.hpp>
#include <unordered_map>

namespace Client::World
{

	/**
	 * \class TerrainRenderer TerrainRenderer.hpp "World/TerrainRenderer.hpp"
	 * \brief Used to render terrain tiles, created from level data
	 */
	class TerrainRenderer
	{
	public:
		/**
		 * \brief Add a level to the terrain renderer to render
		 *
		 * \param identifier The identifier to be given to the terrain tile
		 * \param level The level data to create a terrain tile from
		 * \param textureAtlas The atlas containing the textures used by the level
		 */
		auto addLevel(std::uint32_t identifier, Common::World::Level& level, Graphics::TextureAtlas& textureAtlas) -> void;

		/**
		 * \brief Render all terrain tiles to the target
		 *
		 * \param renderTarget The target to render the tiles to
		 * \param atlas The atlas containing the terrain textures
		 */
		auto render(sf::RenderTarget& renderTarget, sf::Texture& atlas) -> void;

	private:
		std::unordered_map<std::uint32_t, TerrainTile> m_tiles;
	};

} // namespace Client::World