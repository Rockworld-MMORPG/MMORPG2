#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Client::World
{

	class Level;

	class TerrainTile : public sf::Drawable
	{
	public:
		TerrainTile(const Level& level);

	private:
		auto draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void override;

		sf::VertexArray m_vertexArray;
	};

} // namespace Client::World