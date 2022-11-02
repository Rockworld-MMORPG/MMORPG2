#pragma once

#include "Graphics/TextureAtlas.hpp"
#include <Common/World/Level.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Client::World
{

	/**
	 * \class TerrainTile TerrainTile.hpp "World/TerrainTile.hpp"
	 * \brief Graphical representation of level data
	 */
	class TerrainTile : public sf::Drawable
	{
	public:
		/**
		 * \brief Construct a new Terrain Tile object
		 *
		 * \param level The level to construct the terrain tile from
		 * \param textureAtlas The atlas containing the textures used by the level
		 */
		TerrainTile(const Common::World::Level& level, const Graphics::TextureAtlas& textureAtlas);

		/**
		 * \brief Update a quad of the terrain tile with a new texture
		 *
		 * \param xPosition The x position of the quad to update
		 * \param yPosition The y position of the quad to update
		 * \param textureCoordinates The new textures coordinates to apply to the quad
		 */
		auto update(std::size_t xPosition, std::size_t yPosition, sf::FloatRect textureCoordinates) -> void;

		/**
		 * \brief Update a vertex of the terrain tile
		 *
		 * \param index The index of the quad to update
		 * \param position The new position of the vertex
		 * \param color The new colour of the vertex
		 * \param texCoords The new texture coordinates of the vertex
		 */
		auto update(std::size_t index, sf::Vector2f position, sf::Color color, sf::Vector2f texCoords) -> void;

	private:
		/**
		 * \brief Draw the TerrainTile to a render target
		 *
		 * \param renderTarget The target to draw the TerrainTile to
		 * \param renderStates The render states to draw the TerrainTile with
		 */
		auto draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void override;

		sf::VertexArray m_vertexArray;
	};

} // namespace Client::World