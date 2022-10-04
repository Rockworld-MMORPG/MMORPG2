#include "MessageQueue.hpp"

namespace Server
{

	auto MessageQueue::pushInbound(Server::Message&& message) -> void
	{
		m_inboundQueue.emplace(std::forward<Server::Message>(message));
	}

	auto MessageQueue::getInbound() -> std::optional<Server::Message>
	{
		if (m_inboundQueue.empty())
		{
			return {};
		}

		auto message = m_inboundQueue.front();
		m_inboundQueue.pop();
		return {std::move(message)};
	}

	auto MessageQueue::getOutbound() -> std::optional<Server::Message>
	{
		if (m_outboundQueue.empty())
		{
			return {};
		}

		auto message = m_outboundQueue.front();
		m_outboundQueue.pop();
		return {std::move(message)};
	}

	auto MessageQueue::pushOutbound(Server::Message&& message) -> void
	{
		m_outboundQueue.emplace(std::forward<Server::Message>(message));
	}

} // namespace Server