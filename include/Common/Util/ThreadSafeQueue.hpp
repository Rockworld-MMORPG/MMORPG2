#pragma once

#include "Common/Export.hpp"
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace Common::Util
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

} // namespace Common::Util