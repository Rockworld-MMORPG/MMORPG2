#include "NetworkManager.hpp"
#include <array>
#include <thread>

namespace Client
{
	NetworkManager::NetworkManager() :
	    m_currentMessageIdentifier(0),
	    m_lastServerMessageIdentifier(0),
	    m_clientID(entt::null),
	    m_isConnected(false)
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
		if (m_isConnected)
		{
			return;
		}

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
		message.header.entityID   = getClientID();
		message.header.identifier = getNextMessageIdentifier();
		message.header.protocol   = Common::Network::Protocol::TCP;
		message.header.type       = Common::Network::MessageType::Client_Connect;
		message.data << std::uint16_t(m_udpSocket.getLocalPort());

		const std::size_t MAX_CONNECTION_ATTEMPTS = 5;
		for (auto attemptNumber = 1; attemptNumber <= MAX_CONNECTION_ATTEMPTS; ++attemptNumber)
		{
			sendTCP(message);
			spdlog::debug("Awaiting successful connection ({}/{})", attemptNumber, MAX_CONNECTION_ATTEMPTS);
			receiveTCP();
			auto messages = m_messageQueue.clearInbound();
			for (auto& message : messages)
			{
				if (message.header.type == Common::Network::MessageType::Server_SetClientID)
				{
					spdlog::debug("Port requested successfully");
					auto& data = message.data;
					data >> m_clientID;
					spdlog::debug("Set client ID to {}", static_cast<std::uint32_t>(m_clientID));

					m_socketSelector.add(m_tcpSocket);
					m_socketSelector.add(m_udpSocket);
					m_isConnected = true;
					return;
				}
			}
		}

		spdlog::warn("Failed to connect");
	}

	auto NetworkManager::disconnect() -> void
	{
		if (!m_isConnected)
		{
			return;
		}

		spdlog::debug("Disconnecting from server");

		m_messageQueue.clearInbound();
		m_messageQueue.clearOutbound();

		auto message              = Common::Network::Message();
		message.header.entityID   = getClientID();
		message.header.identifier = getNextMessageIdentifier();
		message.header.protocol   = Common::Network::Protocol::TCP;
		message.header.type       = Common::Network::MessageType::Client_Disconnect;

		auto buffer = message.pack();
		auto status = m_tcpSocket.send(buffer.data(), buffer.size());

		m_tcpSocket.disconnect();
		m_socketSelector.clear();

		m_isConnected = false;
	}

	auto NetworkManager::isConnected() const -> bool
	{
		return m_isConnected;
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
		message.header.entityID   = getClientID();
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

	auto NetworkManager::getClientID() -> entt::entity
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
		m_cryptographer.encrypt(buffer);

		auto status = m_udpSocket.send(buffer.data(), buffer.size(), Common::Network::SERVER_ADDRESS, Common::Network::UDP_PORT);
		switch (status)
		{
			case sf::Socket::Status::Done:
				// Success
			default:
				break;
		}
	}

	auto NetworkManager::receiveUDP() -> void
	{
		auto buffer = std::array<std::uint8_t, Common::Network::MAX_MESSAGE_LENGTH>();

		std::size_t length = 0;
		std::optional<sf::IpAddress> optAddress;
		std::uint16_t remotePort = 0;

		auto status = m_udpSocket.receive(buffer.data(), buffer.size(), length, optAddress, remotePort);
		if (status != sf::Socket::Status::Done)
		{
			return;
		}

		if (!optAddress.has_value())
		{
			return;
		}

		if (optAddress.value() != Common::Network::SERVER_ADDRESS)
		{
			return;
		}

		auto vBuffer = std::vector<std::uint8_t>(buffer.data(), buffer.data() + length);
		m_cryptographer.decryptFromRemote(vBuffer);

		auto message = Common::Network::Message();
		message.unpack(vBuffer);

		if (validateMessage(message))
		{
			m_messageQueue.pushInbound(std::move(message));
		}
	}

	auto NetworkManager::sendTCP(const Common::Network::Message& message) -> void
	{
		auto buffer = message.pack();
		m_cryptographer.encrypt(buffer);

		auto status = m_tcpSocket.send(buffer.data(), buffer.size());
		switch (status)
		{
			case sf::Socket::Status::Disconnected:
				spdlog::warn("Client was disconnected");
				disconnect();
				break;
			case sf::Socket::Status::Done:
			// Success
			default:
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
				auto vBuffer = std::vector<std::uint8_t>(buffer.data(), buffer.data() + length);
				m_cryptographer.decryptFromRemote(vBuffer);

				auto message = Common::Network::Message();
				message.unpack(vBuffer);

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
				break;
		}
	}

} // namespace Client