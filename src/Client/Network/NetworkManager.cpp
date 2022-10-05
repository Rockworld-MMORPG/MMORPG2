#include "NetworkManager.hpp"
#include "Common/Network/MessageQueue.hpp"
#include "SFML/System/Time.hpp"
#include "spdlog/fmt/bundled/format.h"
#include <Common/Network/MessageType.hpp>
#include <Common/Network/ServerProperties.hpp>
#include <spdlog/spdlog.h>

namespace Client
{
	NetworkManager g_networkManager;

	auto NetworkManager::init() -> void
	{
		m_connected = false;

		auto status = m_udpSocket.bind(sf::Socket::AnyPort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Failed to bind the UDP socket to any port");
			return;
		}

		spdlog::debug("UDP bound to port {}", m_udpSocket.getLocalPort());
	}

	auto NetworkManager::shutdown() -> void
	{
		disconnect();
		m_udpSocket.unbind();
		m_socketSelector.clear();
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

		{
			sf::Packet packet;
			packet << Common::Network::MessageType::Connect << m_udpSocket.getLocalPort();
			status = m_tcpSocket.send(packet);

			if (status != sf::Socket::Status::Done)
			{
				spdlog::warn("Failed to send connection message (error code {})", static_cast<std::uint32_t>(status));
				return;
			}
			spdlog::debug("Sent connection message requesting UDP port {}", m_udpSocket.getLocalPort());
		}

		const std::size_t MAX_CONNECTION_ATTEMPTS = 5;
		for (auto attemptNumber = 0; attemptNumber < MAX_CONNECTION_ATTEMPTS; ++attemptNumber)
		{
			spdlog::debug("Awaiting successful connection ({}/{})", attemptNumber, MAX_CONNECTION_ATTEMPTS);
			sf::Packet packet;
			status = m_tcpSocket.receive(packet);
			switch (status)
			{
				case sf::Socket::Status::Done:
				{
					Common::Network::MessageType messageType = Common::Network::MessageType::None;
					packet >> messageType;
					if (messageType == Common::Network::MessageType::Connect)
					{
						Common::Network::ClientID_t clientID = -1;
						packet >> clientID;
						spdlog::debug("Port requested successfully and granted client ID {}", clientID);
						m_clientID  = Common::Network::ClientID(clientID);
						m_connected = true;

						m_socketSelector.add(m_tcpSocket);
						m_socketSelector.add(m_udpSocket);
						return;
					}
				}
				break;
				case sf::Socket::Status::Disconnected:
					disconnect();
					attemptNumber = MAX_CONNECTION_ATTEMPTS;
					break;
				default:
					// Try again
					break;
			}
		}
		spdlog::warn("Failed to connect");
	}

	auto NetworkManager::disconnect() -> void
	{
		if (!m_connected)
		{
			return;
		}

		spdlog::debug("Disconnecting from server");

		sf::Packet packet{};
		packet << Common::Network::MessageType::Disconnect;
		auto status = m_tcpSocket.send(packet);

		switch (status)
		{
			case sf::Socket::Status::Done:
			{
				auto attempts                             = 0;
				const std::size_t MAX_CONNECTION_ATTEMPTS = 5;
				while ((m_tcpSocket.receive(packet) != sf::Socket::Status::Disconnected) && (attempts < MAX_CONNECTION_ATTEMPTS))
				{
					++attempts;
				}
			}
			case sf::Socket::Status::Disconnected:
				spdlog::debug("Sent disconnect message successfully");
				break;
			default:
				spdlog::warn("Failed to send disconnect message (error code {})", static_cast<std::uint32_t>(status));
		}

		m_socketSelector.clear();
		m_connected = false;
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

		sendTCP();
		sendUDP();
	}

	auto NetworkManager::pushTCPMessage(sf::Packet&& packet) -> void
	{
		m_tcpQueue.pushOutbound(std::forward<sf::Packet&&>(packet));
	}

	auto NetworkManager::pushUDPMessage(sf::Packet&& packet) -> void
	{
		m_udpQueue.pushOutbound(std::forward<sf::Packet&&>(packet));
	}

	auto NetworkManager::getNextTCPMessage() -> std::optional<sf::Packet>
	{
		return m_tcpQueue.getInbound();
	}

	auto NetworkManager::getNextUDPMessage() -> std::optional<sf::Packet>
	{
		return m_udpQueue.getInbound();
	}

	auto NetworkManager::getClientID() -> Common::Network::ClientID
	{
		return m_clientID;
	}

	auto NetworkManager::sendUDP() -> void
	{
		std::optional<sf::Packet> optPacket;
		while ((optPacket = m_udpQueue.getOutbound()).has_value())
		{
			if (!m_connected)
			{
				spdlog::debug("Tried to send UDP but the client is not connected");
				return;
			}

			auto status = m_udpSocket.send(optPacket.value(), Common::Network::SERVER_ADDRESS, Common::Network::UDP_PORT);
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
	}

	auto NetworkManager::receiveUDP() -> void
	{
		if (!m_connected)
		{
			spdlog::debug("Tried to receive UDP but the client is not connected");
			return;
		}

		sf::Packet packet;
		std::optional<sf::IpAddress> optAddress;
		std::uint16_t remotePort = 0;

		auto status = m_udpSocket.receive(packet, optAddress, remotePort);
		if (status != sf::Socket::Status::Done)
		{
			spdlog::warn("Dropped packet");
			return;
		}

		if (optAddress.value() != Common::Network::SERVER_ADDRESS)
		{
			return;
		}

		m_udpQueue.pushInbound(std::move(packet));
	}

	auto NetworkManager::sendTCP() -> void
	{
		std::optional<sf::Packet> optPacket;
		while ((optPacket = m_tcpQueue.getOutbound()).has_value())
		{
			if (!m_connected)
			{
				spdlog::debug("Tried to send TCP but the client is not connected");
				return;
			}

			auto status = m_tcpSocket.send(optPacket.value());
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
	}

	auto NetworkManager::receiveTCP() -> void
	{
		if (!m_connected)
		{
			spdlog::debug("Tried to receive TCP but the client is not connected");
			return;
		}

		sf::Packet packet;
		auto status = m_tcpSocket.receive(packet);
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

		m_tcpQueue.pushInbound(std::move(packet));
	}

} // namespace Client