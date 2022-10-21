#include "Network/NetworkManager.hpp"
#include "Common/Network/MessageData.hpp"
#include "SFML/System/Time.hpp"
#include "Server/Server.hpp"
#include <Common/Network/Message.hpp>
#include <Common/Network/ServerProperties.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <functional>
#include <spdlog/spdlog.h>

namespace Server
{

	NetworkManager::NetworkManager(Server& server) :
	    Manager(server)
	{
	}

	NetworkManager::~NetworkManager() = default;

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
		for (const auto entity : server.registry.view<Client>())
		{
			markForDisconnect(entity);
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

	auto NetworkManager::getMessages() -> std::vector<Common::Network::Message>
	{
		return m_messageQueue.clearInbound();
	}

	auto NetworkManager::pushMessage(const Common::Network::Protocol protocol, const Common::Network::MessageType type, const entt::entity entityID, Common::Network::MessageData& data) -> void
	{
		auto message = Common::Network::Message();
		message.data = data;

		message.header.protocol   = protocol;
		message.header.entityID   = entityID;
		message.header.identifier = getNextMessageIdentifier();
		message.header.type       = type;

		m_messageQueue.pushOutbound(std::move(message));
	}

	auto NetworkManager::pushMessage(const Common::Network::Protocol protocol, const Common::Network::MessageType type, Common::Network::MessageData& data) -> void
	{
		for (const auto entity : server.registry.view<Client>())
		{
			pushMessage(protocol, type, entity, data);
		}
	}

	auto NetworkManager::update(const sf::Time maxSelectorWaitTime = sf::milliseconds(50)) -> void
	{
		// Disconnect any clients awaiting disconnection
		disconnectClients();

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

		// Wait up to maxSelectorWaitTime for a socket to be ready to receive something
		if (m_socketSelector.wait(maxSelectorWaitTime))
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
				for (const auto entity : server.registry.view<Client>())
				{
					auto& client = server.registry.get<Client>(entity);
					if (m_socketSelector.isReady(*client.tcpSocket))
					{
						receiveTCP(entity, client);
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

	auto NetworkManager::resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<entt::entity>
	{
		std::uint64_t identifier = generateIdentifier(ipAddress, port);
		auto iterator            = m_clientIPMap.find(identifier);
		if (iterator == m_clientIPMap.end())
		{
			return {};
		}

		return {iterator->second};
	}

	auto NetworkManager::setClientUdpPort(entt::entity entityID, std::uint16_t udpPort) -> void
	{
		if (!server.registry.all_of<Client>(entityID))
		{
			spdlog::warn("Tried to find client {} but they do not exist", static_cast<std::uint32_t>(entityID));
		}

		auto& client    = server.registry.get<Client>(entityID);
		client.udpPort  = udpPort;
		auto identifier = generateIdentifier(client.tcpSocket->getRemoteAddress().value(), udpPort);
		m_clientIPMap.emplace(identifier, entityID);
		spdlog::debug("Set client {} UDP port to {}", static_cast<std::uint32_t>(entityID), udpPort);
		spdlog::debug("Mapping identifier {} ({}:{}) to {}", identifier, client.tcpSocket->getRemoteAddress()->toString(), udpPort, static_cast<std::uint32_t>(entityID));

		// Send the client back their client ID
		auto data = Common::Network::MessageData();
		data << entityID;
		pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Connect, entityID, data);
	}

	auto NetworkManager::markForDisconnect(entt::entity entityID) -> void
	{
		m_clientsPendingDisconnection.emplace_back(entityID);
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
				auto entityID = server.registry.create();
				auto& client  = server.registry.emplace<Client>(entityID);

				client.tcpSocket = std::move(tcpSocket);
				m_socketSelector.add(*client.tcpSocket);

				// Success
				auto clientAddress = client.tcpSocket->getRemoteAddress().value();
				spdlog::debug("Accepted a new connection from {} as client {}", clientAddress.toString(), static_cast<std::uint32_t>(entityID));

				auto data = Common::Network::MessageData();
				data << entityID;
				pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Connect, entityID, data);
			}
			break;
			default:
				spdlog::warn("TCP listener failed to accept a connection");
				return;
		}
	}

	auto NetworkManager::closeConnection(entt::entity entityID) -> void
	{
		spdlog::debug("Closing connection {}", static_cast<std::uint32_t>(entityID));
		if (!server.registry.all_of<Client>(entityID))
		{
			spdlog::warn("Attempted to close connection {} but it does not exist", static_cast<std::uint32_t>(entityID));
			return;
		}

		// Disconnect the client
		auto& client = server.registry.get<Client>(entityID);
		m_socketSelector.remove(*client.tcpSocket);
		client.tcpSocket->disconnect();

		auto ipMapIterator = std::find_if(m_clientIPMap.begin(), m_clientIPMap.end(), [&](const std::pair<std::uint64_t, entt::entity> element) {
			return element.second == entityID;
		});
		if (ipMapIterator != m_clientIPMap.end())
		{
			m_clientIPMap.erase(ipMapIterator);
		}

		server.registry.destroy(entityID);
		spdlog::debug("Connection closed successfully");
	}

	auto NetworkManager::disconnectClients() -> void
	{
		for (const auto entityID : m_clientsPendingDisconnection)
		{
			closeConnection(entityID);
		}
		m_clientsPendingDisconnection.clear();
	}

	auto NetworkManager::validateIncomingMessage(const entt::entity entityID, Common::Network::MessageHeader& header) -> bool
	{
		if (header.entityID != entityID)
		{
			// Identifiers don't match
			return false;
		}

		if (!server.registry.all_of<Client>(entityID))
		{
			// Client doesn't exist in the server's client map
			return false;
		}

		auto& client = server.registry.get<Client>(entityID);
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
		return ++m_currentMessageIdentifier;
	}

	auto NetworkManager::sendUDP(Common::Network::Message& message) -> void
	{
		if (!server.registry.valid(message.header.entityID))
		{
			return;
		}

		if (!server.registry.all_of<Client>(message.header.entityID))
		{
			spdlog::warn("Tried to send a message to a client ({}) but they don't exist", static_cast<std::uint32_t>(message.header.entityID));
			return;
		}

		auto& client                = server.registry.get<Client>(message.header.entityID);
		sf::IpAddress remoteAddress = client.tcpSocket->getRemoteAddress().value();
		std::uint16_t remotePort    = client.udpPort;

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
			spdlog::warn("Received a packet from a client without an ID (from {})", static_cast<std::uint32_t>(message.header.entityID));
			return;
		}

		if (validateIncomingMessage(*optClientID, message.header))
		{
			m_messageQueue.pushInbound(std::move(message));
		}
	}

	auto NetworkManager::sendTCP(Common::Network::Message& message) -> void
	{
		if (!server.registry.valid(message.header.entityID))
		{
			return;
		}

		if (!server.registry.all_of<Client>(message.header.entityID))
		{
			return;
		}

		auto& client = server.registry.get<Client>(message.header.entityID);
		auto& socket = client.tcpSocket;
		auto buffer  = message.pack();
		auto status  = socket->send(buffer.data(), buffer.size());
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(message.header.entityID);
				break;
			default:
				spdlog::warn("Failed to send TCP packet");
		}
	}

	auto NetworkManager::receiveTCP(entt::entity entityID, Client& client) -> void
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
					message.header.entityID = entityID;
				}

				m_messageQueue.pushInbound(std::move(message));
			}
			break;
			case sf::Socket::Status::Disconnected:
				markForDisconnect(entityID);
				break;
			default:
				spdlog::warn("Dropped TCP packet");
		}
	}

} // namespace Server