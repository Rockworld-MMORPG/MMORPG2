#include "NetworkManager.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/ServerProperties.hpp"
#include "EntityManager.hpp"
#include "Message.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Packet.hpp"
#include <functional>
#include <spdlog/spdlog.h>
#include <unistd.h>

const sf::Time MAX_SELECTOR_WAIT_TIME = sf::milliseconds(10);

namespace Server
{

	NetworkManager g_networkManager;

	auto NetworkManager::init() -> void
	{
		auto status = sf::Socket::Status::NotReady;
		status      = m_udpSocket.bind(Common::Network::UDP_PORT);
		status      = m_tcpListener.listen(Common::Network::TCP_PORT);

		m_socketSelector.add(m_udpSocket);
		m_socketSelector.add(m_tcpListener);
	}

	auto NetworkManager::shutdown() -> void
	{
		m_socketSelector.clear();
		m_udpSocket.unbind();
		m_tcpListener.close();
	}

	auto NetworkManager::getNextTCPMessage() -> std::optional<Server::Message>
	{
		return m_tcpQueue.getInbound();
	}

	auto NetworkManager::getNextUDPMessage() -> std::optional<Server::Message>
	{
		return m_udpQueue.getInbound();
	}

	auto NetworkManager::pushTCPMessage(Server::Message&& message) -> void
	{
		m_tcpQueue.pushOutbound(std::forward<Server::Message>(message));
	}

	auto NetworkManager::pushUDPMessage(Server::Message&& message) -> void
	{
		m_udpQueue.pushOutbound(std::forward<Server::Message>(message));
	}

	auto NetworkManager::update() -> void
	{
		// Wait until a socket is ready to receive something
		while (m_socketSelector.wait(MAX_SELECTOR_WAIT_TIME))
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

		// Clear out the TCP and UDP outbound queues
		sendTCP();
		sendUDP();

		// Disconnect any clients awaiting disconnection
		disconnectClients();
	}

	auto NetworkManager::resolveClientID(sf::IpAddress ipAddress) -> std::optional<Common::Network::ClientID>
	{
		auto iterator = m_clientIPMap.find(ipAddress.toInteger());
		if (iterator == m_clientIPMap.end())
		{
			return {};
		}

		return iterator->second;
	}

	auto NetworkManager::setClientUdpPort(Common::Network::ClientID clientID, std::uint16_t udpPort) -> void
	{
		auto iterator = m_clientList.find(clientID);
		if (iterator == m_clientList.end())
		{
			return;
		}
		iterator->second.udpPort = udpPort;
		spdlog::debug("Set client {} UDP port to {}", clientID.get(), udpPort);

		// Send the client back their client ID
		Server::Message message{clientID, Message::Protocol::UDP};
		message.packet << Common::Network::MessageType::Connect << clientID;
		pushTCPMessage(std::move(message));
	}

	auto NetworkManager::acceptNewConnection() -> void
	{
		// Add the client to the entity manager
		auto clientID        = g_entityManager.create();
		auto [pair, success] = m_clientList.emplace(clientID, Client());

		// Initialise the client sockets
		auto&& client    = pair->second;
		client.tcpSocket = std::make_unique<sf::TcpSocket>();
		auto status      = m_tcpListener.accept(*client.tcpSocket);
		client.tcpSocket->setBlocking(false);
		m_socketSelector.add(*client.tcpSocket);

		auto clientAddress = client.tcpSocket->getRemoteAddress().value();
		m_clientIPMap.emplace(clientAddress.toInteger(), clientID);
		spdlog::debug("Accepted a new connection from {} as client {}", clientAddress.toString(), clientID);
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
	}

	auto NetworkManager::sendUDP() -> void
	{
		std::optional<Server::Message> message;
		while ((message = m_udpQueue.getOutbound()).has_value())
		{
			sf::Packet packet;
			sf::IpAddress remoteAddress = m_clientList.at(message->clientID).tcpSocket->getRemoteAddress().value();
			std::uint16_t remotePort    = m_clientList.at(message->clientID).udpPort;
			auto status                 = m_udpSocket.send(packet, remoteAddress, remotePort);

			switch (status)
			{
				case sf::Socket::Status::Done:
					// Success
					break;
				default:
					spdlog::warn("Dropped packet");
			}
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

		auto clientID = resolveClientID(remoteAddress.value());
		if (!clientID.has_value())
		{
			spdlog::warn("Received a packet from a client without an ID");
			return;
		}

		Server::Message message{clientID.value(), Server::Message::Protocol::UDP};
		std::swap(message.packet, packet);
		m_udpQueue.pushInbound(std::move(message));
	}

	auto NetworkManager::sendTCP() -> void
	{
		std::optional<Server::Message> message;
		while ((message = m_tcpQueue.getOutbound()).has_value())
		{
			auto clientID = message->clientID;
			auto&& socket = m_clientList.at(clientID).tcpSocket.get();
			auto status   = socket->send(message->packet);
			switch (status)
			{
				case sf::Socket::Status::Done:
					// Success
					break;
				case sf::Socket::Status::Disconnected:
					closeConnection(clientID);
					break;
				default:
					spdlog::warn("Dropped packet");
			}
		}
	}

	auto NetworkManager::receiveTCP(Common::Network::ClientID clientID, Client& client) -> void
	{
		sf::Packet packet;
		auto status = client.tcpSocket->receive(packet);
		switch (status)
		{
			case sf::Socket::Status::Done:
			{
				Server::Message message{clientID, Message::Protocol::TCP};
				std::swap(message.packet, packet);
				m_tcpQueue.pushInbound(std::move(message));
			}
			break;
			case sf::Socket::Status::Disconnected:
				m_clientsPendingDisconnection.emplace_back(clientID);
				break;
			default:
				spdlog::warn("Dropped packet");
		}
	}

} // namespace Server