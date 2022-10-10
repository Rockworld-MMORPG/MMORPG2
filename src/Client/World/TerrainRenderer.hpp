#pragma once

#include "World/TerrainTile.hpp"
namespace Client::World
{

	class TerrainRenderer
	{
	public:
		auto addLevel(Level& level) -> void;
		auto render(sf::RenderTarget& renderTarget) -> void;

	private:
		std::vector<TerrainTile> m_tiles;
	};

} // namespace Client::World