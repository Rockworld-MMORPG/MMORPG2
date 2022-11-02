#pragma once

#include <Common/World/Level.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

const auto TILE_SCALE = 32.0F;

class TextureManager;

class TerrainTile : public sf::Drawable
{
public:
	TerrainTile();
	TerrainTile(const Common::World::Level& level, const TextureManager& textureManager);

	auto update(std::size_t xPosition, std::size_t yPosition, sf::FloatRect textureCoordinates) -> void;
	auto update(std::size_t index, sf::Vector2f position, sf::Color color, sf::Vector2f texCoords) -> void;

private:
	auto draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void override;

	sf::VertexArray m_vertexArray;
};