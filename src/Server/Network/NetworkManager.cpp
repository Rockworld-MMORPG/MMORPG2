#include "Network/NetworkManager.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/ServerProperties.hpp"
#include "EntityManager.hpp"
#include "Network/Client.hpp"
#include "Network/Message.hpp"
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

	auto NetworkManager::getNextMessage() -> std::optional<Message>
	{
		return m_messageQueue.getInbound();
	}

	auto NetworkManager::pushMessage(Message&& message) -> void
	{
		m_messageQueue.pushOutbound(std::forward<Message>(message));
	}

	auto NetworkManager::update() -> void
	{
		// Clear out the outbound queue
		auto outboundQueue = m_messageQueue.clearOutbound();
		for (auto& message : outboundQueue)
		{
			switch (message.protocol)
			{
				case Message::Protocol::TCP:
					sendTCP(message);
					break;
				case Message::Protocol::UDP:
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
		return static_cast<std::uint64_t>(address.toInteger()) << UINT32_WIDTH | static_cast<std::uint64_t>(port);
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
			return;
		}
		iterator->second.udpPort = udpPort;
		auto identifier          = generateIdentifier(iterator->second.tcpSocket->getRemoteAddress().value(), udpPort);
		m_clientIPMap.emplace(identifier, clientID);
		spdlog::debug("Set client {} UDP port to {}", clientID.get(), udpPort);

		// Send the client back their client ID
		Message message{clientID, Message::Protocol::TCP};
		message.packet << Common::Network::MessageType::Connect << clientID;
		pushMessage(std::move(message));
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
				auto clientID        = g_entityManager.create();
				auto [pair, success] = m_clientList.emplace(clientID, Client());

				// Add the client's TCP socket to the selector
				auto&& client    = pair->second;
				client.tcpSocket = std::move(tcpSocket);
				m_socketSelector.add(*client.tcpSocket);

				// Success
				auto clientAddress = client.tcpSocket->getRemoteAddress().value();
				spdlog::debug("Accepted a new connection from {} as client {}", clientAddress.toString(), clientID);
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
			return;
		}

		// Disconnect the client
		auto&& client = iterator->second;
		m_socketSelector.remove(*client.tcpSocket);
		m_clientIPMap.erase(client.tcpSocket->getRemoteAddress()->toInteger());
		client.tcpSocket->disconnect();
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

	auto NetworkManager::sendUDP(Message& message) -> void
	{
		if (message.clientID.get() == -1)
		{
			broadcastUDP(message);
			return;
		}

		sf::IpAddress remoteAddress = m_clientList.at(message.clientID).tcpSocket->getRemoteAddress().value();
		std::uint16_t remotePort    = m_clientList.at(message.clientID).udpPort;
		auto status                 = m_udpSocket.send(message.packet, remoteAddress, remotePort);

		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			default:
				spdlog::warn("Dropped packet");
		}
	}

	auto NetworkManager::receiveUDP() -> void
	{
		sf::Packet packet;
		std::optional<sf::IpAddress> remoteAddress;
		std::uint16_t remotePort = 0;

		auto status = m_udpSocket.receive(packet, remoteAddress, remotePort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Dropped packet");
		}

		auto clientID = resolveClientID(remoteAddress.value(), remotePort);
		if (!clientID.has_value())
		{
			spdlog::warn("Received a packet from a client without an ID");
			return;
		}

		Message message{clientID.value(), Message::Protocol::UDP};
		message.packet = packet;
		m_messageQueue.pushInbound(std::move(message));
	}

	auto NetworkManager::broadcastUDP(Message& message) -> void
	{
		for (const auto& [key, client] : m_clientList)
		{
			auto status = m_udpSocket.send(message.packet, client.tcpSocket->getRemoteAddress().value(), client.udpPort);
			if (status != sf::Socket::Status::Done)
			{
				spdlog::warn("Dropped packet");
			}
		}
	}

	auto NetworkManager::sendTCP(Message& message) -> void
	{
		if (message.clientID.get() == -1)
		{
			broadcastTCP(message);
			return;
		}

		auto clientID = message.clientID;
		auto& socket  = m_clientList.at(clientID).tcpSocket;
		auto status   = socket->send(message.packet);
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(clientID);
				break;
			default:
				spdlog::warn("Dropped packet");
		}
	}

	auto NetworkManager::receiveTCP(Common::Network::ClientID clientID, Client& client) -> void
	{
		Message message{clientID, Message::Protocol::TCP};
		auto status = client.tcpSocket->receive(message.packet);
		switch (status)
		{
			case sf::Socket::Status::Done:
				m_messageQueue.pushInbound(std::move(message));
				break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(clientID);
				break;
			default:
				spdlog::warn("Dropped packet");
		}
	}

	auto NetworkManager::broadcastTCP(Message& message) -> void
	{
		for (const auto& [clientID, client] : m_clientList)
		{
			auto status = client.tcpSocket->send(message.packet);
			switch (status)
			{
				case sf::Socket::Status::Done:
					// Success
					break;
				case sf::Socket::Status::Disconnected:
					markForDisconnect(clientID);
					break;
				default:
					spdlog::warn("Dropped packet");
			}
		}
	}

} // namespace Server