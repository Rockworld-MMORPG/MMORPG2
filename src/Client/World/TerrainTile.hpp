#pragma once

#include "Graphics/TextureAtlas.hpp"
#include <Common/World/Level.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Client::World
{

	class TerrainTile : public sf::Drawable
	{
	public:
		TerrainTile(const Common::World::Level& level, const Graphics::TextureAtlas& textureAtlas);

		auto update(std::size_t xPosition, std::size_t yPosition, sf::FloatRect textureCoordinates) -> void;
		auto update(std::size_t index, sf::Vector2f position, sf::Color color, sf::Vector2f texCoords) -> void;

	private:
		auto draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void override;

		sf::VertexArray m_vertexArray;
	};

} // namespace Client::World