#pragma once

#include <Common/Network.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>

namespace Client
{

	/**
	 * \class NetworkManager NetworkManager.hpp "Network/NetworkManager.hpp"
	 * \brief Manages communications between the client and server
	 */
	class NetworkManager
	{
	public:
		/**
		 * \brief Construct a new Network Manager object
		 *
		 */
		NetworkManager();

		/**
		 * \brief Destroy the Network Manager object
		 *
		 */
		~NetworkManager();

		/**
		 * \brief Connect to the server and perform the handshake protocol
		 *
		 */
		auto connect() -> void;

		/**
		 * \brief Disconnect from the server
		 *
		 */
		auto disconnect() -> void;

		/**
		 * \brief Get whether the client is connected to the server or not
		 *
		 * \return true The client is connected to the server
		 * \return false The client is not connected to the server
		 */
		[[nodiscard]] auto isConnected() const -> bool;

		/**
		 * \brief Receive any messages from the server and send any queued messages back to it
		 *
		 */
		auto update() -> void;

		/**
		 * \brief Push a message into the message queue
		 *
		 * \param protocol The protocol to send the message with
		 * \param type The type of message to send
		 * \param messageData The data to send
		 */
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::MessageData& messageData) -> void;

		/**
		 * \brief Get the next message from the server
		 *
		 * \return std::optional<Common::Network::Message> An optional which may contain a message from the server
		 */
		auto getNextMessage() -> std::optional<Common::Network::Message>;

		/**
		 * \brief Get all the messages in the inbound queue, and clear it
		 *
		 * \return std::vector<Common::Network::Message> A vector containing all the messages in the inbound queue
		 */
		auto getMessages() -> std::vector<Common::Network::Message>;

		/**
		 * \brief Get the client's ID on the server
		 */
		[[nodiscard]] auto getClientID() -> entt::entity;

	private:
		/**
		 * \brief Get the identifier the next message should be sent with
		 */
		auto getNextMessageIdentifier() -> std::uint64_t;

		/**
		 * \brief Check whether a message is valid or not
		 *
		 * \param message The message to validate
		 * \return true The message is valid
		 * \return false The message is not valid
		 */
		auto validateMessage(Common::Network::Message& message) -> bool;

		/**
		 * \brief Send a message using the UDP socket
		 *
		 * \param message The message to send
		 */
		auto sendUDP(const Common::Network::Message& message) -> void;

		/**
		 * \brief Receive a message using the UDP socket, and push it into the inbound queue
		 *
		 */
		auto receiveUDP() -> void;

		/**
		 * \brief Send a message using the TCP socket
		 *
		 * \param message The message to send
		 */
		auto sendTCP(const Common::Network::Message& message) -> void;

		/**
		 * \brief Receive a message using the TCP socket, and push it into the inbound queue
		 *
		 */
		auto receiveTCP() -> void;

		bool m_isConnected;

		entt::entity m_clientID;
		std::uint64_t m_currentMessageIdentifier;
		std::uint64_t m_lastServerMessageIdentifier;

		sf::SocketSelector m_socketSelector;
		sf::UdpSocket m_udpSocket;
		sf::TcpSocket m_tcpSocket;

		Common::Network::MessageQueue<Common::Network::Message> m_messageQueue;
	};

} // namespace Client