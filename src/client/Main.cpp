#include "Version.hpp"
#include "common/Network/ClientID.hpp"
#include "common/Network/MessageType.hpp"
#include "common/Network/ServerProperties.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <vector>

const unsigned int WINDOW_WIDTH  = 1280;
const unsigned int WINDOW_HEIGHT = 720;
const unsigned int BIT_DEPTH     = 32;

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

auto sendPlayerMovement(sf::UdpSocket& socket, sf::Vector2f position) -> void
{
}

auto receivePlayerPosition(sf::UdpSocket& socket) -> std::optional<sf::Vector2f>
{
	return {{0.0F, 0.0F}};
}

auto terminateServer(sf::TcpSocket& tcpSocket)
{
	sf::Packet packet{};
	packet << Common::Network::MessageType::Terminate;

	auto status = tcpSocket.send(packet);
	if (status != sf::Socket::Status::Done && status != sf::Socket::Status::Disconnected)
	{
		spdlog::warn("Failed to send server terminate command (error code {})", static_cast<std::uint32_t>(status));
	}
}

auto connectToServer(sf::TcpSocket& tcpSocket, std::uint16_t localPort) -> Common::Network::ClientID
{
	spdlog::debug("Connecting to server");
	auto status = sf::Socket::Status::NotReady;

	status = tcpSocket.connect(Common::Network::SERVER_ADDRESS, Common::Network::TCP_PORT);
	if (status != sf::Socket::Status::Done)
	{
		spdlog::warn("Failed to connect to the server (error code {})", static_cast<std::uint32_t>(status));
	}
	else
	{
		spdlog::debug("Connected to server successfully");
	}

	sf::Packet packet{};
	packet << Common::Network::MessageType::Connect << localPort;
	status = tcpSocket.send(packet);

	if (status != sf::Socket::Status::Done)
	{
		spdlog::warn("Failed to send connection message (error code {})", static_cast<std::uint32_t>(status));
	}
	else
	{
		spdlog::debug("Sent connection message requesting UDP port {}", localPort);
	}

	packet.clear();

	const std::size_t MAX_CONNECTION_ATTEMPTS = 5;
	for (auto attemptNumber = 0; attemptNumber < MAX_CONNECTION_ATTEMPTS; ++attemptNumber)
	{
		status = tcpSocket.receive(packet);
		if (status == sf::Socket::Status::Done)
		{
			Common::Network::MessageType messageType = Common::Network::MessageType::None;
			packet >> messageType;
			if (messageType == Common::Network::MessageType::Connect)
			{
				Common::Network::ClientID_t clientID = -1;
				packet >> clientID;
				spdlog::debug("Port requested successfully and granted client ID {}", clientID);
				return Common::Network::ClientID(clientID);
			}
		}
	}

	spdlog::warn("Failed to connect");
	return Common::Network::ClientID(-1);
}

auto disconnectFromServer(sf::TcpSocket& tcpSocket) -> void
{
	spdlog::debug("Disconnecting from server");

	sf::Packet packet{};
	packet << Common::Network::MessageType::Disconnect;
	auto status = sf::Socket::Status::NotReady;

	do
	{
		status = tcpSocket.send(packet);
	} while (status == sf::Socket::Status::Partial);

	if (status != sf::Socket::Status::Done)
	{
		spdlog::warn("Failed to send disconnect message (error code {})", static_cast<std::uint32_t>(status));
	}
	else
	{
		spdlog::debug("Sent disconnect message successfully");
	}

	while (tcpSocket.receive(packet) != sf::Socket::Status::Disconnected)
	{
		spdlog::debug("Waiting for the server to disconnect the socket");
		sf::sleep(sf::seconds(1.0F));
	}
}

auto main(int /* argc */, char** argv) -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	spdlog::info("Client version {}.{}.{}", Client::Version::getMajor(), Client::Version::getMinor(), Client::Version::getPatch());

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
		spdlog::warn("Failed to bind UDP socket");
		return 1;
	}
	udpSocket.setBlocking(false);
	std::uint16_t localPort = udpSocket.getLocalPort();
	spdlog::debug("UDP bound to port {}", localPort);

	sf::TcpSocket tcpSocket;
	tcpSocket.setBlocking(true);
	auto clientID = connectToServer(tcpSocket, localPort);

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
				disconnectFromServer(tcpSocket);
			}
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
		{
			terminateServer(tcpSocket);
			windowShouldClose = true;
		}

		auto playerMovement = getPlayerInput();


		// Render all sprites
		renderWindow.clear();
		for (auto& sprite : sprites)
		{
			renderWindow.draw(sprite);
		}
		renderWindow.display();
	}

	return 0;
}