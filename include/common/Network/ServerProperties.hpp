#pragma once

#include <SFML/Network/IpAddress.hpp>
#include <cstdint>

namespace Common::Network
{
	const sf::IpAddress SERVER_ADDRESS = {127, 0, 0, 1};
	const std::uint16_t TCP_PORT       = 23720;
	const std::uint16_t UDP_PORT       = TCP_PORT;
} // namespace Common::Network