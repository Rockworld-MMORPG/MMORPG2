#include "TerrainTile.hpp"
#include "SFML/Graphics/Color.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace Client::World
{
	const auto VERTICES_PER_TILE = 6U;

	/*
	  0 - Top left
	  1 - Bottom left
	  2 - Bottom right
	  3 - Top left
	  4 - Bottom right
	  5 - Top right
	*/
	const auto xOffsets = std::array<float, 6>{0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F};
	const auto yOffsets = std::array<float, 6>{0.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F};

	TerrainTile::TerrainTile(const Common::World::Level& level, const Graphics::TextureAtlas& textureAtlas)
	{
		auto levelData = level.data();

		m_vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
		m_vertexArray.resize(Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * VERTICES_PER_TILE);

		for (auto yPos = std::size_t(0); yPos < Common::World::LEVEL_HEIGHT; ++yPos)
		{
			for (auto xPos = std::size_t(0); xPos < Common::World::LEVEL_WIDTH; ++xPos)
			{
				auto index  = (xPos + yPos * Common::World::LEVEL_WIDTH) * VERTICES_PER_TILE;
				auto floatX = static_cast<float>(xPos);
				auto floatY = static_cast<float>(yPos);

				for (auto i = std::size_t(0); i < VERTICES_PER_TILE; ++i)
				{
					m_vertexArray[index + i].position = sf::Vector2f(floatX + xOffsets.at(i), floatY + yOffsets.at(i)) * static_cast<float>(textureAtlas.getTextureSize().x);
				}

				auto textureCoordinates = textureAtlas.getTextureCoordinates(level.getTile(xPos, yPos));

				auto topLeft     = sf::Vector2f(textureCoordinates.left, textureCoordinates.top);
				auto bottomLeft  = sf::Vector2f(textureCoordinates.left, textureCoordinates.top + textureCoordinates.height);
				auto topRight    = sf::Vector2f(textureCoordinates.left + textureCoordinates.width, textureCoordinates.top);
				auto bottomRight = sf::Vector2f(textureCoordinates.left + textureCoordinates.width, textureCoordinates.top + textureCoordinates.height);

				m_vertexArray[index + 0].texCoords = topLeft;
				m_vertexArray[index + 1].texCoords = bottomLeft;
				m_vertexArray[index + 2].texCoords = bottomRight;

				m_vertexArray[index + 3].texCoords = topLeft;
				m_vertexArray[index + 4].texCoords = bottomRight;
				m_vertexArray[index + 5].texCoords = topRight;
			}
		}
	}

	auto TerrainTile::update(std::size_t xPosition, std::size_t yPosition, sf::FloatRect textureCoordinates) -> void
	{
		auto index = (xPosition + yPosition * Common::World::LEVEL_WIDTH) * VERTICES_PER_TILE;

		m_vertexArray[index + 0].texCoords = sf::Vector2f(textureCoordinates.left, textureCoordinates.top);
		m_vertexArray[index + 1].texCoords = sf::Vector2f(textureCoordinates.left, textureCoordinates.top + textureCoordinates.height);
		m_vertexArray[index + 2].texCoords = sf::Vector2f(textureCoordinates.left + textureCoordinates.width, textureCoordinates.top + textureCoordinates.height);

		m_vertexArray[index + 3].texCoords = sf::Vector2f(textureCoordinates.left, textureCoordinates.top);
		m_vertexArray[index + 4].texCoords = sf::Vector2f(textureCoordinates.left + textureCoordinates.width, textureCoordinates.top + textureCoordinates.height);
		m_vertexArray[index + 5].texCoords = sf::Vector2f(textureCoordinates.left, textureCoordinates.top + textureCoordinates.height);
	}

	auto TerrainTile::update(std::size_t index, sf::Vector2f position, sf::Color color, sf::Vector2f texCoords) -> void
	{
		if (index > Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * VERTICES_PER_TILE)
		{
			return;
		}

		m_vertexArray[index].texCoords = texCoords;
		m_vertexArray[index].color     = color;
		m_vertexArray[index].position  = position;
	}

	auto TerrainTile::draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void
	{
		renderTarget.draw(m_vertexArray, renderStates);
	}

} // namespace Client::World