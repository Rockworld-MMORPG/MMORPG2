#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/System/Sleep.hpp"
#include "SFML/System/Time.hpp"
#include "common/message.hpp"
#include <SFML/Network.hpp>
#include <common/server_properties.hpp>
#include <iostream>
#include <unordered_map>

const std::int32_t MIN_SERVER_TICK_TIME = 20;

struct Player
{
	sf::Vector2f position{0.0F, 0.0F};
};

auto main() -> int
{
	// Maps clients to entity IDs
	std::unordered_map<std::uint32_t, Player> players;

	sf::UdpSocket udpSocket;
	auto status = udpSocket.bind(Common::SERVER_PORT);
	if (status != sf::Socket::Status::Done)
	{
		return 1;
	}
	udpSocket.setBlocking(false);

	sf::Clock clock;
	clock.restart();
	bool shouldExit = false;

	while (!shouldExit)
	{
		auto deltaTime = clock.restart().asSeconds();

		sf::Packet packet{};
		std::optional<sf::IpAddress> remoteAddress{};
		std::uint16_t remotePort = 0;

		sf::Socket::Status status = sf::Socket::Status::NotReady;
		while ((status = udpSocket.receive(packet, remoteAddress, remotePort)) == sf::Socket::Status::Partial)
		{
		}

		if (status == sf::Socket::Status::Done)
		{
			uint32_t messageType{};
			packet >> messageType;
			if (messageType == Common::Message::Terminate)
			{
				shouldExit = true;
				break;
			}

			if (!players.contains(remoteAddress->toInteger()))
			{
				players.emplace(remoteAddress->toInteger(), Player{});
			}

			sf::Vector2f delta{0.0F, 0.0F};
			packet >> delta.x;
			packet >> delta.y;

			std::cout << "Got " << delta.x << ", " << delta.y << "\n";

			delta *= deltaTime * 200.0F;
			auto&& pos = players.at(remoteAddress->toInteger()).position;
			pos += delta;

			std::cout << "Pos is " << pos.x << ", " << pos.y << "\n";

			packet.clear();
			packet << Common::Message::Position << pos.x << pos.y;
			do
			{
				status = udpSocket.send(packet, remoteAddress.value(), Common::CLIENT_PORT);
			} while (status == sf::Socket::Status::Partial);
		}
		else
		{
			sf::sleep(sf::milliseconds(MIN_SERVER_TICK_TIME));
		}
	}


	std::cout << "Hello from server" << std::endl;
	return 0;
}