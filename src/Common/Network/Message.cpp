#include "Common/Network/Message.hpp"
#include "Common/Network/MessageHeader.hpp"
#include "spdlog/fmt/ostr.h"
#include <array>
#include <cstring>

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

	auto operator<<(std::ostream& ostream, const Message& message) -> std::ostream&
	{
		const auto* val = static_cast<const std::uint8_t*>(message.data.data());
		ostream << "--- Message ---\n";
		ostream << fmt::format("\tHeader:\n\t\tClientID: {}\n\t\tIdentifier: {}\n\t\tProtocol: {}\n\t\tType: {:X}\n", message.header.clientID.get(), message.header.identifier, message.header.protocol == Protocol::TCP ? "TCP" : "UDP", static_cast<std::uint8_t>(message.header.type));
		ostream << "\tData:\n\t\t";
		for (auto i = 0; i < message.data.size(); ++i)
		{
			ostream << fmt::format("{:02X}", val[i]);
			if (i % 4 == 0 && i > 0)
			{
				ostream << " ";
			}
		}
		return ostream << "\n";
	}

} // namespace Common::Network