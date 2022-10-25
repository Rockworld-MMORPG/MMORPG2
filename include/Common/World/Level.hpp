#pragma once

#include "Common/Export.hpp"
#include "Common/World/Tile.hpp"
#include <array>
#include <vector>

namespace Common::World
{

	const auto LEVEL_WIDTH  = std::size_t(256);
	const auto LEVEL_HEIGHT = std::size_t(256);

	class COMMON_API Level
	{
	public:
		Level();
		Level(const std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>& data);
		Level(const std::vector<char>& data);

		auto setTile(std::size_t xPosition, std::size_t yPosition, std::uint32_t identifier) -> void;
		[[nodiscard]] auto getTile(std::size_t xPosition, std::size_t yPosition) const -> std::uint32_t;

		auto data() const -> std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>;

	private:
		std::array<std::uint32_t, LEVEL_WIDTH * LEVEL_HEIGHT> m_data;
	};

} // namespace Common::World