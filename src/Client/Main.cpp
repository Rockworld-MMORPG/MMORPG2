#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "Common/Network/ServerProperties.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "Version.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <unordered_map>
#include <vector>

const unsigned int WINDOW_WIDTH  = 1280;
const unsigned int WINDOW_HEIGHT = 720;
const unsigned int BIT_DEPTH     = 32;

using namespace Client;

auto sendPlayerInput() -> sf::Vector2f
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
	{
		auto data = Common::Network::MessageData();
		g_networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Terminate, data);
		return {0.0F, 0.0F};
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
	{
		auto data = Common::Network::MessageData();
		g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
		return {0.0F, 0.0F};
	}

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

	if (playerDelta.lengthSq() > 0.0F)
	{
		auto data = Common::Network::MessageData();
		data << playerDelta.x << playerDelta.y;
		g_networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Movement, data);
	}

	return playerDelta;
}

auto parseUDP(Common::Network::Message& message, std::unordered_map<Common::Network::ClientID, sf::Sprite, Common::Network::ClientIDHash>& sprites, sf::Texture& playerTexture) -> void
{
	switch (message.header.type)
	{
		case Common::Network::MessageType::Position:
		{
			Common::Network::ClientID_t clientID = -1;
			message.data >> clientID;

			auto iterator = sprites.find(Common::Network::ClientID(clientID));
			if (iterator == sprites.end())
			{
				auto [pair, success] = sprites.emplace(clientID, sf::Sprite{});
				pair->second.setTexture(playerTexture);
				iterator = pair;
			}

			sf::Vector2f remotePosition;
			message.data >> remotePosition.x >> remotePosition.y;
			auto clientPosition = iterator->second.getPosition();

			const float MAX_CLIENT_POSITION_ERROR = 50.0F;
			if ((clientPosition - remotePosition).lengthSq() > (MAX_CLIENT_POSITION_ERROR * MAX_CLIENT_POSITION_ERROR))
			{
				const float LERP_RATE = 0.01F;
				auto lerpedPosition   = clientPosition + (remotePosition - clientPosition) * LERP_RATE;
				iterator->second.setPosition(lerpedPosition);
			}
		}
		break;
		case Common::Network::MessageType::CreateEntity:
		{
			Common::Network::ClientID_t clientID = -1;
			message.data >> clientID;
			if (clientID == -1)
			{
				// This probably wasn't supposed to happen
				break;
			}

			auto iterator = sprites.find(Common::Network::ClientID(clientID));
			if (iterator == sprites.end())
			{
				auto [pair, success] = sprites.emplace(clientID, sf::Sprite{});
				pair->second.setTexture(playerTexture);
			}
		}
		break;
		case Common::Network::MessageType::DestroyEntity:
		{
			Common::Network::ClientID_t clientID = -1;
			message.data >> clientID;
			sprites.erase(Common::Network::ClientID(clientID));
		}
		break;
		default:
			// Do nothing
			break;
	}
}

auto parseMessages(std::unordered_map<Common::Network::ClientID, sf::Sprite, Common::Network::ClientIDHash>& sprites, sf::Texture& playerTexture)
{
	auto messages = g_networkManager.getMessages();
	for (auto& message : messages)
	{
		switch (message.header.protocol)
		{
			case Common::Network::Protocol::TCP:
				break;
			case Common::Network::Protocol::UDP:
				parseUDP(message, sprites, playerTexture);
				break;
		}
	}
}

auto main(int /* argc */, char** argv) -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::trace);
#endif

	spdlog::info("Client version {}.{}.{}", Version::getMajor(), Version::getMinor(), Version::getPatch());

	auto executablePath      = std::filesystem::path(*argv);
	const auto executableDir = executablePath.parent_path();
	const auto assetDir      = executableDir / "assets";

	spdlog::debug("Initialising the network manager");
	g_networkManager.init();

	std::unordered_map<Common::Network::ClientID, sf::Sprite, Common::Network::ClientIDHash> sprites;

	sf::RenderWindow renderWindow;
	renderWindow.create(sf::VideoMode{sf::Vector2u{WINDOW_WIDTH, WINDOW_HEIGHT}, BIT_DEPTH}, "Client");
	renderWindow.setFramerateLimit(60);

	sf::Clock clock;
	clock.restart();

	sf::Texture playerTexture;
	if (!playerTexture.loadFromFile(assetDir / "player.png"))
	{
		return 2;
	}

	g_networkManager.connect();

	bool windowShouldClose = false;
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


		g_networkManager.update();

		auto movement = sendPlayerInput();
		// Predict player movement
		auto iterator = sprites.find(g_networkManager.getClientID());
		if (iterator != sprites.end())
		{
			iterator->second.move(movement * 200.0F * deltaTime);
		}

		parseMessages(sprites, playerTexture);

		// Render all sprites
		renderWindow.clear();
		for (auto& [id, sprite] : sprites)
		{
			renderWindow.draw(sprite);
		}
		renderWindow.display();
	}

	g_networkManager.shutdown();

	return 0;
}