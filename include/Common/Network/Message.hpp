#pragma once

#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageHeader.hpp"
#include "spdlog/fmt/ostr.h"

namespace Common::Network
{
	const std::size_t MAX_MESSAGE_LENGTH = 1 << 15;

	struct Message
	{
		MessageHeader header;
		MessageData data;

		[[nodiscard]] auto pack() const -> std::vector<std::uint8_t>;
		auto unpack(std::array<std::uint8_t, MAX_MESSAGE_LENGTH>& buffer, std::size_t length) -> void;
	};

} // namespace Common::Network