#pragma once

#include "Common/Export.hpp"
#include "Common/World/Tile.hpp"
#include <array>
#include <vector>

namespace Common::World
{

	const auto LEVEL_WIDTH  = std::size_t(256);
	const auto LEVEL_HEIGHT = std::size_t(256);

	/**
	 * \class Level Level.hpp <Common/World/Level.hpp>
	 * \brief Data representing a level in the game world
	 *
	 */
	class COMMON_API Level
	{
	public:
		/**
		 * \brief Construct an empty level
		 *
		 */
		Level();

		/**
		 * \brief Construct a level from a byte array of the correct length
		 *
		 * \param data The byte array to construct the level from
		 */
		Level(const std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>& data);

		/**
		 * \brief Construct a level from a vector of the correct length
		 *
		 * \param data The vector of bytes to construct the level from
		 */
		Level(const std::vector<char>& data);

		/**
		 * \brief Sets the tile at a position to the given type
		 *
		 * \param xPosition The x position of the tile
		 * \param yPosition The y position of the tile
		 * \param identifier The identifier of the tile type
		 */
		auto setTile(std::size_t xPosition, std::size_t yPosition, std::uint32_t identifier) -> void;

		/**
		 * \brief Gets the tile type identifier of the tile at a position
		 *
		 * \param xPosition The x position of the tile
		 * \param yPosition The y position of the tile
		 * \return std::uint32_t The tile type identifier of the tile, or -1 if the coordinates were out-of-range
		 */
		[[nodiscard]] auto getTile(std::size_t xPosition, std::size_t yPosition) const -> std::uint32_t;

		/**
		 * \brief Creates a byte array representing the level
		 */
		auto data() const -> std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>;

	private:
		std::array<std::uint32_t, LEVEL_WIDTH * LEVEL_HEIGHT> m_data;
	};

} // namespace Common::World