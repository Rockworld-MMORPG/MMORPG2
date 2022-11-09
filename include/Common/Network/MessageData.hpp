#pragma once

#include "Common/Export.hpp"
#include <cstdint>
#include <entt/entity/entity.hpp>
#include <string>
#include <vector>

namespace Common::Network
{

	/**
	 * \struct MessageData MessageData.hpp <Common/Network/MessageData.hpp>
	 * \brief Allows for data of any type to be conveniently bundled into a vector of bytes
	 *
	 */
	class COMMON_API MessageData
	{
	public:
		MessageData();

		auto operator<<(bool value) -> MessageData&;

		auto operator<<(std::uint8_t value) -> MessageData&;
		auto operator<<(std::uint16_t value) -> MessageData&;
		auto operator<<(std::uint32_t value) -> MessageData&;
		auto operator<<(std::uint64_t value) -> MessageData&;

		auto operator<<(std::int8_t value) -> MessageData&;
		auto operator<<(std::int16_t value) -> MessageData&;
		auto operator<<(std::int32_t value) -> MessageData&;
		auto operator<<(std::int64_t value) -> MessageData&;

		auto operator<<(float value) -> MessageData&;
		auto operator<<(double value) -> MessageData&;

		auto operator<<(entt::entity value) -> MessageData&;
		auto operator<<(const std::string& value) -> MessageData&;

		auto operator>>(bool& value) -> MessageData&;

		auto operator>>(std::uint8_t& value) -> MessageData&;
		auto operator>>(std::uint16_t& value) -> MessageData&;
		auto operator>>(std::uint32_t& value) -> MessageData&;
		auto operator>>(std::uint64_t& value) -> MessageData&;

		auto operator>>(std::int8_t& value) -> MessageData&;
		auto operator>>(std::int16_t& value) -> MessageData&;
		auto operator>>(std::int32_t& value) -> MessageData&;
		auto operator>>(std::int64_t& value) -> MessageData&;

		auto operator>>(float& value) -> MessageData&;
		auto operator>>(double& value) -> MessageData&;

		auto operator>>(entt::entity& value) -> MessageData&;
		auto operator>>(std::string& value) -> MessageData&;

		/**
		 * \brief Gets the size of the contained data
		 */
		[[nodiscard]] auto size() const -> std::size_t;

		/**
		 * \brief Gets a const void pointer to the contained data
		 */
		[[nodiscard]] auto data() const -> const void*;

		/**
		 * \brief Gets a void pointer to the contained data
		 */
		[[nodiscard]] auto data() -> void*;

		/**
		 * \brief Resizes the message data
		 *
		 * \param newSize The new size of the message data
		 */
		auto resize(std::size_t newSize) -> void;

	private:
		std::vector<std::uint8_t> m_data;
		std::size_t m_readHead;
	};

} // namespace Common::Network