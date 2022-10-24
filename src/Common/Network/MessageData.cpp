#include "Common/Network/MessageData.hpp"
#include <cstdint>

namespace Common::Network
{

	MessageData::MessageData() :
	    m_readHead(0) {}

	auto MessageData::operator<<(const bool value) -> MessageData&
	{
		m_data.emplace_back(value ? std::uint8_t(0xFF) : std::uint8_t(0xAA));
		return *this;
	}

	auto MessageData::operator<<(const std::uint8_t value) -> MessageData&
	{
		m_data.emplace_back(value);
		return *this;
	}

	auto MessageData::operator<<(const std::uint16_t value) -> MessageData&
	{
		m_data.reserve(m_data.size() + 2);
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 0));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 1));
		return *this;
	}

	auto MessageData::operator<<(const std::uint32_t value) -> MessageData&
	{
		m_data.reserve(m_data.size() + 4);
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 0));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 1));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 2));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 3));
		return *this;
	}

	auto MessageData::operator<<(const std::uint64_t value) -> MessageData&
	{
		m_data.reserve(m_data.size() + 8);
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 0));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 1));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 2));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 3));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 4));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 5));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 6));
		m_data.emplace_back(static_cast<std::uint8_t>(value >> sizeof(std::uint8_t) * 7));
		return *this;
	}

	auto MessageData::operator<<(const float value) -> MessageData&
	{
		const auto tempValue = *reinterpret_cast<const std::uint32_t*>(&value);

		m_data.reserve(m_data.size() + 4);
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint32_t>(tempValue) >> sizeof(std::uint8_t) * 0));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint32_t>(tempValue) >> sizeof(std::uint8_t) * 1));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint32_t>(tempValue) >> sizeof(std::uint8_t) * 2));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint32_t>(tempValue) >> sizeof(std::uint8_t) * 3));
		return *this;
	}

	auto MessageData::operator<<(const double value) -> MessageData&
	{
		const auto tempValue = *reinterpret_cast<const std::uint64_t*>(&value);

		m_data.reserve(m_data.size() + 8);
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 0));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 1));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 2));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 3));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 4));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 5));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 6));
		m_data.emplace_back(static_cast<std::uint8_t>(static_cast<std::uint64_t>(tempValue) >> sizeof(std::uint8_t) * 7));
		return *this;
	}

	auto MessageData::operator<<(entt::entity value) -> MessageData&
	{
		static_assert(sizeof(std::uint32_t) == sizeof(entt::entity));
		return operator<<(static_cast<std::uint32_t>(value));
	}


	auto MessageData::operator>>(bool& value) -> MessageData&
	{
		value = !(m_data.at(m_readHead) != 0xFF);
		m_readHead += 1;
		return *this;
	}

	auto MessageData::operator>>(std::uint8_t& value) -> MessageData&
	{
		value = m_data.at(m_readHead);
		m_readHead += 1;
		return *this;
	}

#define UNPACK_MULTIPLE(VALUE_T, VALUE)                                                     \
	{                                                                                         \
		using value_t         = VALUE_T;                                                        \
		value                 = 0x0000;                                                         \
		auto packedValueCount = (sizeof(value_t) / sizeof(std::uint8_t));                       \
		for (auto i = 0; i < packedValueCount; ++i)                                             \
		{                                                                                       \
			value |= static_cast<value_t>(m_data.at(m_readHead + i)) << sizeof(std::uint8_t) * i; \
		}                                                                                       \
		m_readHead += packedValueCount;                                                         \
		return *this;                                                                           \
	}

	auto MessageData::operator>>(std::uint16_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint16_t, value)
	}

	auto MessageData::operator>>(std::uint32_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint32_t, value)
	}

	auto MessageData::operator>>(std::uint64_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint64_t, value)
	}

	auto MessageData::operator>>(float& value) -> MessageData&
	{
		using value_t         = std::uint32_t;
		value_t tempValue     = 0x0000;
		auto packedValueCount = (sizeof(value_t) / sizeof(std::uint8_t));
		for (auto i = 0; i < packedValueCount; ++i)
		{
			tempValue |= static_cast<value_t>(m_data.at(m_readHead + i)) << sizeof(std::uint8_t) * i;
		}

		m_readHead += packedValueCount;
		value = *reinterpret_cast<float*>(&tempValue);
		return *this;
	}

	auto MessageData::operator>>(double& value) -> MessageData&
	{
		using value_t         = std::uint64_t;
		value_t tempValue     = 0x0000;
		auto packedValueCount = (sizeof(value_t) / sizeof(std::uint8_t));
		for (auto i = 0; i < packedValueCount; ++i)
		{
			tempValue |= static_cast<value_t>(m_data.at(m_readHead + i)) << sizeof(std::uint8_t) * i;
		}

		m_readHead += packedValueCount;
		value = *reinterpret_cast<float*>(&tempValue);
		return *this;
	}

	auto MessageData::operator>>(entt::entity& value) -> MessageData&
	{
		static_assert(sizeof(std::uint32_t) == sizeof(entt::entity));
		auto temp             = std::uint32_t(0);
		auto packedValueCount = (sizeof(std::uint32_t) / sizeof(std::uint8_t));
		for (auto i = 0; i < packedValueCount; ++i)
		{
			temp |= static_cast<std::uint32_t>(m_data.at(m_readHead + i)) << sizeof(std::uint8_t) * i;
		}
		m_readHead += packedValueCount;
		value = static_cast<entt::entity>(temp);
		return *this;
	}

#undef UNPACK_MULTIPLE

	auto MessageData::size() const -> std::size_t
	{
		return m_data.size();
	}

	auto MessageData::data() const -> const void*
	{
		return m_data.data();
	}

	auto MessageData::data() -> void*
	{
		return m_data.data();
	}

	auto MessageData::resize(std::size_t newSize) -> void
	{
		m_data.resize(newSize);
	}


} // namespace Common::Network