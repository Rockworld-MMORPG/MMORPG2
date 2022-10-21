#include "Common/Network/MessageType.hpp"
#include <Common/Network/Message.hpp>
#include <Common/Network/ServerProperties.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>

auto main() -> int
{
	auto socket = sf::TcpSocket();
	auto status = socket.connect(Common::Network::SERVER_ADDRESS, Common::Network::TCP_PORT);

	if (status != sf::Socket::Status::Done)
	{
		spdlog::error("Failed to connect to server");
		return 1;
	}

	auto shouldExit                 = false;
	std::uint64_t messageIdentifier = 0;

	while (!shouldExit)
	{
		auto userInput = std::string();
		std::cout << "$> ";
		std::getline(std::cin, userInput);

		if (userInput == "exit")
		{
			shouldExit = true;
		}
		else
		{
			auto message              = Common::Network::Message();
			message.header.entityID   = entt::null;
			message.header.identifier = messageIdentifier++;
			message.header.protocol   = Common::Network::Protocol::TCP;
			message.header.type       = Common::Network::MessageType::Command;

			for (const auto c : userInput)
			{
				message.data << static_cast<std::uint8_t>(c);
			}

			auto buffer = message.pack();
			status      = socket.send(buffer.data(), buffer.size());

			switch (status)
			{
				case sf::Socket::Status::Done:
					break;
				case sf::Socket::Status::Disconnected:
					shouldExit = true;
				default:
					break;
			}
		}
	}
}