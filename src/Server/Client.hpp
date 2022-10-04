#pragma once

#include "SFML/Network/TcpSocket.hpp"
#include <memory>

namespace Server
{

	struct Client
	{
		std::unique_ptr<sf::TcpSocket> tcpSocket = nullptr;
		std::uint32_t serverMessageNumber        = 0;
		std::uint32_t clientMessageNumber        = 0;
		std::uint16_t udpPort                    = 0;
	};

} // namespace Server