#pragma once

#include "World/TerrainTile.hpp"
#include <Common/World/Level.hpp>

namespace Client::World
{

	class TerrainRenderer
	{
	public:
		auto addLevel(Common::World::Level& level) -> void;
		auto render(sf::RenderTarget& renderTarget) -> void;

	private:
		std::vector<TerrainTile> m_tiles;
	};

} // namespace Client::World