#include "TerrainTile.hpp"
#include "SFML/Graphics/Color.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

const auto VERTICES_PER_TILE = 6U;


TerrainTile::TerrainTile(const Common::World::Level& level)
{
	m_vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
	m_vertexArray.resize(Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * VERTICES_PER_TILE);

	for (auto yPos = std::size_t(0); yPos < Common::World::LEVEL_HEIGHT; ++yPos)
	{
		for (auto xPos = std::size_t(0); xPos < Common::World::LEVEL_WIDTH; ++xPos)
		{
			auto index  = (xPos + yPos * Common::World::LEVEL_WIDTH) * VERTICES_PER_TILE;
			auto floatX = static_cast<float>(xPos);
			auto floatY = static_cast<float>(yPos);

			const auto xOffsets = std::array<float, 6>{0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F};
			const auto yOffsets = std::array<float, 6>{0.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F};

			for (auto i = std::size_t(0); i < VERTICES_PER_TILE; ++i)
			{
				m_vertexArray[index + i].position = sf::Vector2f(floatX + xOffsets.at(i), floatY + yOffsets.at(i)) * TILE_SCALE;
				m_vertexArray[index + i].color    = sf::Color(level.getTile(xPos, yPos).type[0], level.getTile(xPos, yPos).type[1], level.getTile(xPos, yPos).type[2]);
			}
		}
	}
}

auto TerrainTile::draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void
{
	renderTarget.draw(m_vertexArray, renderStates);
}