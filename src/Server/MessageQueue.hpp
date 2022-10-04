#pragma once

#include "Message.hpp"
#include <optional>
#include <queue>

namespace Server
{

	class MessageQueue
	{
	public:
		auto pushInbound(Server::Message&& message) -> void;
		auto getInbound() -> std::optional<Server::Message>;

		auto pushOutbound(Server::Message&& message) -> void;
		auto getOutbound() -> std::optional<Server::Message>;

	private:
		std::queue<Server::Message> m_inboundQueue;
		std::queue<Server::Message> m_outboundQueue;
	};

} // namespace Server