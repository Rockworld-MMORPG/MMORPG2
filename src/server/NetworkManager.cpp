#include "NetworkManager.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/System/Time.hpp"
#include "common/server_properties.hpp"
#include <iostream>
#include <utility>

const sf::Time MAX_SELECTOR_WAIT_TIME = sf::milliseconds(10);

namespace Server
{

	NetworkManager::NetworkManager() :
	    m_nextClientID(0)
	{
		auto status = sf::Socket::Status::NotReady;
		status      = m_udpSocket.bind(Common::UDP_PORT);
		status      = m_tcpListener.listen(Common::TCP_PORT);

		m_socketSelector.add(m_udpSocket);
		m_socketSelector.add(m_tcpListener);
	}

	auto NetworkManager::getNextTCPMessage() -> std::optional<Message>
	{
		if (!m_inboundTCPQueue.empty())
		{
			auto message = m_inboundTCPQueue.front();
			m_inboundTCPQueue.pop();
			return {std::move(message)};
		}
		return {};
	}

	auto NetworkManager::getNextUDPMessage() -> std::optional<Message>
	{
		if (!m_inboundUDPQueue.empty())
		{
			auto message = m_inboundUDPQueue.front();
			m_inboundUDPQueue.pop();
			return {std::move(message)};
		}
		return {};
	}

	auto NetworkManager::pushTCPMessage(Message&& message) -> void
	{
		m_outboundTCPQueue.push(std::move(message));
	}

	auto NetworkManager::pushUDPMessage(Message&& message) -> void
	{
		m_outboundUDPQueue.push(std::move(message));
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
						receiveTCP(id, client);
					}
				}
			}
		}

		// Clear out the TCP and UDP outbound queues
		sendTCP();
		sendUDP();
	}

	auto NetworkManager::setClientUdpPort(ClientID clientID, std::uint16_t udpPort) -> void
	{
		auto iterator = m_clientList.find(clientID);
		if (iterator == m_clientList.end())
		{
			return;
		}
		iterator->second.udpPort = udpPort;
		std::cout << "Set client " << clientID << " UDP port to " << udpPort << "\n";
	}

	auto NetworkManager::acceptNewConnection() -> void
	{
		auto clientID        = generateClientID();
		auto [pair, success] = m_clientList.emplace(clientID, Client{});

		auto&& client    = pair->second;
		client.tcpSocket = std::make_unique<sf::TcpSocket>();
		auto status      = m_tcpListener.accept(*client.tcpSocket);
		client.tcpSocket->setBlocking(false);
		m_socketSelector.add(*client.tcpSocket);

		std::cout << "Accepted a new connection from " << client.tcpSocket->getRemoteAddress()->toString() << " as client " << clientID << "\n";
	}

	auto NetworkManager::closeConnection(ClientID clientID) -> void
	{
		std::cout << "Closing connection " << clientID << "\n";
		auto iterator = m_clientList.find(clientID);
		if (iterator == m_clientList.end())
		{
			return;
		}

		std::cout << "Disconnecting socket\n";
		m_socketSelector.remove(*iterator->second.tcpSocket);
		iterator->second.tcpSocket->disconnect();
		m_clientList.erase(iterator);
		std::cout << "Disconnect successful\n";
	}

	auto NetworkManager::resolveClientID(sf::IpAddress& remoteAddress) -> std::optional<ClientID>
	{
		for (const auto& [k, v] : m_clientList)
		{
			auto tcpAddress = v.tcpSocket->getRemoteAddress();
			if (tcpAddress.value() == remoteAddress)
			{
				return k;
			}
		}
		return {};
	}

	auto NetworkManager::generateClientID() -> ClientID
	{
		return m_nextClientID++;
	}

	auto NetworkManager::receiveUDP() -> void
	{
		auto status = sf::Socket::Status::NotReady;

		sf::Packet packet{};
		std::optional<sf::IpAddress> remoteAddress{};
		std::uint16_t remotePort = 0;

		do
		{
			status = m_udpSocket.receive(packet, remoteAddress, remotePort);
		} while (status == sf::Socket::Status::Partial);

		if (status == sf::Socket::Status::Done)
		{
			auto localClientID = resolveClientID(remoteAddress.value());
			if (localClientID.has_value())
			{
				Message message{};
				message.clientID = localClientID.value();
				message.data     = std::move(packet);
				m_inboundUDPQueue.emplace(std::move(message));
			}
		}
	}

	auto NetworkManager::receiveTCP(ClientID clientID, Client& client) -> void
	{
		auto status = sf::Socket::Status::NotReady;
		sf::Packet packet{};

		do
		{
			status = client.tcpSocket->receive(packet);
		} while (status == sf::Socket::Status::Partial);

		if (status == sf::Socket::Status::Done)
		{
			Message message{};
			message.clientID = clientID;
			message.data     = std::move(packet);
			m_inboundTCPQueue.emplace(std::move(message));
		}
	}

	auto NetworkManager::sendUDP() -> void
	{
		while (!m_outboundUDPQueue.empty())
		{
			Message message{};
			std::swap(message, m_outboundUDPQueue.front());
			m_outboundUDPQueue.pop();
			auto status = sf::Socket::Status::NotReady;

			// Check if we have data for this client
			if (!m_clientList.contains(message.clientID))
			{
				std::cout << "No client data\n";
				// If we don't then just delete the message
				break;
			}

			// Get the data for the client
			auto&& client      = m_clientList.at(message.clientID);
			auto remoteAddress = client.tcpSocket->getRemoteAddress();
			auto remotePort    = client.udpPort;

			// Send the packet to the client
			do
			{
				status = m_udpSocket.send(message.data, remoteAddress.value(), remotePort);
			} while (status == sf::Socket::Status::Partial);
		}
	}

	auto NetworkManager::sendTCP() -> void
	{
		while (!m_outboundTCPQueue.empty())
		{
			auto&& message = m_outboundTCPQueue.front();
			auto status    = sf::Socket::Status::NotReady;

			if (m_clientList.contains(message.clientID))
			{
				m_outboundTCPQueue.pop();
				break;
			}

			auto&& client = m_clientList.at(message.clientID);
			do
			{
				status = client.tcpSocket->send(message.data);
			} while (status == sf::Socket::Status::Partial);

			m_outboundTCPQueue.pop();
		}
	}

} // namespace Server