#include "Common/World/Tile.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "TerrainRenderer.hpp"
#include "TerrainTile.hpp"
#include <Common/World/Level.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

const auto MAX_CURSOR_POS_X = TILE_SCALE * Common::World::LEVEL_WIDTH;
const auto MAX_CURSOR_POS_Y = TILE_SCALE * Common::World::LEVEL_HEIGHT;

auto level = Common::World::Level();

auto cursorPosition = sf::Vector2f();
auto tilePosition   = sf::Vector2i();

auto validCursorPosition() -> bool
{
	return (cursorPosition.x >= 0 && cursorPosition.x < MAX_CURSOR_POS_X && cursorPosition.y >= 0 && cursorPosition.y < MAX_CURSOR_POS_Y);
}

auto saveLevel(const std::filesystem::path& filepath) -> void
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

auto readLevel(const std::filesystem::path& filepath) -> void
{
	if (!std::filesystem::exists(filepath))
	{
		return;
	}

	auto reader   = std::ifstream(filepath, std::ios::binary | std::ios::ate | std::ios::in);
	auto fileSize = reader.tellg();

	const auto targetSize = Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * sizeof(Common::World::Tile);
	if (fileSize != targetSize)
	{
		spdlog::error("File is not the right size {}B/{}B", fileSize, targetSize);
		return;
	}

	auto buffer = std::array<char, targetSize>();
	reader.seekg(std::ios::beg);
	reader.read(buffer.data(), fileSize);
	reader.close();

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
}

auto showFileInfoWindow(std::filesystem::path filepath)
{
	static std::string filepathInput = filepath.string();

	ImGui::Begin("File info");
	ImGui::InputText("Filepath", &filepathInput);
	if (ImGui::Button("Save"))
	{
		saveLevel(filepathInput);
	}
	if (ImGui::Button("Load"))
	{
		readLevel(filepathInput);
	}
	ImGui::End();
}

auto showTileInfoWindow()
{
	ImGui::Begin("Tile info");
	ImGui::Text("X position: %i", tilePosition.x);
	ImGui::Text("Y position: %i", tilePosition.y);
	ImGui::Text("X position: %.2f", cursorPosition.x);
	ImGui::Text("Y position: %.2f", cursorPosition.y);
	ImGui::End();
}

auto main(int argc, char** argv) -> int
{
	if (argc != 2)
	{
		spdlog::error("Wrong number of parameters");
		return 1;
	}

	auto filepath = std::filesystem::path(argv[1]);
	if (std::filesystem::exists(filepath))
	{
		readLevel(filepath);
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

	if (!ImGui::SFML::Init(renderWindow))
	{
		return 2;
	}

	sf::Clock deltaClock;

	while (renderWindow.isOpen())
	{
		auto event = sf::Event();
		while (renderWindow.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(renderWindow, event);

			switch (event.type)
			{
				case sf::Event::Closed:
					renderWindow.close();
					break;
				case sf::Event::Resized:
					view.setSize(static_cast<sf::Vector2f>(renderWindow.getSize()));
					viewOffset = view.getSize() * 0.5F;
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
							if (!validCursorPosition()) { break; }

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
							if (!validCursorPosition()) { break; }

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
					break;
				default:
					break;
			}
		}

		ImGui::SFML::Update(renderWindow, deltaClock.restart());

		cursorPosition = mousePosition + (view.getCenter() - viewOffset);

		tilePosition = sf::Vector2i(
		                   std::clamp(cursorPosition.x, 0.0F, MAX_CURSOR_POS_X),
		                   std::clamp(cursorPosition.y, 0.0F, MAX_CURSOR_POS_Y))
		               / static_cast<std::int32_t>(TILE_SCALE);

		auto fTilePosition = static_cast<sf::Vector2f>(tilePosition);

		tileSelector.setPosition(fTilePosition * TILE_SCALE);

		showFileInfoWindow(filepath);
		showTileInfoWindow();

		renderWindow.clear(sf::Color::Magenta);
		// World view
		renderWindow.setView(view);
		terrainRenderer.render(renderWindow);

		if (validCursorPosition())
		{
			renderWindow.draw(tileSelector);
		}

		// UI view
		ImGui::SFML::Render(renderWindow);
		renderWindow.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
if