#pragma once

#include <SFML/Network/Packet.hpp>
#include <cstdint>

namespace Common
{

	enum class Message : std::uint32_t
	{
		None = 0,
		Connect,
		Disconnect,
		Movement,
		Position,
		Terminate
	};

	inline auto operator>>(sf::Packet& packet, Message& message) -> sf::Packet&
	{
		uint32_t messageCode = 0;
		packet >> messageCode;
		message = Common::Message(messageCode);
		return packet;
	}

	inline auto operator<<(sf::Packet& packet, Message& message) -> sf::Packet&
	{
		return packet << static_cast<std::uint32_t>(message);
	}

} // namespace Common