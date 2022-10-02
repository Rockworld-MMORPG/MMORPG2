#pragma once

#include "SFML/Network/IpAddress.hpp"
#include <SFML/Network/Packet.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <cstdint>
#include <optional>
#include <queue>
#include <unordered_map>

namespace Server
{

	using ClientID = std::uint32_t;

	struct Client
	{
		std::unique_ptr<sf::TcpSocket> tcpSocket = nullptr;
		std::uint16_t udpPort                    = 0;
	};

	struct Message
	{
		ClientID clientID = 0;
		sf::Packet data{};
	};

	class NetworkManager
	{
	public:
		NetworkManager();

		auto update() -> void;

		auto getNextUDPMessage() -> std::optional<Message>;
		auto getNextTCPMessage() -> std::optional<Message>;

		auto pushUDPMessage(Message&& message) -> void;
		auto pushTCPMessage(Message&& message) -> void;

		auto closeConnection(ClientID clientID) -> void;
		auto setClientUdpPort(ClientID clientID, std::uint16_t udpPort) -> void;
		auto resolveClientID(sf::IpAddress& remoteAddress) -> std::optional<ClientID>;

	private:
		auto generateClientID() -> ClientID;
		auto acceptNewConnection() -> void;

		auto receiveUDP() -> void;
		auto receiveTCP(ClientID clientID, Client& client) -> void;

		auto sendTCP() -> void;
		auto sendUDP() -> void;

		sf::SocketSelector m_socketSelector;
		sf::TcpListener m_tcpListener;
		sf::UdpSocket m_udpSocket;

		std::unordered_map<ClientID, Client> m_clientList;

		std::queue<Message> m_inboundUDPQueue;
		std::queue<Message> m_outboundUDPQueue;

		std::queue<Message> m_inboundTCPQueue;
		std::queue<Message> m_outboundTCPQueue;

		ClientID m_nextClientID;
	};

} // namespace Server