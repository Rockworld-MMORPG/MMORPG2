#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/UdpSocket.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "common/common.hpp"
#include "common/message.hpp"
#include "common/server_properties.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

const unsigned int WINDOW_WIDTH  = 1280;
const unsigned int WINDOW_HEIGHT = 720;
const unsigned int BIT_DEPTH     = 32;

const float PLAYER_MOVE_SPEED = 100.0F;

const sf::IpAddress SERVER_ADDRESS{127, 0, 0, 1};

auto getPlayerInput() -> sf::Vector2f
{
	sf::Vector2f playerDelta{0.0F, 0.0F};

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
	{
		playerDelta.x -= 1.0F;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
	{
		playerDelta.x += 1.0F;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
	{
		playerDelta.y -= 1.0F;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
	{
		playerDelta.y += 1.0F;
	}

	return playerDelta;
}

auto sendPlayerMovement(sf::UdpSocket& socket, const sf::Vector2f& movement) -> void
{
	sf::Packet packet{};
	packet << static_cast<std::uint32_t>(Common::Message::Movement) << movement.x << movement.y;
	sf::Socket::Status status = sf::Socket::Status::NotReady;
	do
	{
		status = socket.send(packet, SERVER_ADDRESS, Common::UDP_PORT);
	} while (status == sf::Socket::Status::Partial);
}

auto receivePlayerPosition(sf::UdpSocket& socket) -> std::optional<sf::Vector2f>
{
	sf::Packet packet{};
	std::optional<sf::IpAddress> remoteAddress{};
	std::uint16_t remotePort  = 0;
	sf::Socket::Status status = sf::Socket::Status::NotReady;
	do
	{
		status = socket.receive(packet, remoteAddress, remotePort);
	} while (status == sf::Socket::Status::Partial);

	if (status == sf::Socket::Status::Done)
	{
		sf::Vector2f playerPosition{0.0F, 0.0F};
		auto type = Common::Message::None;
		packet >> type >> playerPosition.x >> playerPosition.y;
		return playerPosition;
	}

	return std::optional<sf::Vector2f>{};
}

auto connectToServer(sf::TcpSocket& tcpSocket, std::uint16_t localPort) -> void
{
	std::cout << "Connecting to server\n";
	auto status = sf::Socket::Status::NotReady;

	status = tcpSocket.connect(Common::SERVER_ADDRESS, Common::TCP_PORT);
	if (status != sf::Socket::Status::Done)
	{
		std::cout << "Failed to connect\n";
		std::cout << "Error " << static_cast<std::uint32_t>(status) << "\n";
	}
	else
	{
		std::cout << "Connection successful\n";
	}

	sf::Packet packet{};
	packet << static_cast<std::uint32_t>(Common::Message::Connect) << localPort;
	status = tcpSocket.send(packet);

	if (status != sf::Socket::Status::Done)
	{
		std::cout << "Failed to send connection message\n";
	}
	else
	{
		std::cout << "Connected to server and requested UDP port " << localPort << "\n";
	}
}

auto disconnectFromServer(sf::TcpSocket& tcpSocket) -> void
{
	std::cout << "Disconnecting from server\n";

	sf::Packet packet{};
	packet << static_cast<std::uint32_t>(Common::Message::Disconnect);
	auto status = sf::Socket::Status::NotReady;

	do
	{
		status = tcpSocket.send(packet);
	} while (status == sf::Socket::Status::Partial);

	if (status != sf::Socket::Status::Done)
	{
		std::cout << "Failed to send disconnect message\n";
	}
	else
	{
		std::cout << "Disconnected successfully\n";
	}

	while (tcpSocket.receive(packet) != sf::Socket::Status::Disconnected)
	{
		std::cout << "Awaiting disconnect from server\n";
	}
}

auto main(int /* argc */, char** argv) -> int
{
	auto executablePath = std::filesystem::path(*argv);
	std::filesystem::current_path(executablePath.parent_path());

	std::vector<sf::Sprite> sprites{};
	bool windowShouldClose = false;
	sf::RenderWindow renderWindow;
	renderWindow.create(sf::VideoMode{sf::Vector2u{WINDOW_WIDTH, WINDOW_HEIGHT}, BIT_DEPTH}, "Client");
	renderWindow.setVerticalSyncEnabled(true);

	sf::UdpSocket udpSocket;
	auto status = udpSocket.bind(sf::Socket::AnyPort);
	if (status != sf::Socket::Status::Done)
	{
		std::cout << "Failed to bind UDP socket\n";
		return 1;
	}
	udpSocket.setBlocking(false);
	std::uint16_t localPort = udpSocket.getLocalPort();
	std::cout << "UDP bound to port " << localPort << "\n";

	sf::TcpSocket tcpSocket;
	tcpSocket.setBlocking(true);
	connectToServer(tcpSocket, localPort);

	sf::Clock clock;
	clock.restart();

	sf::Texture playerTexture;
	if (!playerTexture.loadFromFile("assets/player.png"))
	{
		return 2;
	}

	sprites.emplace_back(sf::Sprite{});
	auto&& player = sprites.back();
	player.setTexture(playerTexture);
	player.setPosition(sf::Vector2f{0.0F, 0.0F});

	sendPlayerMovement(udpSocket, player.getPosition());
	auto playerPosition = receivePlayerPosition(udpSocket);
	if (playerPosition.has_value())
	{
		player.setPosition(playerPosition.value());
	}

	while (!windowShouldClose)
	{
		// Get the time elapsed since the previous frame
		float deltaTime = clock.restart().asSeconds();

		// Check if the window is closed
		sf::Event event{};
		while (renderWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				windowShouldClose = true;
			}
		}

		auto playerMovement = getPlayerInput();

		// Send player input if there is any
		if (playerMovement.length() != 0.0F)
		{
			sendPlayerMovement(udpSocket, playerMovement);
		}

		// Receive the player position from the server
		auto playerPosition = receivePlayerPosition(udpSocket);
		if (playerPosition.has_value())
		{
			player.setPosition(playerPosition.value());
		}

		// Render all sprites
		renderWindow.clear();
		for (auto& sprite : sprites)
		{
			renderWindow.draw(sprite);
		}
		renderWindow.display();
	}

	disconnectFromServer(tcpSocket);

	return 0;
}