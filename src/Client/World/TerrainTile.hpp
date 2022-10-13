#pragma once

#include <Common/World/Level.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace Client::World
{
	class TerrainTile : public sf::Drawable
	{
	public:
		TerrainTile(const Common::World::Level& level);

	private:
		auto draw(sf::RenderTarget& renderTarget, const sf::RenderStates& renderStates) const -> void override;

		sf::VertexArray m_vertexArray;
	};

} // namespace Client::World