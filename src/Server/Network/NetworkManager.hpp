#pragma once

#include "Common/Network/ClientID.hpp"
#include "Network/Client.hpp"
#include "Server/Manager.hpp"
#include <Common/Network/Message.hpp>
#include <Common/Network/MessageQueue.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <list>
#include <unordered_map>

namespace Server
{
	class NetworkManager : public Manager
	{
	public:
		NetworkManager(Server& server);
		~NetworkManager() override;

		auto init() -> bool;
		auto shutdown() -> void;

		auto update() -> void override;

		auto getNextMessage() -> std::optional<Common::Network::Message>;
		auto getMessages() -> std::vector<Common::Network::Message>;
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::ClientID clientID, Common::Network::MessageData& data) -> void;
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::MessageData& data) -> void;

		auto resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<Common::Network::ClientID>;
		auto setClientUdpPort(Common::Network::ClientID clientID, std::uint16_t udpPort) -> void;
		auto markForDisconnect(Common::Network::ClientID clientID) -> void;

	private:
		auto generateClientID() -> Common::Network::ClientID;
		auto acceptNewConnection() -> void;
		auto closeConnection(Common::Network::ClientID clientID) -> void;
		auto disconnectClients() -> void;

		auto getNextMessageIdentifier() -> std::uint64_t;
		auto validateIncomingMessage(Common::Network::ClientID clientID, Common::Network::MessageHeader& header) -> bool;

		auto sendUDP(Common::Network::Message& message) -> void;
		auto receiveUDP() -> void;

		auto sendTCP(Common::Network::Message& message) -> void;
		auto receiveTCP(Common::Network::ClientID clientID, Client& client) -> void;

		sf::SocketSelector m_socketSelector;
		sf::TcpListener m_tcpListener;
		sf::UdpSocket m_udpSocket;

		std::unordered_map<Common::Network::ClientID, Client> m_clientList;
		std::unordered_map<std::uint64_t, Common::Network::ClientID> m_clientIPMap;
		std::list<Common::Network::ClientID> m_clientsPendingDisconnection;

		Common::Network::MessageQueue<Common::Network::Message> m_messageQueue;

		Common::Network::ClientID m_nextClientID;
		std::uint64_t m_currentMessageIdentifier = 0;
	};

} // namespace Server