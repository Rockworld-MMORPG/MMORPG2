#include "Common/World/Level.hpp"

namespace Common::World
{

	Level::Level() :
	    m_data(){};

	auto Level::setTile(const std::size_t xPosition, const std::size_t yPosition, const Tile tile) -> void
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return;
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return;
		}

		m_data.at(yPosition * LEVEL_WIDTH + xPosition) = tile;
	}

	auto Level::getTile(const std::size_t xPosition, const std::size_t yPosition) const -> Tile
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return {};
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return {};
		}

		return m_data.at(yPosition * LEVEL_WIDTH + xPosition);
	}

} // namespace Common::World