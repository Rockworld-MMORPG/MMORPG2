#include "Common/World/Tile.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "TerrainRenderer.hpp"
#include "TerrainTile.hpp"
#include <Common/World/Level.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

auto saveLevel(const std::filesystem::path& filepath, Common::World::Level& level) -> void
{
	auto writer = std::ofstream(filepath, std::ios::binary | std::ios::out);

	for (auto yIndex = 0; yIndex < Common::World::LEVEL_HEIGHT; ++yIndex)
	{
		for (auto xIndex = 0; xIndex < Common::World::LEVEL_WIDTH; ++xIndex)
		{
			auto tile = level.getTile(xIndex, yIndex);
			writer << tile.type[0];
			writer << tile.type[1];
			writer << tile.type[2];
			writer << static_cast<std::uint8_t>(tile.travelMode);
		}
	}

	writer.close();
}

auto readLevel(const std::filesystem::path& filepath) -> Common::World::Level
{
	if (!std::filesystem::exists(filepath))
	{
		return {};
	}

	auto reader   = std::ifstream(filepath, std::ios::binary | std::ios::ate | std::ios::in);
	auto fileSize = reader.tellg();

	const auto targetSize = Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * sizeof(Common::World::Tile);
	if (fileSize != targetSize)
	{
		spdlog::error("File is not the right size {}B/{}B", fileSize, targetSize);
		return {};
	}

	auto buffer = std::array<char, targetSize>();
	reader.seekg(std::ios::beg);
	reader.read(buffer.data(), fileSize);
	reader.close();

	auto level       = Common::World::Level();
	auto bufferIndex = std::size_t(0);
	for (auto yIndex = 0; yIndex < Common::World::LEVEL_HEIGHT; ++yIndex)
	{
		for (auto xIndex = 0; xIndex < Common::World::LEVEL_WIDTH; ++xIndex)
		{
			auto tile       = Common::World::Tile();
			tile.type[0]    = static_cast<std::uint8_t>(buffer.at(bufferIndex++));
			tile.type[1]    = static_cast<std::uint8_t>(buffer.at(bufferIndex++));
			tile.type[2]    = static_cast<std::uint8_t>(buffer.at(bufferIndex++));
			tile.travelMode = static_cast<Common::World::Tile::TravelMode>(buffer.at(bufferIndex++));
			level.setTile(xIndex, yIndex, tile);
		}
	}

	return level;
}

auto main(int argc, char** argv) -> int
{
	if (argc != 2)
	{
		spdlog::error("Wrong number of parameters");
		return 1;
	}

	auto filepath = std::filesystem::path(argv[1]);
	auto level    = Common::World::Level();
	if (std::filesystem::exists(filepath))
	{
		level = readLevel(filepath);
	}

	auto renderWindow = sf::RenderWindow();
	renderWindow.create(sf::VideoMode(sf::Vector2u(1280, 1280)), fmt::format("Level Editor <{}>", filepath.string()));
	renderWindow.setVerticalSyncEnabled(true);

	auto font = sf::Font();
	font.loadFromFile(std::filesystem::path("OpenSans-Regular.ttf"));

	auto view = sf::View();
	view.setCenter(sf::Vector2f(0.0F, 0.0F));
	view.setSize(sf::Vector2f(TILE_SCALE * 32, TILE_SCALE * 32));
	view.setViewport(sf::FloatRect(sf::Vector2f(0.0F, 0.0F), sf::Vector2f(1.0F, 1280.0F / std::max(renderWindow.getSize().x, renderWindow.getSize().y))));

	auto viewOffset = view.getSize() * 0.5F;

	auto positionReadout
	    = sf::Text();
	positionReadout.setFont(font);
	positionReadout.setPosition(sf::Vector2f(0.0F, 0.0F));
	positionReadout.setCharacterSize(18);

	auto tileSelector = sf::RectangleShape();
	tileSelector.setSize(sf::Vector2f(TILE_SCALE, TILE_SCALE));
	tileSelector.setFillColor(sf::Color::Transparent);
	tileSelector.setOutlineColor(sf::Color::Red);
	tileSelector.setOutlineThickness(2.0F);

	auto mousePosition
	    = sf::Vector2f(0.0F, 0.0F);
	auto prevMousePosition = sf::Vector2f(0.0F, 0.0F);

	auto lmbPressed = false;
	auto mmbPressed = false;
	auto rmbPressed = false;

	auto terrainRenderer = TerrainRenderer();
	terrainRenderer.addLevel(level);

	while (renderWindow.isOpen())
	{
		auto cursorPosition = mousePosition + (view.getCenter() - viewOffset);

		auto tilePosition = static_cast<sf::Vector2i>(cursorPosition) / static_cast<std::int32_t>(TILE_SCALE);
		tilePosition.x    = std::clamp(tilePosition.x, 0, static_cast<std::int32_t>(Common::World::LEVEL_WIDTH - 1));
		tilePosition.y    = std::clamp(tilePosition.y, 0, static_cast<std::int32_t>(Common::World::LEVEL_HEIGHT - 1));

		auto fTilePosition = static_cast<sf::Vector2f>(tilePosition);

		tileSelector.setPosition(fTilePosition * TILE_SCALE);
		positionReadout.setString(fmt::format("X: {:.2f}\tY: {:.2f}\nX: {:3}\tY:{:3}", mousePosition.x, mousePosition.y, tilePosition.x, tilePosition.y));


		renderWindow.clear(sf::Color::Magenta);
		// World view
		renderWindow.setView(view);
		terrainRenderer.render(renderWindow);
		renderWindow.draw(tileSelector);

		// UI view
		renderWindow.setView(renderWindow.getDefaultView());
		renderWindow.draw(positionReadout);
		renderWindow.display();

		auto event = sf::Event();
		while (renderWindow.pollEvent(event))
		{
			switch (event.type)
			{
				case sf::Event::Closed:
					renderWindow.close();
					break;
				case sf::Event::MouseMoved:
				{
					prevMousePosition = mousePosition;
					mousePosition     = sf::Vector2f(event.mouseMove.x, event.mouseMove.y);

					auto delta = prevMousePosition - mousePosition;
					if (mmbPressed)
					{
						view.move(delta);
					}
				}
				break;
				case sf::Event::MouseButtonPressed:
				{
					switch (event.mouseButton.button)
					{
						case sf::Mouse::Button::Left:
							lmbPressed = true;
							break;
						case sf::Mouse::Button::Middle:
							mmbPressed = true;
							break;
						case sf::Mouse::Button::Right:
							rmbPressed = true;
							break;
					}
				}
				break;
				case sf::Event::MouseButtonReleased:
				{
					switch (event.mouseButton.button)
					{
						case sf::Mouse::Button::Left:
						{
							auto tile = level.getTile(tilePosition.x, tilePosition.y);
							tile.type[0] += 25;
							tile.type[1] += 25;
							tile.type[2] += 25;
							tile.travelMode = Common::World::Tile::TravelMode::Fly | Common::World::Tile::TravelMode::Walk;
							level.setTile(tilePosition.x, tilePosition.y, tile);

							terrainRenderer.clear();
							terrainRenderer.addLevel(level);

							lmbPressed = false;
						}
						break;
						case sf::Mouse::Button::Middle:
							mmbPressed = false;
							break;
						case sf::Mouse::Button::Right:
						{
							auto tile       = Common::World::Tile();
							tile.type[0]    = 0;
							tile.type[1]    = 0;
							tile.type[2]    = 0;
							tile.travelMode = Common::World::Tile::TravelMode::Fly | Common::World::Tile::TravelMode::Walk;
							level.setTile(tilePosition.x, tilePosition.y, tile);

							terrainRenderer.clear();
							terrainRenderer.addLevel(level);

							rmbPressed = false;
						}
						break;
					}
				}
				break;
				case sf::Event::KeyReleased:
				{
					switch (event.key.code)
					{
						case sf::Keyboard::Key::S:
							if (event.key.control)
							{
								spdlog::info("Saving level as {}", filepath.string());
								saveLevel(filepath, level);
							}
							break;
						case sf::Keyboard::Key::Comma:
						{
							renderWindow.setMouseCursorGrabbed(true);
						}
						break;
						case sf::Keyboard::Key::Period:
						{
							renderWindow.setMouseCursorGrabbed(false);
						}
						break;
						default:
							break;
					}
				}
				break;
				default:
					break;
			}
		}
	}
	return 0;
}