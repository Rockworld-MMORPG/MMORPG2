#pragma once

#include "Common/Export.hpp"
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

namespace Common::Util
{

	/**
	 * \class ThreadSafeQueue ThreadSafeQueue.hpp <Common/Util/ThreadSafeQueue.hpp>
	 * \brief A queue with thread-safe push, pop, and clear operations
	 *
	 * \tparam T The type the queue should contain
	 */
	template<typename T>
	class ThreadSafeQueue
	{
	public:
		/**
		 * \brief Pushes an rvalue of type T into the queue
		 *
		 * \param value The rvalue to push into the queue
		 */
		auto push(T&& value) -> void
		{
			std::scoped_lock<std::mutex> lock{m_mutex};
			m_queue.emplace(std::forward<T&&>(value));
		}

		/**
		 * \brief Pops the value at the front of the queue and returns it, if it exists
		 *
		 * \return std::optional<T> An optional which may contain a value of type T
		 */
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

		/**
		 * \brief Clears the queue and returns all the values previously contained in it
		 *
		 * \return std::vector<T> A vector containing all the previous contained values
		 */
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