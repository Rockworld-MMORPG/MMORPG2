#include "Network/NetworkManager.hpp"
#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageHeader.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "Common/Network/ServerProperties.hpp"
#include "EntityManager.hpp"
#include "Network/Client.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include <functional>
#include <spdlog/spdlog.h>

namespace Server
{

	NetworkManager g_networkManager;

	auto NetworkManager::init() -> bool
	{
		auto status = sf::Socket::Status::NotReady;
		status      = m_udpSocket.bind(Common::Network::UDP_PORT);
		switch (status)
		{
			case sf::Socket::Status::Done:
				spdlog::debug("Bound UDP socket to port {}", Common::Network::UDP_PORT);
				break;
			default:
				spdlog::warn("Failed to bind UDP socket to port {}", Common::Network::UDP_PORT);
				return false;
		}

		status = m_tcpListener.listen(Common::Network::TCP_PORT);
		switch (status)
		{
			case sf::Socket::Status::Done:
				spdlog::debug("Bound TCP listener to port {}", Common::Network::TCP_PORT);
				break;
			default:
				spdlog::warn("Failed to bind TCP listener to port {}", Common::Network::TCP_PORT);
				m_udpSocket.unbind();
				return false;
		}

		m_socketSelector.add(m_udpSocket);
		m_socketSelector.add(m_tcpListener);
		return true;
	}

	auto NetworkManager::shutdown() -> void
	{
		for (const auto& [identifier, client] : m_clientList)
		{
			markForDisconnect(identifier);
		}
		disconnectClients();

		m_socketSelector.clear();
		m_udpSocket.unbind();
		m_tcpListener.close();
	}

	auto NetworkManager::getNextMessage() -> std::optional<Common::Network::Message>
	{
		return m_messageQueue.getInbound();
	}

	auto NetworkManager::pushMessage(const Common::Network::Protocol protocol, const Common::Network::MessageType type, const Common::Network::ClientID clientID, Common::Network::MessageData& data) -> void
	{
		auto messageIdentifier = getNextMessageIdentifier();

		if (clientID.get() == -1)
		{
			// clientID is -1 so broadcast to all connected clients
			for (const auto& [identifier, client] : m_clientList)
			{
				auto message = Common::Network::Message();
				message.data = data;

				message.header.protocol   = protocol;
				message.header.clientID   = identifier;
				message.header.identifier = messageIdentifier;
				message.header.type       = type;

				m_messageQueue.pushOutbound(std::move(message));
			}
		}
		else
		{
			auto message = Common::Network::Message();
			message.data = data;

			message.header.protocol   = protocol;
			message.header.clientID   = clientID;
			message.header.identifier = messageIdentifier;
			message.header.type       = type;

			// clientID is specified so send to the specific client
			m_messageQueue.pushOutbound(std::move(message));
		}
	}

	auto NetworkManager::update() -> void
	{
		// Clear out the outbound queue
		auto outboundQueue = m_messageQueue.clearOutbound();
		for (auto& message : outboundQueue)
		{
			switch (message.header.protocol)
			{
				case Common::Network::Protocol::TCP:
					sendTCP(message);
					break;
				case Common::Network::Protocol::UDP:
					sendUDP(message);
					break;
			}
		}

		// Disconnect any clients awaiting disconnection
		disconnectClients();

		// Wait up to MAX_SELECTOR_WAIT_TIME for a socket to be ready to receive something
		const auto MAX_SELECTOR_WAIT_TIME = sf::milliseconds(50);
		if (m_socketSelector.wait(MAX_SELECTOR_WAIT_TIME))
		{
			if (m_socketSelector.isReady(m_tcpListener))
			{
				// Handle a new TCP connection
				acceptNewConnection();
			}
			else if (m_socketSelector.isReady(m_udpSocket))
			{
				// Handle UDP data
				receiveUDP();
			}
			else
			{
				// Handle TCP data
				for (auto& [id, client] : m_clientList)
				{
					if (m_socketSelector.isReady(*client.tcpSocket))
					{
						receiveTCP(Common::Network::ClientID(id), client);
					}
				}
			}
		}
	}

	auto generateIdentifier(sf::IpAddress address, std::uint16_t port) -> std::uint64_t
	{
		std::uint64_t identifier = 0x00;
		identifier |= static_cast<std::uint64_t>(address.toInteger()) << UINT32_WIDTH;
		identifier |= static_cast<std::uint64_t>(port);
		return identifier;
	}

	auto NetworkManager::resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<Common::Network::ClientID>
	{
		std::uint64_t identifier = generateIdentifier(ipAddress, port);
		auto iterator            = m_clientIPMap.find(identifier);
		if (iterator == m_clientIPMap.end())
		{
			return {};
		}

		return {iterator->second};
	}

	auto NetworkManager::setClientUdpPort(Common::Network::ClientID clientID, std::uint16_t udpPort) -> void
	{
		auto iterator = m_clientList.find(clientID);
		if (iterator == m_clientList.end())
		{
			spdlog::warn("Tried to find client {} but they do not exist", clientID.get());
			return;
		}
		iterator->second.udpPort = udpPort;
		auto identifier          = generateIdentifier(iterator->second.tcpSocket->getRemoteAddress().value(), udpPort);
		m_clientIPMap.emplace(identifier, clientID);
		spdlog::debug("Set client {} UDP port to {}", clientID.get(), udpPort);
		spdlog::debug("Mapping identifier {} ({}:{}) to {}", identifier, iterator->second.tcpSocket->getRemoteAddress()->toString(), udpPort, clientID.get());

		// Send the client back their client ID
		auto data
		    = Common::Network::MessageData();
		data << clientID;

		pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Connect, clientID, data);
	}

	auto NetworkManager::markForDisconnect(Common::Network::ClientID clientID) -> void
	{
		m_clientsPendingDisconnection.emplace_back(clientID);
	}

	auto NetworkManager::acceptNewConnection() -> void
	{
		auto tcpSocket = std::make_unique<sf::TcpSocket>();

		// Initialise the client sockets
		auto status = m_tcpListener.accept(*tcpSocket);
		switch (status)
		{
			case sf::Socket::Status::Done:
			{
				// Add the client to the entity manager
				auto clientID = g_entityManager.create();
				if (clientID == Common::Network::ClientID(-1))
				{
					// We can't use -1 so use something else
					clientID = g_entityManager.create();
				}
				auto [pair, success] = m_clientList.emplace(clientID, Client());

				// Add the client's TCP socket to the selector
				auto&& client    = pair->second;
				client.tcpSocket = std::move(tcpSocket);
				m_socketSelector.add(*client.tcpSocket);

				// Success
				auto clientAddress = client.tcpSocket->getRemoteAddress().value();
				spdlog::debug("Accepted a new connection from {} as client {}", clientAddress.toString(), clientID);

				auto data = Common::Network::MessageData();
				data << clientID;
				pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Connect, clientID, data);
			}
			break;
			default:
				spdlog::warn("TCP listener failed to accept a connection");
				return;
		}
	}

	auto NetworkManager::closeConnection(Common::Network::ClientID clientID) -> void
	{
		spdlog::debug("Closing connection {}", clientID.get());
		auto iterator = m_clientList.find(clientID);
		if (iterator == m_clientList.end())
		{
			spdlog::warn("Attempted to close connection {} but it does not exist", clientID.get());
			return;
		}

		// Disconnect the client
		auto&& client = iterator->second;
		m_socketSelector.remove(*client.tcpSocket);
		client.tcpSocket->disconnect();

		m_clientIPMap.erase(generateIdentifier(*client.tcpSocket->getRemoteAddress(), client.udpPort));
		m_clientList.erase(iterator);

		spdlog::debug("Connection closed successfully");

		// Remove the client from the entity manager
		g_entityManager.destroy(clientID);
	}

	auto NetworkManager::disconnectClients() -> void
	{
		for (const auto clientID : m_clientsPendingDisconnection)
		{
			closeConnection(clientID);
		}
		m_clientsPendingDisconnection.clear();
	}

	auto NetworkManager::validateIncomingMessage(const Common::Network::ClientID clientID, Common::Network::MessageHeader& header) -> bool
	{
		if (header.clientID != clientID)
		{
			// Identifiers don't match
			return false;
		}

		auto clientIterator = m_clientList.find(clientID);
		if (clientIterator == m_clientList.end())
		{
			// Client doesn't exist in the server's client map
			return false;
		}

		auto& client = clientIterator->second;
		if (client.lastMessageIdentifier > header.identifier)
		{
			// Message is out-of-date
			return false;
		}

		// Update the last message identifier because we have a newer message
		client.lastMessageIdentifier = header.identifier;

		return true;
	}

	auto NetworkManager::getNextMessageIdentifier() -> std::uint64_t
	{
		return m_currentMessageIdentifier++;
	}

	auto NetworkManager::sendUDP(Common::Network::Message& message) -> void
	{
		sf::IpAddress remoteAddress = m_clientList.at(message.header.clientID).tcpSocket->getRemoteAddress().value();
		std::uint16_t remotePort    = m_clientList.at(message.header.clientID).udpPort;

		auto buffer = message.pack();
		auto status = m_udpSocket.send(buffer.data(), buffer.size(), remoteAddress, remotePort);

		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			default:
				spdlog::warn("Failed to send UDP packet to {}:{}", remoteAddress.toString(), remotePort);
		}
	}

	auto NetworkManager::receiveUDP() -> void
	{
		sf::Packet packet;
		std::optional<sf::IpAddress> remoteAddress;
		std::uint16_t remotePort = 0;

		auto buffer        = std::array<std::uint8_t, Common::Network::MAX_MESSAGE_LENGTH>();
		std::size_t length = 0;

		auto status = m_udpSocket.receive(buffer.data(), Common::Network::MAX_MESSAGE_LENGTH, length, remoteAddress, remotePort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Dropped UDP packet");
		}

		auto message = Common::Network::Message();
		message.unpack(buffer, length);

		auto optClientID = resolveClientID(remoteAddress.value(), remotePort);
		if (!optClientID.has_value())
		{
			spdlog::warn("Received a packet from a client without an ID (from {})", message.header.clientID);
			return;
		}

		if (validateIncomingMessage(*optClientID, message.header))
		{
			m_messageQueue.pushInbound(std::move(message));
		}
	}

	auto NetworkManager::sendTCP(Common::Network::Message& message) -> void
	{
		auto clientID       = message.header.clientID;
		auto clientIterator = m_clientList.find(clientID);
		if (clientIterator == m_clientList.end())
		{
			return;
		}

		auto& socket = clientIterator->second.tcpSocket;
		auto buffer  = message.pack();
		auto status  = socket->send(buffer.data(), buffer.size());
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(clientID);
				break;
			default:
				spdlog::warn("Failed to send TCP packet");
		}
	}

	auto NetworkManager::receiveTCP(Common::Network::ClientID clientID, Client& client) -> void
	{
		static auto buffer = std::array<std::uint8_t, Common::Network::MAX_MESSAGE_LENGTH>();
		std::size_t length = 0;

		auto status = client.tcpSocket->receive(buffer.data(), buffer.size(), length);
		switch (status)
		{
			case sf::Socket::Status::Done:
			{
				auto message = Common::Network::Message();
				message.unpack(buffer, length);

				// Special case because setting ports is hard
				if (message.header.type == Common::Network::MessageType::Connect)
				{
					message.header.clientID = clientID;
				}

				m_messageQueue.pushInbound(std::move(message));
			}
			break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(clientID);
				break;
			default:
				spdlog::warn("Dropped TCP packet");
		}
	}

} // namespace Server