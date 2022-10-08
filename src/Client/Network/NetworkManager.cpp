#include "NetworkManager.hpp"
#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageQueue.hpp"
#include "Common/Network/Protocol.hpp"
#include "SFML/System/Time.hpp"
#include "spdlog/fmt/bundled/format.h"
#include <Common/Network/MessageType.hpp>
#include <Common/Network/ServerProperties.hpp>
#include <spdlog/spdlog.h>

namespace Client
{
	NetworkManager::NetworkManager() :
	    m_currentMessageIdentifier(0),
	    m_lastServerMessageIdentifier(0),
	    m_clientID(-1)
	{
		auto status = m_udpSocket.bind(sf::Socket::AnyPort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Failed to bind the UDP socket to any port");
			return;
		}

		spdlog::debug("UDP bound to port {}", m_udpSocket.getLocalPort());
	}

	NetworkManager::~NetworkManager()
	{
		disconnect();
	}

	auto NetworkManager::connect() -> void
	{
		spdlog::debug("Connecting to server");
		auto status = sf::Socket::Status::NotReady;

		status = m_tcpSocket.connect(Common::Network::SERVER_ADDRESS, Common::Network::TCP_PORT);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Failed to connect to the server (error code {})", static_cast<std::uint32_t>(status));
			return;
		}
		spdlog::debug("Connected to server successfully");

		m_messageQueue.clearInbound();
		m_messageQueue.clearOutbound();

		auto message              = Common::Network::Message();
		message.header.clientID   = getClientID();
		message.header.identifier = getNextMessageIdentifier();
		message.header.protocol   = Common::Network::Protocol::TCP;
		message.header.type       = Common::Network::MessageType::Connect;
		message.data << m_udpSocket.getLocalPort();

		const std::size_t MAX_CONNECTION_ATTEMPTS = 5;
		for (auto attemptNumber = 0; attemptNumber < MAX_CONNECTION_ATTEMPTS; ++attemptNumber)
		{
			spdlog::debug("Awaiting successful connection ({}/{})", attemptNumber, MAX_CONNECTION_ATTEMPTS);
			sendTCP(message);
			receiveTCP();
			auto messages = m_messageQueue.clearInbound();
			for (auto& message : messages)
			{
				if (message.header.type == Common::Network::MessageType::Connect)
				{
					Common::Network::ClientID_t clientID = -1;
					message.data >> clientID;
					m_clientID = Common::Network::ClientID(clientID);
					spdlog::debug("Port requested successfully and granted client ID {}", m_clientID.get());

					m_socketSelector.add(m_tcpSocket);
					m_socketSelector.add(m_udpSocket);
					return;
				}
			}
		}

		spdlog::warn("Failed to connect");
	}

	auto NetworkManager::disconnect() -> void
	{
		spdlog::debug("Disconnecting from server");

		m_messageQueue.clearInbound();
		m_messageQueue.clearOutbound();

		auto message              = Common::Network::Message();
		message.header.clientID   = getClientID();
		message.header.identifier = getNextMessageIdentifier();
		message.header.protocol   = Common::Network::Protocol::TCP;
		message.header.type       = Common::Network::MessageType::Disconnect;

		auto buffer = message.pack();
		auto status = m_tcpSocket.send(buffer.data(), buffer.size());

		m_tcpSocket.disconnect();
		m_socketSelector.clear();
	}

	auto NetworkManager::update() -> void
	{
		const auto SELECTOR_TIMEOUT = sf::milliseconds(5);
		if (m_socketSelector.wait(SELECTOR_TIMEOUT))
		{
			if (m_socketSelector.isReady(m_tcpSocket))
			{
				receiveTCP();
			}
			else
			{
				receiveUDP();
			}
		}

		auto outboundMessages = m_messageQueue.clearOutbound();
		for (const auto& message : outboundMessages)
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
	}

	auto NetworkManager::pushMessage(const Common::Network::Protocol protocol, const Common::Network::MessageType type, Common::Network::MessageData& messageData) -> void
	{
		auto message              = Common::Network::Message();
		message.header.clientID   = getClientID();
		message.header.identifier = getNextMessageIdentifier();
		message.header.protocol   = protocol;
		message.header.type       = type;

		message.data = messageData;
		m_messageQueue.pushOutbound(std::move(message));
	}

	auto NetworkManager::getNextMessage() -> std::optional<Common::Network::Message>
	{
		return m_messageQueue.getInbound();
	}

	auto NetworkManager::getMessages() -> std::vector<Common::Network::Message>
	{
		return m_messageQueue.clearInbound();
	}

	auto NetworkManager::getClientID() -> Common::Network::ClientID
	{
		return m_clientID;
	}

	auto NetworkManager::getNextMessageIdentifier() -> std::uint64_t
	{
		return m_currentMessageIdentifier++;
	}

	auto NetworkManager::validateMessage(Common::Network::Message& message) -> bool
	{
		if (message.header.identifier <= m_lastServerMessageIdentifier)
		{
			return false;
		}
		m_lastServerMessageIdentifier = message.header.identifier;
		return true;
	}

	auto NetworkManager::sendUDP(const Common::Network::Message& message) -> void
	{
		auto buffer = message.pack();
		auto status = m_udpSocket.send(buffer.data(), buffer.size(), Common::Network::SERVER_ADDRESS, Common::Network::UDP_PORT);
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			default:
				spdlog::warn("Dropped packet");
				break;
		}
	}

	auto NetworkManager::receiveUDP() -> void
	{
		auto buffer        = std::array<std::uint8_t, Common::Network::MAX_MESSAGE_LENGTH>();
		std::size_t length = 0;
		std::optional<sf::IpAddress> optAddress;
		std::uint16_t remotePort = 0;

		auto status = m_udpSocket.receive(buffer.data(), buffer.size(), length, optAddress, remotePort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Dropped packet");
			return;
		}

		if (optAddress.value() != Common::Network::SERVER_ADDRESS)
		{
			return;
		}

		auto message = Common::Network::Message();
		message.unpack(buffer, length);

		if (validateMessage(message))
		{
			m_messageQueue.pushInbound(std::move(message));
		}
	}

	auto NetworkManager::sendTCP(const Common::Network::Message& message) -> void
	{
		auto buffer = message.pack();
		auto status = m_tcpSocket.send(buffer.data(), buffer.size());
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
				break;
			case sf::Socket::Status::Disconnected:
				spdlog::warn("Client was disconnected");
				disconnect();
				break;
			default:
				spdlog::warn("Dropped packet");
				break;
		}
	}

	auto NetworkManager::receiveTCP() -> void
	{
		auto buffer        = std::array<std::uint8_t, Common::Network::MAX_MESSAGE_LENGTH>();
		std::size_t length = 0;
		auto status        = m_tcpSocket.receive(buffer.data(), buffer.size(), length);

		switch (status)
		{
			case sf::Socket::Status::Done:
			{
				auto message = Common::Network::Message();
				message.unpack(buffer, length);

				if (validateMessage(message))
				{
					m_messageQueue.pushInbound(std::move(message));
				}
			}
			// Success
			break;
			case sf::Socket::Status::Disconnected:
				spdlog::warn("Client was disconnected");
				disconnect();
				break;
			default:
				spdlog::warn("Dropped packet");
				break;
		}
	}

} // namespace Client