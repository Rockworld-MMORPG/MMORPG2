#pragma once
#include <optional>
#include <queue>

namespace Common::Network
{

	template<typename T>
	class MessageQueue
	{
	public:
		auto pushInbound(T&& message) -> void
		{
			m_inboundQueue.emplace(std::forward<T>(message));
		}
		auto getInbound() -> std::optional<T>
		{
			if (m_inboundQueue.empty())
			{
				return {};
			}

			auto message = m_inboundQueue.front();
			m_inboundQueue.pop();
			return {std::move(message)};
		}

		auto pushOutbound(T&& message) -> void
		{
			m_outboundQueue.emplace(std::forward<T>(message));
		}

		auto getOutbound() -> std::optional<T>
		{
			if (m_outboundQueue.empty())
			{
				return {};
			}

			auto message = m_outboundQueue.front();
			m_outboundQueue.pop();
			return {std::move(message)};
		}

	private:
		std::queue<T> m_inboundQueue;
		std::queue<T> m_outboundQueue;
	};

} // namespace Common::Network