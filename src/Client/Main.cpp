#include "Common/Network/ClientID.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/ServerProperties.hpp"
#include "NetworkManager.hpp"
#include "Version.hpp"
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


auto main(int /* argc */, char** argv) -> int
{
#if !defined(NDEBUG)
	spdlog::set_level(spdlog::level::debug);
#endif

	spdlog::info("Client version {}.{}.{}", Client::Version::getMajor(), Client::Version::getMinor(), Client::Version::getPatch());

	auto executablePath = std::filesystem::path(*argv);
	std::filesystem::current_path(executablePath.parent_path());

	spdlog::debug("Initialising the network manager");
	Client::g_networkManager.init();

	std::vector<sf::Sprite> sprites{};
	bool windowShouldClose = false;
	sf::RenderWindow renderWindow;
	renderWindow.create(sf::VideoMode{sf::Vector2u{WINDOW_WIDTH, WINDOW_HEIGHT}, BIT_DEPTH}, "Client");
	renderWindow.setVerticalSyncEnabled(true);

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

	Client::g_networkManager.connect();

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

		Client::g_networkManager.update();

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
		{
			sf::Packet packet;
			packet << Common::Network::MessageType::Terminate;
			Client::g_networkManager.pushTCPMessage(std::move(packet));
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

	Client::g_networkManager.shutdown();

	return 0;
}