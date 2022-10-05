#pragma once

#include <cstdint>

namespace sf
{
	class Packet;
}

namespace Common::Network
{

	enum class MessageType : std::uint8_t
	{
		// 0x0 - Control messages
		None       = 0x00,
		Connect    = 0x01,
		Disconnect = 0x02,
		Terminate  = 0x03,

		// 0x1 - Gameplay messages
		CreateEntity  = 0x10,
		DestroyEntity = 0x11,
		Movement      = 0x12,
		Position      = 0x13,
	};

	auto operator<<(sf::Packet& packet, MessageType messageType) -> sf::Packet&;
	auto operator>>(sf::Packet& packet, MessageType& messageType) -> sf::Packet&;

}; // namespace Common::Network