#pragma once

#include "SFML/Network/Packet.hpp"
#include "common/Network/ClientID.hpp"
namespace Server
{

	struct Message
	{
		Common::Network::ClientID clientID;
		enum class Protocol
		{
			TCP,
			UDP
		} protocol;
		sf::Packet packet;

		Message(Common::Network::ClientID clientID, Protocol protocol);
	};

} // namespace Server