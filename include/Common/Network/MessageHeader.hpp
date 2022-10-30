#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include <entt/entity/entity.hpp>

namespace Common::Network
{

	/**
	 * \struct MessageHeader MessageHeader.hpp <Common/Network/MessageHeader.hpp>
	 * \brief The data required to send a message over the network
	 *
	 */
	struct MessageHeader
	{
		entt::entity entityID    = entt::null;
		std::uint64_t identifier = 0;
		Protocol protocol        = Protocol::TCP;
		MessageType type         = MessageType::None;
	};

} // namespace Common::Network