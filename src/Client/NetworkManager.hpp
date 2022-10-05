#pragma once

#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageQueue.hpp"
#include "SFML/Network/Packet.hpp"
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

		auto pushTCPMessage(sf::Packet&& packet) -> void;
		auto pushUDPMessage(sf::Packet&& packet) -> void;

		auto getNextTCPMessage() -> std::optional<sf::Packet>;
		auto getNextUDPMessage() -> std::optional<sf::Packet>;

		auto getClientID() -> Common::Network::ClientID;

	private:
		auto sendUDP() -> void;
		auto receiveUDP() -> void;

		auto sendTCP() -> void;
		auto receiveTCP() -> void;

		Common::Network::ClientID m_clientID;
		bool m_connected;

		sf::SocketSelector m_socketSelector;
		sf::UdpSocket m_udpSocket;
		sf::TcpSocket m_tcpSocket;

		Common::Network::MessageQueue<sf::Packet> m_tcpQueue;
		Common::Network::MessageQueue<sf::Packet> m_udpQueue;
	};

	extern NetworkManager g_networkManager;

} // namespace Client