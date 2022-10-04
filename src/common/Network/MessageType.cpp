#include "common/Network/MessageType.hpp"
#include <SFML/Network/Packet.hpp>

namespace Common::Network
{

	auto operator<<(sf::Packet& packet, MessageType messageType) -> sf::Packet&
	{
		return packet << static_cast<std::uint8_t>(messageType);
	}

	auto operator>>(sf::Packet& packet, MessageType& messageType) -> sf::Packet&
	{
		std::uint8_t mType = 0;
		packet >> mType;
		messageType = static_cast<MessageType>(mType);
		return packet;
	}

} // namespace Common::Network