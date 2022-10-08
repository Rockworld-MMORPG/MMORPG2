#pragma once

#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"

namespace Common::Network
{

	struct MessageHeader
	{
		ClientID clientID        = ClientID(-1);
		std::uint64_t identifier = 0;
		Protocol protocol        = Protocol::TCP;
		MessageType type         = MessageType::None;
	};

} // namespace Common::Network