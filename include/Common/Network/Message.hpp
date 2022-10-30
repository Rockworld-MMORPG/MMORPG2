#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageHeader.hpp"

namespace Common::Network
{
	const std::size_t MAX_MESSAGE_LENGTH = 1 << 15;

	/**
	 * \struct Message Message.hpp <Common/Network/Message.hpp>
	 * \brief A piece, or pieces, of data that is sent over the network
	 *
	 */
	struct COMMON_API Message
	{
		MessageHeader header;
		MessageData data;

		/**
		 * \brief Packs the message into a vector of bytes
		 *
		 * \return A vector of bytes representing the message data
		 */
		[[nodiscard]] auto pack() const -> std::vector<std::uint8_t>;

		/**
		 * \brief Unpacks a byte array of length MAX_MESSAGE_LENGTH into the message
		 *
		 * \param buffer The byte array to unpack
		 * \param length The actual length of the message inside the buffer
		 */
		auto unpack(std::array<std::uint8_t, MAX_MESSAGE_LENGTH>& buffer, std::size_t length) -> void;
	};

} // namespace Common::Network