#pragma once
#include <mutex>
#include <optional>
#include <queue>

namespace Common::Network
{

	template<typename T>
	class ThreadSafeQueue
	{
	public:
		auto push(T&& value) -> void
		{
			std::scoped_lock<std::mutex> lock{m_mutex};
			m_queue.emplace(std::forward<T&&>(value));
		}

		auto pop() -> std::optional<T>
		{
			std::scoped_lock<std::mutex> lock{m_mutex};
			if (m_queue.empty())
			{
				return {};
			}

			auto value = m_queue.front();
			m_queue.pop();
			return value;
		}

		auto clear() -> std::vector<T>
		{
			std::scoped_lock<std::mutex> lock{m_mutex};
			if (m_queue.empty())
			{
				return {};
			}

			std::vector<T> vec;
			vec.reserve(m_queue.size());
			for (auto i = 0; i < m_queue.size(); ++i)
			{
				vec.emplace_back(std::move(m_queue.front()));
				m_queue.pop();
			}

			return vec;
		}

	private:
		std::queue<T> m_queue;
		std::mutex m_mutex;
	};

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
		ThreadSafeQueue<T> m_inbound;
		ThreadSafeQueue<T> m_outbound;
	};

} // namespace Common::Network