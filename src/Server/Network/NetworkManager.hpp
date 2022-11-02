#pragma once

#include "Common/Network/Crypto.hpp"
#include "Network/Client.hpp"
#include "Server/Manager.hpp"
#include <Common/Network.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <entt/entity/entity.hpp>
#include <list>
#include <unordered_map>

namespace Server
{
	/**
	 * \class NetworkManager
	 * \brief Manages communication with all clients
	 */
	class NetworkManager : public Manager
	{
	public:
		/**
		 * \brief Construct a new Network Manager object
		 *
		 * \param server The server object the network manager is contained within
		 */
		NetworkManager(Server& server);

		/**
		 * \brief Destroy the Network Manager object
		 *
		 */
		~NetworkManager() override;

		/**
		 * \brief Initialise the Network Manager
		 *
		 * \return true The Network Manager was successfully initialised
		 * \return false The Network Manager failed to initialise
		 */
		auto init() -> bool;

		/**
		 * \brief Shut down the network manager and terminate all connections
		 *
		 */
		auto shutdown() -> void;

		/**
		 * \brief Receive any messages from clients, send any message in the outbound queue, and disconnect any clients pending disconnection
		 *
		 * \param maxSelectorWaitTime The maximum amount of time to wait to receive a message from a client
		 */
		auto update(sf::Time maxSelectorWaitTime) -> void;

		/**
		 * \brief Get the next message in the inbound queue
		 *
		 * \return std::optional<Common::Network::Message> An optional which may contain a Message
		 */
		auto getNextMessage() -> std::optional<Common::Network::Message>;

		/**
		 * \brief Get all the messages in the inbound queue, and clear it
		 *
		 * \return std::vector<Common::Network::Message> A vector containing all the messages in the inbound queue
		 */
		auto getMessages() -> std::vector<Common::Network::Message>;

		/**
		 * \brief Push a message into the outbound queue, to a specific client
		 *
		 * \param protocol The protocol to send the message over
		 * \param type The type of message to send
		 * \param entityID The ID of the client to send the message to
		 * \param data The data to send to the client
		 */
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, entt::entity entityID, Common::Network::MessageData& data) -> void;

		/**
		 * \brief Push a message into the outbound queue, to all connected clients
		 *
		 * \param protocol The protocol to send the message over
		 * \param type The type of message to send
		 * \param data The data to send to the clients
		 */
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::MessageData& data) -> void;

		/**
		 * \brief Resolve a client's ID based on their IP address and port
		 *
		 * \param ipAddress The remote address of the client
		 * \param port The port the client used to send the message
		 * \return std::optional<entt::entity> An optional which may contain the ID of the client, if they could be resolved
		 */
		auto resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<entt::entity>;

		/**
		 * \brief Set the UDP port used to communicate with a client
		 *
		 * \param entityID The ID of the client
		 * \param udpPort The UDP port to use to communicate
		 */
		auto setClientUdpPort(entt::entity entityID, std::uint16_t udpPort) -> void;

		/**
		 * \brief Designates a client to be disconnected
		 *
		 * \param entityID The ID of the client to disconnect
		 */
		auto markForDisconnect(entt::entity entityID) -> void;

	private:
		/**
		 * \brief Generate a new client ID
		 *
		 * \return entt::entity The new client ID
		 */
		auto generateClientID() -> entt::entity;

		/**
		 * \brief Accept a connection as a new client
		 *
		 */
		auto acceptNewConnection() -> void;

		/**
		 * \brief Close the connection with a client
		 *
		 * \param entityID The ID of the client to close the connection with
		 */
		auto closeConnection(entt::entity entityID) -> void;

		/**
		 * \brief Disconnect any clients awaiting disconnection
		 *
		 */
		auto disconnectClients() -> void;

		/**
		 * \brief Get the identifier the next message should be sent with
		 */
		auto getNextMessageIdentifier() -> std::uint64_t;

		/**
		 * \brief Check whether an incoming message is valid or not
		 *
		 * \param entityID The ID of the client that sent the message
		 * \param header The header of the message
		 * \return true The message is valid
		 * \return false The message is not valid
		 */
		auto validateIncomingMessage(entt::entity entityID, Common::Network::MessageHeader& header) -> bool;

		/**
		 * \brief Send a message using the UDP socket
		 *
		 * \param message The message to send
		 */
		auto sendUDP(Common::Network::Message& message) -> void;

		/**
		 * \brief Receive a message using the UDP socket, and push it into the inbound message queue
		 *
		 */
		auto receiveUDP() -> void;

		/**
		 * \brief Send a message using a TCP socket
		 *
		 * \param message The message to send
		 */
		auto sendTCP(Common::Network::Message& message) -> void;

		/**
		 * \brief Receive a message from a specific client using their TCP socket
		 *
		 * \param entityID The ID of the client to receive the message from
		 * \param client The client data
		 */
		auto receiveTCP(entt::entity entityID, Client& client) -> void;

		sf::SocketSelector m_socketSelector;
		sf::TcpListener m_tcpListener;
		sf::UdpSocket m_udpSocket;

		std::unordered_map<std::uint64_t, entt::entity> m_clientIPMap;
		std::list<entt::entity> m_clientsPendingDisconnection;

		Common::Network::PublicKeyCryptographer m_cryptographer;

		Common::Network::MessageQueue<Common::Network::Message> m_messageQueue;
		std::uint64_t m_currentMessageIdentifier = 0;
	};

} // namespace Server