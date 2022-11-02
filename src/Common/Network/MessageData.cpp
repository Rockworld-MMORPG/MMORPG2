#include "Common/Network/MessageData.hpp"

#define PACK_MULTIPLE(VALUE_T, VALUE)                                                          \
	{                                                                                            \
		m_data.reserve(m_data.size() + sizeof(VALUE_T));                                           \
		for (auto i = 0; i < sizeof(VALUE_T); ++i)                                                 \
		{                                                                                          \
			m_data.emplace_back(static_cast<std::uint8_t>((VALUE) >> sizeof(std::uint8_t) * i * 8)); \
		}                                                                                          \
	}

#define UNPACK_MULTIPLE(VALUE_T, VALUE)                                                             \
	{                                                                                                 \
		using value_t         = VALUE_T;                                                                \
		(VALUE)               = 0;                                                                      \
		auto packedValueCount = (sizeof(value_t) / sizeof(std::uint8_t));                               \
		for (auto i = 0; i < packedValueCount; ++i)                                                     \
		{                                                                                               \
			(VALUE) |= static_cast<value_t>(m_data.at(m_readHead + i)) << (sizeof(std::uint8_t) * i * 8); \
		}                                                                                               \
		m_readHead += packedValueCount;                                                                 \
	}

namespace Common::Network
{

	MessageData::MessageData() :
	    m_readHead(0) {}

	auto MessageData::operator<<(const bool value) -> MessageData&
	{
		m_data.emplace_back(static_cast<std::uint8_t>(value));
		return *this;
	}

	auto MessageData::operator<<(const std::uint8_t value) -> MessageData&
	{
		m_data.emplace_back(value);
		return *this;
	}

	auto MessageData::operator<<(const std::uint16_t value) -> MessageData&
	{
		PACK_MULTIPLE(std::uint16_t, value);
		return *this;
	}

	auto MessageData::operator<<(const std::uint32_t value) -> MessageData&
	{
		PACK_MULTIPLE(std::uint32_t, value);
		return *this;
	}

	auto MessageData::operator<<(const std::uint64_t value) -> MessageData&
	{
		PACK_MULTIPLE(std::uint64_t, value);
		return *this;
	}

	auto MessageData::operator<<(const float value) -> MessageData&
	{
		// Should be true by definition but doesn't hurt to check
		static_assert(sizeof(float) == sizeof(std::uint32_t));
		const auto tempValue = *reinterpret_cast<const std::uint32_t*>(&value);
		return operator<<(tempValue);
	}

	auto MessageData::operator<<(const double value) -> MessageData&
	{
		// Should be true by definition but doesn't hurt to check
		static_assert(sizeof(double) == sizeof(std::uint64_t));
		const auto tempValue = *reinterpret_cast<const std::uint64_t*>(&value);
		return operator<<(tempValue);
	}

	auto MessageData::operator<<(entt::entity value) -> MessageData&
	{
		static_assert(sizeof(std::uint32_t) == sizeof(entt::entity));
		const auto tempValue = static_cast<std::uint32_t>(value);
		return operator<<(tempValue);
	}

	auto MessageData::operator<<(const std::string& value) -> MessageData&
	{
		m_data.reserve(m_data.size() + value.size() + sizeof(std::uint16_t));

		auto length = std::uint16_t(value.size());
		operator<<(length);

		for (const auto character : value)
		{
			m_data.emplace_back(static_cast<std::uint8_t>(character));
		}

		return *this;
	}

	auto MessageData::operator>>(bool& value) -> MessageData&
	{
		value = static_cast<bool>(m_data.at(m_readHead));
		m_readHead += 1;
		return *this;
	}

	auto MessageData::operator>>(std::uint8_t& value) -> MessageData&
	{
		value = m_data.at(m_readHead);
		m_readHead += 1;
		return *this;
	}


	auto MessageData::operator>>(std::uint16_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint16_t, value)
		return *this;
	}

	auto MessageData::operator>>(std::uint32_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint32_t, value)
		return *this;
	}

	auto MessageData::operator>>(std::uint64_t& value) -> MessageData&
	{
		UNPACK_MULTIPLE(std::uint64_t, value)
		return *this;
	}

	auto MessageData::operator>>(float& value) -> MessageData&
	{
		using value_t = std::uint32_t;
		static_assert(sizeof(value_t) == sizeof(float));

		value_t tempValue = 0x0000;
		UNPACK_MULTIPLE(value_t, tempValue)
		value = *reinterpret_cast<float*>(&tempValue);
		return *this;
	}

	auto MessageData::operator>>(double& value) -> MessageData&
	{
		using value_t = std::uint64_t;
		static_assert(sizeof(value_t) == sizeof(double));

		value_t tempValue = 0x0000;
		UNPACK_MULTIPLE(value_t, tempValue);
		value = *reinterpret_cast<double*>(&tempValue);
		return *this;
	}

	auto MessageData::operator>>(entt::entity& value) -> MessageData&
	{
		static_assert(sizeof(std::uint32_t) == sizeof(entt::entity));

		auto tempValue = std::uint32_t(0);
		UNPACK_MULTIPLE(std::uint32_t, tempValue)
		value = static_cast<entt::entity>(tempValue);
		return *this;
	}

	auto MessageData::operator>>(std::string& value) -> MessageData&
	{
		auto length = std::uint16_t(0);
		operator>>(length);

		value.resize(length);
		for (auto i = 0; i < length; ++i)
		{
			auto val = std::uint8_t(0);
			operator>>(val);
			value.at(i) = static_cast<char>(val);
		}

		return *this;
	}

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

#undef PACK_MULTIPLE
#undef UNPACK_MULTIPLE