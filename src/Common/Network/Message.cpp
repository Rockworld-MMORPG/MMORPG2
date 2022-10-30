#include "Common/Network/Message.hpp"
#include "Common/Network/MessageHeader.hpp"

namespace Common::Network
{

	auto Message::pack() const -> std::vector<std::uint8_t>
	{
		auto buffer = std::vector<std::uint8_t>();
		buffer.resize(data.size() + sizeof(Common::Network::MessageHeader));

		std::memcpy(buffer.data(), &header, sizeof(MessageHeader));
		std::memcpy(buffer.data() + sizeof(MessageHeader), data.data(), data.size());

		return buffer;
	}

	auto Message::unpack(std::array<std::uint8_t, MAX_MESSAGE_LENGTH>& buffer, const std::size_t length) -> void
	{
		auto dataLength = length - sizeof(MessageHeader);
		data.resize(dataLength);

		std::memcpy(&header, buffer.data(), sizeof(MessageHeader));
		std::memcpy(data.data(), buffer.data() + sizeof(MessageHeader), data.size());
	}

} // namespace Common::Network