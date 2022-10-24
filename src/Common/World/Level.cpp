#include "Common/World/Level.hpp"
#include <spdlog/spdlog.h>

namespace Common::World
{

	Level::Level() :
	    m_data() {}

	Level::Level(const std::vector<char>& data) :
	    m_data()
	{
		const auto TARGET_SIZE = LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t);
		if (data.size() != TARGET_SIZE)
		{
			spdlog::warn("Tried to create a level but the provided data is not the right size ({} / {}B)", data.size(), TARGET_SIZE);
			return;
		}

		std::memcpy(m_data.data(), data.data(), m_data.size() * sizeof(std::uint32_t));
	}

	Level::Level(const std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>& data)
	{
		auto index = std::size_t(0);
		for (auto& tile : m_data)
		{
			tile |= static_cast<std::uint32_t>(data.at(index++)) << UINT8_WIDTH * 0;
			tile |= static_cast<std::uint32_t>(data.at(index++)) << UINT8_WIDTH * 1;
			tile |= static_cast<std::uint32_t>(data.at(index++)) << UINT8_WIDTH * 2;
			tile |= static_cast<std::uint32_t>(data.at(index++)) << UINT8_WIDTH * 3;
		}
	}

	auto Level::setTile(const std::size_t xPosition, const std::size_t yPosition, const std::uint32_t identifier) -> void
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return;
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return;
		}

		m_data.at(yPosition * LEVEL_WIDTH + xPosition) = identifier;
	}

	auto Level::getTile(const std::size_t xPosition, const std::size_t yPosition) const -> std::uint32_t
	{
		if (xPosition > LEVEL_WIDTH)
		{
			return -1;
		}

		if (yPosition > LEVEL_HEIGHT)
		{
			return -1;
		}

		return m_data.at(yPosition * LEVEL_WIDTH + xPosition);
	}

	auto Level::data() const -> std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>
	{
		auto out = std::array<char, LEVEL_WIDTH * LEVEL_HEIGHT * sizeof(std::uint32_t)>();

		for (auto i = 0; i < out.size(); ++i)
		{
			out[i] = reinterpret_cast<const char*>(m_data.data())[i];
		}
		return out;
	}

} // namespace Common::World