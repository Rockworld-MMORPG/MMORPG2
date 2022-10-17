#pragma once

#include "Network/Client.hpp"
#include "Server/Manager.hpp"
#include <Common/Network/Message.hpp>
#include <Common/Network/MessageQueue.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/UdpSocket.hpp>
#include <entt/entity/entity.hpp>
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

		auto update(sf::Time maxSelectorWaitTime) -> void;

		auto getNextMessage() -> std::optional<Common::Network::Message>;
		auto getMessages() -> std::vector<Common::Network::Message>;
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, entt::entity entityID, Common::Network::MessageData& data) -> void;
		auto pushMessage(Common::Network::Protocol protocol, Common::Network::MessageType type, Common::Network::MessageData& data) -> void;

		auto resolveClientID(sf::IpAddress ipAddress, std::uint16_t port) -> std::optional<entt::entity>;
		auto setClientUdpPort(entt::entity entityID, std::uint16_t udpPort) -> void;
		auto markForDisconnect(entt::entity entityID) -> void;

	private:
		auto generateClientID() -> entt::entity;
		auto acceptNewConnection() -> void;
		auto closeConnection(entt::entity entityID) -> void;
		auto disconnectClients() -> void;

		auto getNextMessageIdentifier() -> std::uint64_t;
		auto validateIncomingMessage(entt::entity entityID, Common::Network::MessageHeader& header) -> bool;

		auto sendUDP(Common::Network::Message& message) -> void;
		auto receiveUDP() -> void;

		auto sendTCP(Common::Network::Message& message) -> void;
		auto receiveTCP(entt::entity entityID, Client& client) -> void;

		sf::SocketSelector m_socketSelector;
		sf::TcpListener m_tcpListener;
		sf::UdpSocket m_udpSocket;

		std::unordered_map<std::uint64_t, entt::entity> m_clientIPMap;
		std::list<entt::entity> m_clientsPendingDisconnection;

		Common::Network::MessageQueue<Common::Network::Message> m_messageQueue;
		std::uint64_t m_currentMessageIdentifier = 0;
	};

} // namespace Server