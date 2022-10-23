#include "Common/World/Tile.hpp"
#include "TerrainRenderer.hpp"
#include "TerrainTile.hpp"
#include "TextureManager.hpp"
#include <Common/World/Level.hpp>
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <imgui-SFML.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>

auto assetDir = std::filesystem::path();

const auto MAX_CURSOR_POS_X = TILE_SCALE * Common::World::LEVEL_WIDTH;
const auto MAX_CURSOR_POS_Y = TILE_SCALE * Common::World::LEVEL_HEIGHT;

auto viewZoomScale = float(1.0F);

auto currentlyLoadedLevel = std::filesystem::path();
auto level                = Common::World::Level();

auto textureManager  = TextureManager();
auto tilePaletteMap  = std::unordered_map<std::uint32_t, sf::Sprite>();
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

auto selectedTile = std::uint32_t(0);


auto validCursorPosition() -> bool
{
	return (cursorPosition.x >= 0 && cursorPosition.x < MAX_CURSOR_POS_X && cursorPosition.y >= 0 && cursorPosition.y < MAX_CURSOR_POS_Y);
}

auto loadTile(std::filesystem::path filepath) -> void
{
	spdlog::debug("Attempting to load tile {}", filepath.string());
	if (!std::filesystem::exists(assetDir / filepath))
	{
		spdlog::warn("Tile does not exist", filepath.string());
		return;
	}

	auto reader = std::ifstream(assetDir / filepath);
	auto json   = nlohmann::json::parse(reader);

	auto identifier  = json.at("identifier");
	auto texturePath = std::filesystem::path(assetDir / json.at("texture_path"));

	auto image = sf::Image();
	{
		auto success = image.loadFromFile(texturePath);
		if (!success)
		{
			spdlog::warn("Failed to load tile {}", filepath.string());
			return;
		}
	}

	textureManager.addTexture(identifier, image);

	auto [pair, success]
	    = tilePaletteMap.emplace(identifier, sf::Sprite());

	auto& sprite = pair->second;
	sprite.setTexture(textureManager.getAtlas());
	auto tRect = textureManager.getTextureRect(identifier);
	auto fRect = textureManager.getTextureCoords(identifier);
	spdlog::debug("Given coordinates - Top: {:0.5F}, Left: {:0.5F}, Width: {:0.5F}, Height: {:0.5F}", fRect.top, fRect.left, fRect.width, fRect.height);

	sprite.setTextureRect(tRect);
}

auto saveLevel() -> void
{
	spdlog::debug("Saving level to {}", currentlyLoadedLevel.string());
	auto writer = std::ofstream(assetDir / currentlyLoadedLevel, std::ios::out | std::ios::binary);

	auto data = level.data();
	writer.write(data.data(), data.size());
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

	const auto targetSize = Common::World::LEVEL_WIDTH * Common::World::LEVEL_HEIGHT * sizeof(std::uint32_t);
	if (fileSize != targetSize)
	{
		spdlog::error("File is not the right size {}B/{}B", fileSize, targetSize);
		return;
	}

	auto buffer = std::array<char, targetSize>();
	reader.seekg(std::ios::beg);
	reader.read(buffer.data(), fileSize);
	reader.close();

	level = Common::World::Level(buffer);

	currentlyLoadedLevel = correctedFilepath.relative_path();
	terrainRenderer.clear();
	terrainRenderer.addLevel(level, textureManager);
}

auto showFileInfoWindow() -> void
{
	static std::string filepathInput = currentlyLoadedLevel.relative_path().string();

	ImGui::Begin("File info");
	ImGui::InputText("Filepath", &filepathInput);
	if (ImGui::Button("Save"))
	{
		currentlyLoadedLevel = filepathInput;
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
	ImGui::Text(fmt::format("Selected Tile ID: {}", selectedTile).c_str());
	auto i = std::size_t(0);

	for (auto& [id, sprite] : tilePaletteMap)
	{
		if (i % 4 == 0)
		{
			ImGui::Spacing();
		}
		else
		{
			ImGui::SameLine();
		}
		ImGui::PushID(id);
		if (ImGui::ImageButton(sprite))
		{
			spdlog::debug("Clicked {}", id);
			selectedTile = id;
		}
		ImGui::PopID();
		++i;
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
			showTileSelectorWindow();
			break;
		case Tools::Fill:
			ImGui::Text("Currently selected tool: Fill");
			showTileSelectorWindow();
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
					if (level.getTile(x, y) == initialTile)
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

	auto renderWindow    = sf::RenderWindow();
	auto contextSettings = sf::ContextSettings{0, 0, 0, 3, 0, sf::ContextSettings::Attribute::Default, false};
	renderWindow.create(sf::VideoMode(sf::Vector2u(1280, 1280)), "Level Editor", sf::Style::Default, contextSettings);
	renderWindow.setVerticalSyncEnabled(true);

	auto tilesDir = assetDir / "tiles";
	for (auto file : std::filesystem::recursive_directory_iterator(tilesDir))
	{
		if (file.path().extension() == ".json")
		{
			loadTile(file);
		}
	}

	auto view = sf::View();
	view.setCenter(sf::Vector2f(0.0F, 0.0F));
	view.setSize(sf::Vector2f(TILE_SCALE * 32, TILE_SCALE * 32));

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

	terrainRenderer.clear();
	terrainRenderer.addLevel(level, textureManager);

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
			if (!ImGui::GetIO().WantCaptureMouse)
			{
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
							default:
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
								terrainRenderer.addLevel(level, textureManager);

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
							default:
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

		renderWindow.clear(sf::Color::Black);
		// World view
		renderWindow.setView(view);
		sf::RenderStates states = sf::RenderStates::Default;
		states.texture          = &textureManager.getAtlas();
		terrainRenderer.render(renderWindow, states);

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
