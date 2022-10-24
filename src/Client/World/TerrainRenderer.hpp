#pragma once

#include "World/TerrainTile.hpp"
#include <Common/World/Level.hpp>
#include <unordered_map>

namespace Client::World
{

	class TerrainRenderer
	{
	public:
		auto addLevel(std::uint32_t identifier, Common::World::Level& level, Graphics::TextureAtlas& textureAtlas) -> void;
		auto render(sf::RenderTarget& renderTarget, sf::Texture& atlas) -> void;

	private:
		std::unordered_map<std::uint32_t, TerrainTile> m_tiles;
	};

} // namespace Client::World