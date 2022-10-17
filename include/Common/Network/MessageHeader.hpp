#pragma once

#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include <entt/entity/entity.hpp>

namespace Common::Network
{

	struct MessageHeader
	{
		entt::entity entityID    = entt::null;
		std::uint64_t identifier = 0;
		Protocol protocol        = Protocol::TCP;
		MessageType type         = MessageType::None;
	};

} // namespace Common::Network