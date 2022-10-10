#include "World/TerrainTile.hpp"
#include "World/Level.hpp"
#include <SFML/Graphics/RenderTarget.hpp>

namespace Client::World
{

	const auto TILE_SCALE = 64.0F;


	TerrainTile::TerrainTile(const Level& level)
	{
		m_vertexArray.setPrimitiveType(sf::PrimitiveType::Triangles);
		m_vertexArray.resize(LEVEL_WIDTH * LEVEL_HEIGHT * 6U);

		for (auto yPos = std::size_t(0); yPos < LEVEL_HEIGHT; ++yPos)
		{
			for (auto xPos = std::size_t(0); xPos < LEVEL_WIDTH; ++xPos)
			{
				auto index  = (xPos + yPos * LEVEL_WIDTH) * 6U;
				auto floatX = static_cast<float>(xPos);
				auto floatY = static_cast<float>(yPos);

				const auto xOffsets = std::array<float, 6>{0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 1.0F};
				const auto yOffsets = std::array<float, 6>{0.0F, 1.0F, 1.0F, 0.0F, 1.0F, 0.0F};

				for (auto i = std::size_t(0); i < 6; ++i)
				{
					m_vertexArray[index + i].position = sf::Vector2f(floatX + xOffsets.at(i), floatY + yOffsets.at(i)) * TILE_SCALE;
					switch (level.getTile(xPos, yPos))
					{
						case 0:
							m_vertexArray[index + i].color = sf::Color::White;
							break;
						case 1:
							m_vertexArray[index + i].color = sf::Color::Green;
							break;
						case 2:
							m_vertexArray[index + i].color = sf::Color::Blue;
							break;
						default:
							m_vertexArray[index + i].color = sf::Color::Red;
							break;
					}
				}
			}
		}
	}

	auto TerrainTile::draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void
	{
		renderTarget.draw(m_vertexArray, renderStates);
	}

} // namespace Client::World