#include "Common/World/Tile.hpp"
#include "TerrainRenderer.hpp"
#include "TerrainTile.hpp"
#include "spdlog/common.h"
#include <Common/World/Level.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

auto assetDir = std::filesystem::path();

const auto MAX_CURSOR_POS_X = TILE_SCALE * Common::World::LEVEL_WIDTH;
const auto MAX_CURSOR_POS_Y = TILE_SCALE * Common::World::LEVEL_HEIGHT;

auto viewZoomScale = float(1.0F);

auto currentlyLoadedLevel = std::filesystem::path();
auto level                = Common::World::Level();

auto tileMap         = std::unordered_map<std::string, std::tuple<sf::Texture, sf::Sprite, Common::World::Tile>>();
auto terrainRenderer = TerrainRenderer();

enum class Tools
{
	None,
	Brush,
	Fill
};
auto currentlySelectedTool = Tools::None;

enum class Shape
{
	Circle,
	Square
};
const auto MIN_BRUSH_SIZE = std::int32_t(1);
const auto MAX_BRUSH_SIZE = std::int32_t(32);
auto brushSize            = MIN_BRUSH_SIZE;
auto brushShape           = Shape::Square;

auto cursorPosition = sf::Vector2f();
auto tilePosition   = sf::Vector2i();

auto selectedTile = Common::World::Tile();


auto validCursorPosition() -> bool
{
	return (cursorPosition.x >= 0 && cursorPosition.x < MAX_CURSOR_POS_X && cursorPosition.y >= 0 && cursorPosition.y < MAX_CURSOR_POS_Y);
}

auto loadTile(std::string identifier, std::filesystem::path filepath) -> void
{
	auto correctedFilepath = assetDir / filepath;
	if (!std::filesystem::exists(correctedFilepath))
	{
		return;
	}

	auto [pair, success] = tileMap.emplace(identifier, std::make_tuple<sf::Texture, sf::Sprite, Common::World::Tile>(sf::Texture(), sf::Sprite(), Common::World::Tile()));

	auto& texture = std::get<sf::Texture>(pair->second);
	texture.loadFromFile(correctedFilepath);

	auto& sprite = std::get<sf::Sprite>(pair->second);
	auto& tile   = std::get<Common::World::Tile>(pair->second);


	auto averageColour = sf::Color();
	{
		auto image = texture.copyToImage();
		auto r     = std::uint64_t(0);
		auto g     = std::uint64_t(0);
		auto b     = std::uint64_t(0);

		for (auto y = 0; y < image.getSize().y; ++y)
		{
			for (auto x = 0; x < image.getSize().x; ++x)
			{
				auto iC = image.getPixel(sf::Vector2u(x, y));
				r += iC.r;
				g += iC.g;
				b += iC.b;
			}
		}

		averageColour.r = r / (image.getSize().x * image.getSize().y);
		averageColour.g = g / (image.getSize().x * image.getSize().y);
		averageColour.b = b / (image.getSize().x * image.getSize().y);
	}

	sprite.setTexture(texture);
	tile = Common::World::Tile{
	    {averageColour.r, averageColour.g, averageColour.b},
	    Common::World::Tile::TravelMode::Walk | Common::World::Tile::TravelMode::Fly};
}

auto saveLevel() -> void
{
	auto writer = std::ofstream(currentlyLoadedLevel, std::ios::binary | std::ios::out);

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
	spdlog::debug("Saved level");
}

auto readLevel(const std::filesystem::path& filepath) -> void
{
	auto correctedFilepath = (filepath.is_absolute()) ? filepath : assetDir / filepath;
	spdlog::debug("Loading {}", correctedFilepath.string());

	if (!std::filesystem::exists(correctedFilepath))
	{
		spdlog::debug("File does not exist");
		return;
	}

	auto reader   = std::ifstream(correctedFilepath, std::ios::binary | std::ios::ate | std::ios::in);
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

	currentlyLoadedLevel = correctedFilepath.relative_path();
	terrainRenderer.clear();
	terrainRenderer.addLevel(level);
}

auto showFileInfoWindow() -> void
{
	static std::string filepathInput = currentlyLoadedLevel.relative_path().string();

	ImGui::Begin("File info");
	ImGui::InputText("Filepath", &filepathInput);
	if (ImGui::Button("Save"))
	{
		saveLevel();
	}
	if (ImGui::Button("Load"))
	{
		readLevel(filepathInput);
	}
	ImGui::End();
}

auto showTileInfoWindow() -> void
{
	ImGui::Begin("Tile info");
	ImGui::Text("X position: %i", tilePosition.x);
	ImGui::Text("Y position: %i", tilePosition.y);
	ImGui::Text("X position: %.2f", cursorPosition.x);
	ImGui::Text("Y position: %.2f", cursorPosition.y);
	ImGui::End();
}

auto showTileSelectorWindow() -> void
{
	ImGui::Begin("Tile palette");
	auto i = std::size_t(0);
	for (const auto& [id, tuple] : tileMap)
	{
		++i;
		ImGui::SameLine();
		if (ImGui::ImageButton(std::get<sf::Sprite>(tuple)))
		{
			selectedTile = std::get<Common::World::Tile>(tuple);
		};
		if (i == 4)
		{
			ImGui::Spacing();
			i = 0;
		}
	}

	ImGui::End();
}

auto showToolsWindow() -> void
{
	ImGui::Begin("Tools");
	if (ImGui::Button("None"))
	{
		currentlySelectedTool = Tools::None;
	}
	ImGui::SameLine();
	if (ImGui::Button("Brush"))
	{
		currentlySelectedTool = Tools::Brush;
	}
	ImGui::SameLine();
	if (ImGui::Button("Fill"))
	{
		currentlySelectedTool = Tools::Fill;
	}

	ImGui::Spacing();

	switch (currentlySelectedTool)
	{
		case Tools::Brush:
			ImGui::Text("Currently selected tool: Brush");
			ImGui::SliderInt("Size", &brushSize, MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
			break;
		case Tools::Fill:
			ImGui::Text("Currently selected tool: Fill");
			break;
		default:
			ImGui::Text("Currently selected tool: None");
			break;
	}
	ImGui::End();
}

auto useTool()
{
	switch (currentlySelectedTool)
	{
		case Tools::Brush:
		{
			switch (brushShape)
			{
				case Shape::Circle:

					break;
				case Shape::Square:
				{
					auto halfSize = brushSize / 2;
					if (brushSize % 2 == 1)
					{
						++halfSize;
					}

					for (auto y = tilePosition.y - (brushSize / 2); y < tilePosition.y + halfSize; ++y)
					{
						for (auto x = tilePosition.x - (brushSize / 2); x < tilePosition.x + halfSize; ++x)
						{
							level.setTile(x, y, selectedTile);
						}
					}
				}
				break;
			}
		}
		break;
		case Tools::Fill:
		{
			auto initialTile = level.getTile(tilePosition.x, tilePosition.y);
			for (auto y = 0; y < Common::World::LEVEL_HEIGHT; ++y)
			{
				for (auto x = 0; x < Common::World::LEVEL_WIDTH; ++x)
				{
					if (level.getTile(x, y).type[0] == initialTile.type[0])
					{
						level.setTile(x, y, selectedTile);
					}
				}
			}
		}
		break;
		default:
			break;
	}
}

auto main(int argc, char** argv) -> int
{
#if not defined NDEBUG
	spdlog::set_level(spdlog::level::debug);
#endif

	assetDir = std::filesystem::path(argv[0]).parent_path() / "assets";

	if (argc == 2)
	{
		currentlyLoadedLevel = assetDir / argv[1];
		if (std::filesystem::exists(currentlyLoadedLevel))
		{
			readLevel(currentlyLoadedLevel);
		}
		else
		{
			currentlyLoadedLevel = assetDir / "autosave.dat";
		}
	}

	selectedTile.type[0] = 30;
	selectedTile.type[1] = 200;
	selectedTile.type[2] = 70;

	auto renderWindow = sf::RenderWindow();
	renderWindow.create(sf::VideoMode(sf::Vector2u(1280, 1280)), "Level Editor");
	renderWindow.setVerticalSyncEnabled(true);

	loadTile("grass_centre", "tile_grass_centre.png");
	loadTile("water_centre", "tile_water_centre.png");
	loadTile("grass_water_east", "tile_grass_water_east.png");
	loadTile("grass_water_west", "tile_grass_water_west.png");
	loadTile("grass_water_north", "tile_grass_water_north.png");
	loadTile("grass_water_south", "tile_grass_water_south.png");

	auto font = sf::Font();
	font.loadFromFile(std::filesystem::path("OpenSans-Regular.ttf"));

	auto view = sf::View();
	view.setCenter(sf::Vector2f(0.0F, 0.0F));
	view.setSize(sf::Vector2f(TILE_SCALE * 32, TILE_SCALE * 32));

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
					view.setSize(static_cast<sf::Vector2f>(renderWindow.getSize()) * viewZoomScale);
					break;
				case sf::Event::MouseMoved:
				{
					prevMousePosition = mousePosition;
					mousePosition     = sf::Vector2f(event.mouseMove.x, event.mouseMove.y) * viewZoomScale;

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

							useTool();
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
							rmbPressed = false;
						}
						break;
					}
				}
				break;
				case sf::Event::MouseWheelScrolled:
				{
					auto scroll = event.mouseWheelScroll;
					if (scroll.delta > 0.0F)
					{
						viewZoomScale -= 0.1F;
					}
					else if (scroll.delta < 0.0F)
					{
						viewZoomScale += 0.1F;
					}
					view.setSize(static_cast<sf::Vector2f>(renderWindow.getSize()) * viewZoomScale);
				}
				break;
				default:
					break;
			}
		}

		ImGui::SFML::Update(renderWindow, deltaClock.restart());

		auto viewOffset = view.getSize() * 0.5F;
		cursorPosition  = mousePosition + (view.getCenter() - viewOffset);

		tilePosition = sf::Vector2i(
		                   std::clamp(cursorPosition.x, 0.0F, MAX_CURSOR_POS_X),
		                   std::clamp(cursorPosition.y, 0.0F, MAX_CURSOR_POS_Y))
		               / static_cast<std::int32_t>(TILE_SCALE);

		auto fTilePosition = static_cast<sf::Vector2f>(tilePosition);
		auto sizeOffset    = sf::Vector2f(TILE_SCALE, TILE_SCALE);
		if (brushSize % 2 != 0)
		{
			sizeOffset *= static_cast<float>(brushSize - 1) * 0.5F;
		}
		else
		{
			sizeOffset *= static_cast<float>(brushSize) * 0.5F;
		}

		tileSelector.setOutlineThickness(1.0F / static_cast<float>(brushSize) * viewZoomScale);
		tileSelector.setScale(sf::Vector2f(brushSize, brushSize));

		tileSelector.setPosition((fTilePosition * TILE_SCALE) - sizeOffset);

		showFileInfoWindow();
		showTileInfoWindow();
		showToolsWindow();
		showTileSelectorWindow();

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
