#pragma once

#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageQueue.hpp"
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/SocketSelector.hpp"
#include "SFML/Network/TcpListener.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include <list>
#include <unordered_map>

namespace Server
{
	struct Client;
	struct Message;

	class NetworkManager
	{
	public:
		auto init() -> bool;
		auto shutdown() -> void;

		auto update() -> void;

		auto getNextMessage() -> std::optional<Message>;
		auto pushMessage(Message&& message) -> void;

		auto resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<Common::Network::ClientID>;
		auto setClientUdpPort(Common::Network::ClientID clientID, std::uint16_t udpPort) -> void;
		auto markForDisconnect(Common::Network::ClientID clientID) -> void;

	private:
		auto generateClientID() -> Common::Network::ClientID;
		auto acceptNewConnection() -> void;
		auto closeConnection(Common::Network::ClientID clientID) -> void;
		auto disconnectClients() -> void;

		auto sendUDP(Message& message) -> void;
		auto receiveUDP() -> void;
		auto broadcastUDP(Message& message) -> void;

		auto sendTCP(Message& message) -> void;
		auto receiveTCP(Common::Network::ClientID clientID, Client& client) -> void;
		auto broadcastTCP(Message& message) -> void;

		sf::SocketSelector m_socketSelector;
		sf::TcpListener m_tcpListener;
		sf::UdpSocket m_udpSocket;

		std::unordered_map<Common::Network::ClientID, Client, Common::Network::ClientIDHash> m_clientList;
		std::unordered_map<std::uint64_t, Common::Network::ClientID> m_clientIPMap;
		std::list<Common::Network::ClientID> m_clientsPendingDisconnection;

		Common::Network::MessageQueue<Message> m_messageQueue;
	};

	extern NetworkManager g_networkManager;

} // namespace Server