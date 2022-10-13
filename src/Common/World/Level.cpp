#include "Common/World/Level.hpp"

namespace Common::World
{

	Level::Level() :
	    m_data(){};

	auto Level::setTile(const std::size_t xPosition, const std::size_t yPosition, const std::uint8_t value) -> void
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return;
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return;
		}

		m_data.at(yPosition * LEVEL_WIDTH + xPosition).type = value;
	}

	auto Level::getTile(const std::size_t xPosition, const std::size_t yPosition) const -> std::uint8_t
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return -1;
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return -1;
		}

		return m_data.at(yPosition * LEVEL_WIDTH + xPosition).type;
	}

} // namespace Common::World