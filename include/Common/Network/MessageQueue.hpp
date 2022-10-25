#pragma once

#include "Common/Export.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Util/ThreadSafeQueue.hpp"

namespace Common::Network
{
	template<typename T>
	class MessageQueue
	{
	public:
		auto pushInbound(T&& value) -> void
		{
			m_inbound.push(std::forward<T&&>(value));
		}

		auto getInbound() -> std::optional<T>
		{
			return m_inbound.pop();
		}

		auto clearInbound() -> std::vector<T>
		{
			return m_inbound.clear();
		}

		auto pushOutbound(T&& value) -> void
		{
			m_outbound.push(std::forward<T&&>(value));
		}

		auto getOutbound() -> std::optional<T>
		{
			return m_outbound.pop();
		}

		auto clearOutbound() -> std::vector<T>
		{
			return m_outbound.clear();
		}

	private:
		Util::ThreadSafeQueue<T> m_inbound;
		Util::ThreadSafeQueue<T> m_outbound;
	};

} // namespace Common::Network