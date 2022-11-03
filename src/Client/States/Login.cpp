#include "States/Login.hpp"
#include "Discord/DiscordManager.hpp"
#include "Engine/Engine.hpp"
#include "Network/NetworkManager.hpp"
#include "States/Game.hpp"
#include "UI/UI.hpp"

namespace Client::States
{
	Login::Login(Engine& engine) :
	    State(engine)
	{
		engine.assetManager.loadAsset("levels/test_level.dat", "test_level");
		engine.assetManager.loadAsset("textures/player.png", "player");
		engine.assetManager.loadAsset("OpenSans-Regular.ttf", "font");

		m_uiRenderer.resize(sf::Vector2f(1280.0F, 720.0F));

		const auto& font = engine.assetManager.getAsset("font");
		auto success     = m_font.loadFromMemory(font.data(), font.size());

		UI::createElement(m_registry, "text_username", 0, UI::TextCreateInfo{sf::Vector2f(100.0F, 100.0F), m_font, "Username", 48});
		UI::createElement(m_registry, "text_password", 0, UI::TextCreateInfo{sf::Vector2f(100.0F, 200.0F), m_font, "Password", 48});

		m_usernameTextEntity = UI::createElement(m_registry, "textinput_username", 0, UI::TextInputCreateInfo{sf::Vector2f(400.0F, 100.0F), 24, UI::TextInputCreateInfo::NO_MASKING, 48, m_font});
		m_passwordTextEntity = UI::createElement(m_registry, "textinput_password", 0, UI::TextInputCreateInfo{sf::Vector2f(400.0F, 200.0F), 24, '*', 48, m_font});

		UI::createElement(m_registry, "button_login", 0, UI::RectButtonCreateInfo{sf::Vector2f(470.0F, 320.0F), sf::Vector2f(160.0F, 60.0F), "Login", m_font, {}, [&](sf::Mouse::Button b) {
			                                                                          if (!engine.networkManager.isConnected())
			                                                                          {
				                                                                          engine.networkManager.connect();
			                                                                          }

			                                                                          if (engine.networkManager.isConnected())
			                                                                          {
				                                                                          auto data = Common::Network::MessageData();
				                                                                          data << m_registry.get<UI::TextInputData>(m_usernameTextEntity).input << m_registry.get<UI::TextInputData>(m_passwordTextEntity).input;
				                                                                          engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Client_Authenticate, data);
			                                                                          }
		                                                                          }});
		UI::createElement(m_registry, "button_quit", 0, UI::RectButtonCreateInfo{sf::Vector2f(670.0F, 320.0F), sf::Vector2f(160.0F, 60.0F), "Quit", m_font, {}, [&](sf::Mouse::Button b) {
			                                                                         engine.setShouldExit(true);
		                                                                         }});

		engine.inputManager.bindAction(sf::Keyboard::W, Common::Input::ActionType::MoveForward);
		engine.inputManager.bindAction(sf::Keyboard::A, Common::Input::ActionType::StrafeLeft);
		engine.inputManager.bindAction(sf::Keyboard::S, Common::Input::ActionType::MoveBackward);
		engine.inputManager.bindAction(sf::Keyboard::D, Common::Input::ActionType::StrafeRight);
	}

	Login::~Login()
	{
		engine.networkManager.disconnect();
		engine.window.setView(engine.window.getDefaultView());
	}

	auto Login::parseMessages(std::vector<Common::Network::Message>& messages) -> void
	{
		for (auto& message : messages)
		{
			if (message.header.protocol != Common::Network::Protocol::TCP)
			{
				// Login uses TCP exclusively
				break;
			}

			using MT = Common::Network::MessageType;
			switch (message.header.type)
			{
				case MT::Server_Authenticate:
				{
					auto errorCode = std::uint8_t(-1);

					message.data >> errorCode;
					switch (errorCode)
					{
						case 0: // Success
							engine.pushState(std::make_unique<States::Game>(engine));
							break;
						case 1: // Incorrect username
							spdlog::info("Incorrect username");
							break;
						case 2: // Incorrect password
							spdlog::info("Incorrect password");
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

	auto Login::handleEvents(sf::Event& event) -> void
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
			default:
				break;
		}
	}

	auto Login::update(const sf::Time deltaTime) -> void
	{
		UI::update(m_registry, deltaTime);
	}

	auto Login::render(sf::RenderTarget& renderTarget) -> void
	{
		m_uiRenderer.render(m_registry, renderTarget);
	}

	auto Login::onEnter() -> void
	{
		DiscordManager::get().setStatus("In the Main Menu");
	}

} // namespace Client::States