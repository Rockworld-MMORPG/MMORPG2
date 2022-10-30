#pragma once

#include "Common/Export.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Util/ThreadSafeQueue.hpp"

namespace Common::Network
{
	/**
	 * \struct MessageQueue MessageQueue.hpp <Common/Network/MessageQueue.hpp>
	 * \brief A templated queue utility class containing both an inbound and outbound queue
	 *
	 * \tparam T The type the queues should contain
	 */
	template<typename T>
	class MessageQueue
	{
	public:
		/**
		 * \brief Pushes an rvalue of type T onto the inbound queue
		 *
		 * \param value The value to push onto the queue
		 */
		auto pushInbound(T&& value) -> void
		{
			m_inbound.push(std::forward<T&&>(value));
		}

		/**
		 * \brief Gets the value at the front of the inbound queue, if there is one
		 *
		 * \param value An optional which may contain a value of type T
		 */
		auto getInbound() -> std::optional<T>
		{
			return m_inbound.pop();
		}

		/**
		 * \brief Clears the inbound queue and returns the contained values
		 *
		 * \return std::vector<T> A vector containing all the values that were in the queue
		 */
		auto clearInbound() -> std::vector<T>
		{
			return m_inbound.clear();
		}

		/**
		 * \brief Pushes an rvalue of type T onto the outbound queue
		 *
		 * \param value The value to push onto the queue
		 */
		auto pushOutbound(T&& value) -> void
		{
			m_outbound.push(std::forward<T&&>(value));
		}

		/**
		 * \brief Gets the value at the front of the outbound queue, if there is one
		 *
		 * \param value An optional which may contain a value of type T
		 */
		auto getOutbound() -> std::optional<T>
		{
			return m_outbound.pop();
		}

		/**
		 * \brief Clears the outbound queue and returns the contained values
		 *
		 * \return std::vector<T> A vector containing all the values that were in the queue
		 */
		auto clearOutbound() -> std::vector<T>
		{
			return m_outbound.clear();
		}

	private:
		Util::ThreadSafeQueue<T> m_inbound;
		Util::ThreadSafeQueue<T> m_outbound;
	};

} // namespace Common::Network