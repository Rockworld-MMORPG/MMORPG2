#include "Game/Game.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/TextureAtlas.hpp"
#include "Network/NetworkManager.hpp"
#include "UI/UI.hpp"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Client::Game
{
	Game::Game(Engine& engine) :
	    State(engine),
	    m_textureAtlas(sf::Vector2u(32, 32), sf::Vector2u(32, 32))
	{
		engine.assetManager.loadAsset("levels/test_level.dat", "test_level");
		engine.assetManager.loadAsset("textures/player.png", "player");
		engine.assetManager.loadAsset("OpenSans-Regular.ttf", "font");

#define LOAD_TILE_TEXTURE(NAME) engine.assetManager.loadAsset("textures/" #NAME ".png", #NAME)
#define LOAD_TILE_DATA(NAME) engine.assetManager.loadAsset("tiles/" #NAME ".json", "tile_data_" #NAME)

		// TODO - Create some sort of manifest format to load all these
		LOAD_TILE_TEXTURE(tile_grass_centre);
		LOAD_TILE_TEXTURE(tile_stone_centre);
		LOAD_TILE_TEXTURE(tile_water_centre);
		LOAD_TILE_TEXTURE(tile_grass_stone_east);
		LOAD_TILE_TEXTURE(tile_grass_stone_west);
		LOAD_TILE_TEXTURE(tile_grass_stone_north);
		LOAD_TILE_TEXTURE(tile_grass_stone_south);
		LOAD_TILE_TEXTURE(tile_grass_water_east);
		LOAD_TILE_TEXTURE(tile_grass_water_west);
		LOAD_TILE_TEXTURE(tile_grass_water_north);
		LOAD_TILE_TEXTURE(tile_grass_water_south);

		LOAD_TILE_DATA(grass_centre);
		LOAD_TILE_DATA(stone_centre);
		LOAD_TILE_DATA(water_centre);
		LOAD_TILE_DATA(grass_stone_east);
		LOAD_TILE_DATA(grass_stone_west);
		LOAD_TILE_DATA(grass_stone_north);
		LOAD_TILE_DATA(grass_stone_south);
		LOAD_TILE_DATA(grass_water_east);
		LOAD_TILE_DATA(grass_water_west);
		LOAD_TILE_DATA(grass_water_north);
		LOAD_TILE_DATA(grass_water_south);

#undef LOAD_TILE_TEXTURE
#undef LOAD_TILE_DATA

		loadTile(engine.assetManager.getAsset("tile_data_grass_centre"));
		loadTile(engine.assetManager.getAsset("tile_data_stone_centre"));
		loadTile(engine.assetManager.getAsset("tile_data_water_centre"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_stone_east"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_stone_west"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_stone_north"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_stone_south"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_water_east"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_water_west"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_water_north"));
		loadTile(engine.assetManager.getAsset("tile_data_grass_water_south"));

		m_camera.setCenter(sf::Vector2f(0.0F, 0.0F));
		m_camera.setSize(sf::Vector2f(1280.0F, 720.0F));

		engine.window.setView(m_camera);
		m_uiRenderer.resize(sf::Vector2f(1280.0F, 720.0F));

		const auto& player = engine.assetManager.getAsset("player");
		auto success       = m_playerTexture.loadFromMemory(player.data(), player.size());

		const auto& font = engine.assetManager.getAsset("font");
		success          = m_font.loadFromMemory(font.data(), font.size());

		UI::createElement(m_registry, "image_portrait", 0, UI::ImageCreateInfo{sf::Vector2f(10.0F, 10.0F), sf::Vector2f(16.0F, 16.0F), m_playerTexture});
		UI::createElement(m_registry, "text_health", 0, UI::TextCreateInfo{sf::Vector2f(100.0F, 10.0F), m_font, "Health [ 100 / 100 ]", 20});
		UI::createElement(m_registry, "text_power", 0, UI::TextCreateInfo{sf::Vector2f(101.0F, 35.0F), m_font, "Power [ 100 / 100 ]", 20});
		UI::createElement(m_registry, "button_connect", 0, UI::ButtonCreateInfo{sf::Vector2f(10.0F, 670.0F), sf::Vector2f(100.0F, 40.0F), "Connect", m_font, {}, [&](sf::Mouse::Button b) {
			                                                                        if (engine.networkManager.isConnected()) { return; }
			                                                                        engine.networkManager.connect();
			                                                                        if (engine.networkManager.isConnected())
			                                                                        {
				                                                                        auto data = Common::Network::MessageData();
				                                                                        engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Client_Spawn, data);

				                                                                        // TODO - change this to the actual tile identifier
				                                                                        data = Common::Network::MessageData();
				                                                                        data << std::uint32_t(0);
				                                                                        engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Client_GetWorldState, data);
			                                                                        }
		                                                                        }});
		UI::createElement(m_registry, "button_disconnect", 0, UI::ButtonCreateInfo{sf::Vector2f(120.0F, 670.0F), sf::Vector2f(100.0F, 40.0F), "Disconnect", m_font, {}, [&](sf::Mouse::Button b) {
			                                                                           if (!engine.networkManager.isConnected()) { return; }
			                                                                           engine.networkManager.disconnect();
			                                                                           for (const auto entity : m_registry.view<entt::entity>())
			                                                                           {
				                                                                           m_registry.destroy(entity);
			                                                                           }
		                                                                           }});

		UI::createElement(m_registry, "test_text_input", 0, UI::TextInputCreateInfo{sf::Vector2f(10.0F, 640.0F), sf::Vector2f(250.0F, 20.0F), 16, m_font});


		engine.inputManager.bindAction(sf::Keyboard::W, Common::Input::ActionType::MoveForward);
		engine.inputManager.bindAction(sf::Keyboard::A, Common::Input::ActionType::StrafeLeft);
		engine.inputManager.bindAction(sf::Keyboard::S, Common::Input::ActionType::MoveBackward);
		engine.inputManager.bindAction(sf::Keyboard::D, Common::Input::ActionType::StrafeRight);

		const auto& testLevel = engine.assetManager.getAsset("test_level");
		m_level               = Common::World::Level(testLevel);

		m_terrainRenderer.addLevel(entt::hashed_string("test_level").value(), m_level, m_textureAtlas);
	}

	Game::~Game()
	{
		engine.networkManager.disconnect();
		engine.window.setView(engine.window.getDefaultView());
	}

	auto Game::parseTCP(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::Server_Disconnect:
				engine.networkManager.disconnect();
				break;
			default:
				break;
		}
	}

	auto Game::parseUDP(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::Server_CreateEntity:
			{
				auto serverEntityID           = entt::entity(entt::null);
				auto worldPositionComponent   = Common::Game::WorldPosition();
				auto worldEntityTypeComponent = Common::Game::WorldEntityType();

				message.data >> serverEntityID;
				worldEntityTypeComponent.deserialise(message.data);
				worldPositionComponent.deserialise(message.data);

				auto entity  = m_registry.create();
				auto& sprite = m_registry.emplace<sf::Sprite>(entity);
				sprite.setTexture(m_playerTexture);
				sprite.setPosition(worldPositionComponent.position);

				auto& serverID = m_registry.emplace<entt::entity>(entity);
				serverID       = serverEntityID;

				auto& inputState = m_registry.emplace<Common::Input::InputState>(entity);
			}
			break;
			case Common::Network::MessageType::Server_InputState:
			{
				auto serverEntity = entt::entity();
				message.data >> serverEntity;
				for (const auto entity : m_registry.view<Common::Input::InputState, entt::entity>())
				{
					if (m_registry.get<entt::entity>(entity) != serverEntity)
					{
						continue;
					}
					auto& inputState = m_registry.get<Common::Input::InputState>(entity);
					message.data >> inputState.forwards >> inputState.backwards >> inputState.left >> inputState.right;
					break;
				}
			}
			break;
			case Common::Network::MessageType::Server_DestroyEntity:
			{
				auto serverEntity = entt::entity();
				auto localEntity  = entt::entity(entt::null);
				message.data >> serverEntity;
				for (const auto entity : m_registry.view<entt::entity>())
				{
					if (m_registry.get<entt::entity>(entity) == serverEntity)
					{
						localEntity = entity;
						break;
					}
				}
				if (localEntity != entt::null)
				{
					m_registry.destroy(localEntity);
				}
			}
			break;
			case Common::Network::MessageType::Server_WorldState:
			{
				auto entityCount = std::uint32_t(0);
				message.data >> entityCount;

				for (auto i = 0; i < entityCount; ++i)
				{
					auto serverEntityID           = entt::entity(entt::null);
					auto worldPositionComponent   = Common::Game::WorldPosition();
					auto worldEntityTypeComponent = Common::Game::WorldEntityType();

					message.data >> serverEntityID;
					worldEntityTypeComponent.deserialise(message.data);
					worldPositionComponent.deserialise(message.data);

					auto clientEntityIterator = std::find_if(m_registry.view<entt::entity>().begin(), m_registry.view<entt::entity>().end(), [&](const entt::entity entity) {
						return m_registry.get<entt::entity>(entity) == serverEntityID;
					});
					if (clientEntityIterator == m_registry.view<entt::entity>().end())
					{
						auto newEntity = m_registry.create();
						auto& sprite   = m_registry.emplace<sf::Sprite>(newEntity);
						sprite.setTexture(m_playerTexture);
						sprite.setPosition(worldPositionComponent.position);

						auto& serverID = m_registry.emplace<entt::entity>(newEntity);
						serverID       = serverEntityID;

						auto& inputState = m_registry.emplace<Common::Input::InputState>(newEntity);
					}
					else
					{
						auto existingEntity = *clientEntityIterator;
						auto& sprite        = m_registry.get<sf::Sprite>(existingEntity);
						sprite.setPosition(worldPositionComponent.position);
					}
				}
			}
			break;
			default:
				break;
		}
	}

	auto Game::parseMessages(std::vector<Common::Network::Message>& messages) -> void
	{
		for (auto& message : messages)
		{
			switch (message.header.protocol)
			{
				case Common::Network::Protocol::TCP:
					parseTCP(message);
					break;
				case Common::Network::Protocol::UDP:
					parseUDP(message);
					break;
			}
		}
	}

	auto sendAction(const Common::Input::ActionType actionType, const Common::Input::Action::State actionState, NetworkManager& networkManager) -> void
	{
		auto action  = Common::Input::Action();
		action.type  = actionType;
		action.state = actionState;

		auto data = Common::Network::MessageData();
		data << action;

		networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Client_Action, data);
	}

	auto Game::handleEvents(sf::Event& event) -> void
	{
		if (UI::handleEvents(m_registry, event))
		{
			return;
		}

		engine.inputManager.parseEvents(event);

		switch (event.type)
		{
			case sf::Event::Closed:
				engine.setShouldExit(true);
				break;
			case sf::Event::Resized:
			{
				auto windowSize = static_cast<sf::Vector2f>(engine.window.getSize());
				m_camera.setSize(windowSize);
			}
			break;
			default:
				break;
		}
	}

	auto Game::update(const sf::Time deltaTime) -> void
	{
		UI::update(m_registry, deltaTime);

		auto changedStates = engine.inputManager.getChangedStates();
		for (const auto actionType : changedStates)
		{
			auto pressed = engine.inputManager.getState(actionType).isPressed;

			sendAction(actionType, pressed ? Common::Input::Action::State::Begin : Common::Input::Action::State::End, engine.networkManager);
		}

		for (const auto entity : m_registry.view<sf::Sprite, Common::Input::InputState>())
		{
			auto& input    = m_registry.get<Common::Input::InputState>(entity);
			auto& sprite   = m_registry.get<sf::Sprite>(entity);
			auto& serverID = m_registry.get<entt::entity>(entity);

			if (serverID == engine.networkManager.getClientID())
			{
				m_camera.setCenter(sprite.getPosition() + sprite.getGlobalBounds().getSize() * 0.5F);
			}

			auto deltaPosition = sf::Vector2f(0.0F, 0.0F);
			if (input.forwards)
			{
				deltaPosition.y -= 200.0F;
			}
			if (input.backwards)
			{
				deltaPosition.y += 200.0F;
			}
			if (input.left)
			{
				deltaPosition.x -= 200.0F;
			}
			if (input.right)
			{
				deltaPosition.x += 200.0F;
			}

			deltaPosition *= deltaTime.asSeconds();
			sprite.move(deltaPosition);
		}
	}

	auto Game::render(sf::RenderTarget& renderTarget) -> void
	{
		renderTarget.setView(m_camera);
		m_terrainRenderer.render(renderTarget, m_textureAtlas.getTexture());


		for (const auto entity : m_registry.view<sf::Sprite>(entt::exclude<UI::ElementData>))
		{
			renderTarget.draw(m_registry.get<sf::Sprite>(entity));
		}

		m_uiRenderer.render(m_registry, renderTarget);
	}

	auto Game::loadTile(const std::vector<char>& data) -> void
	{
		if (data.empty())
		{
			spdlog::debug("Tried to load a tile but the data is empty");
			return;
		}
		auto json = nlohmann::json::parse(data);

		auto identifier  = json.at("identifier");
		auto textureData = engine.assetManager.getAsset(json.at("texture_path"));

		auto image = sf::Image();
		{
			auto success = image.loadFromMemory(textureData.data(), textureData.size());
			if (!success)
			{
				spdlog::warn("Failed to load tile {}", json.at("texture_path"));
				return;
			}
		}

		m_textureAtlas.addTexture(identifier, image);
		spdlog::debug("Loaded tile {}", static_cast<std::uint32_t>(identifier));
	}

} // namespace Client::Game