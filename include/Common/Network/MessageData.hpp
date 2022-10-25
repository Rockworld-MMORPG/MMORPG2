#pragma once

#include "Common/Export.hpp"
#include <cstdint>
#include <entt/entity/entity.hpp>
#include <vector>

namespace Common::Network
{

	class COMMON_API MessageData
	{
	public:
		MessageData();

		auto operator<<(bool value) -> MessageData&;
		auto operator<<(std::uint8_t value) -> MessageData&;
		auto operator<<(std::uint16_t value) -> MessageData&;
		auto operator<<(std::uint32_t value) -> MessageData&;
		auto operator<<(std::uint64_t value) -> MessageData&;
		auto operator<<(float value) -> MessageData&;
		auto operator<<(double value) -> MessageData&;
		auto operator<<(entt::entity value) -> MessageData&;

		auto operator>>(bool& value) -> MessageData&;
		auto operator>>(std::uint8_t& value) -> MessageData&;
		auto operator>>(std::uint16_t& value) -> MessageData&;
		auto operator>>(std::uint32_t& value) -> MessageData&;
		auto operator>>(std::uint64_t& value) -> MessageData&;
		auto operator>>(float& value) -> MessageData&;
		auto operator>>(double& value) -> MessageData&;
		auto operator>>(entt::entity& value) -> MessageData&;

		[[nodiscard]] auto size() const -> std::size_t;
		[[nodiscard]] auto data() const -> const void*;
		[[nodiscard]] auto data() -> void*;
		auto resize(std::size_t newSize) -> void;

	private:
		std::vector<std::uint8_t> m_data;
		std::size_t m_readHead;
	};

} // namespace Common::Network