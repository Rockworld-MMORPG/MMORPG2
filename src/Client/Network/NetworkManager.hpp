#pragma once

#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageQueue.hpp"
#include "Common/Network/Protocol.hpp"
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>

namespace Client
{

	class NetworkManager
	{
	public:
		auto init() -> void;
		auto shutdown() -> void;

		auto connect() -> void;
		auto disconnect() -> void;

		auto update() -> void;

		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::MessageData& messageData) -> void;
		auto getNextMessage() -> std::optional<Common::Network::Message>;
		auto getMessages() -> std::vector<Common::Network::Message>;

		auto getClientID() -> Common::Network::ClientID;

	private:
		auto getNextMessageIdentifier() -> std::uint64_t;

		auto sendUDP(const Common::Network::Message& message) -> void;
		auto receiveUDP() -> void;

		auto sendTCP(const Common::Network::Message& message) -> void;
		auto receiveTCP() -> void;

		Common::Network::ClientID m_clientID;
		std::uint64_t m_currentMessageIdentifier;
		bool m_connected;

		sf::SocketSelector m_socketSelector;
		sf::UdpSocket m_udpSocket;
		sf::TcpSocket m_tcpSocket;

		Common::Network::MessageQueue<Common::Network::Message> m_messageQueue;
	};

	extern NetworkManager g_networkManager;

} // namespace Client