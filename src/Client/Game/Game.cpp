#include "Game/Game.hpp"
#include "Common/Input/Action.hpp"
#include "Common/Input/ActionType.hpp"
#include "Common/Network/MessageData.hpp"
#include "Common/Network/MessageType.hpp"
#include "Common/Network/Protocol.hpp"
#include "Engine/Engine.hpp"
#include "Network/NetworkManager.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/System/Time.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/Window/Event.hpp"
#include "SFML/Window/Keyboard.hpp"

namespace Client::Game
{

	Game::Game(Engine& engine) :
	    State(engine)
	{
		m_playerTexture.loadFromFile(engine.assetDirectory / "player.png");
		engine.networkManager.connect();
	}

	Game::~Game() = default;

	auto Game::parseTCP(Common::Network::Message& message) -> void
	{
		switch (message.header.type)
		{
			case Common::Network::MessageType::Disconnect:
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
			case Common::Network::MessageType::Position:
			{
				std::uint32_t size = 0;
				message.data >> size;

				for (auto i = 0; i < size; ++i)
				{
					Common::Network::ClientID_t entity = -1;
					sf::Vector2f remotePosition{0.0F, 0.0F};

					message.data >> entity >> remotePosition.x >> remotePosition.y;

					auto iterator = m_sprites.find(Common::Network::ClientID(entity));
					if (iterator == m_sprites.end())
					{
						auto [pair, success] = m_sprites.emplace(entity, sf::Sprite{});
						pair->second.setTexture(m_playerTexture);
						iterator = pair;
					}

					iterator->second.setPosition(remotePosition);
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

				auto iterator = m_sprites.find(Common::Network::ClientID(clientID));
				if (iterator == m_sprites.end())
				{
					auto [pair, success] = m_sprites.emplace(clientID, sf::Sprite{});
					pair->second.setTexture(m_playerTexture);
				}
			}
			break;
			case Common::Network::MessageType::DestroyEntity:
			{
				Common::Network::ClientID_t clientID = -1;
				message.data >> clientID;
				m_sprites.erase(Common::Network::ClientID(clientID));
			}
			break;
			default:
				// Do nothing
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

		networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::Action, data);
	}

	auto Game::handleEvents(sf::Event& event) -> void
	{
		switch (event.type)
		{
			case sf::Event::Closed:
				engine.setShouldExit(true);
				break;
			case sf::Event::KeyPressed:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Key::W:
						sendAction(Common::Input::ActionType::MoveForward, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::A:
						sendAction(Common::Input::ActionType::StrafeLeft, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::S:
						sendAction(Common::Input::ActionType::MoveBackward, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					case sf::Keyboard::Key::D:
						sendAction(Common::Input::ActionType::StrafeRight, Common::Input::Action::State::Begin, engine.networkManager);
						break;
					default:
						break;
				}
			}
			break;
			case sf::Event::KeyReleased:
			{
				switch (event.key.code)
				{
					case sf::Keyboard::Key::W:
						sendAction(Common::Input::ActionType::MoveForward, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::A:
						sendAction(Common::Input::ActionType::StrafeLeft, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::S:
						sendAction(Common::Input::ActionType::MoveBackward, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::D:
						sendAction(Common::Input::ActionType::StrafeRight, Common::Input::Action::State::End, engine.networkManager);
						break;
					case sf::Keyboard::Key::P:
					{
						auto data = Common::Network::MessageData();
						engine.networkManager.pushMessage(Common::Network::Protocol::TCP, Common::Network::MessageType::Terminate, data);
					}
					break;
					case sf::Keyboard::Key::C:
					{
						auto data = Common::Network::MessageData();
						engine.networkManager.pushMessage(Common::Network::Protocol::UDP, Common::Network::MessageType::CreateEntity, data);
					}
					default:
						break;
				}
			}
			break;
			default:
				break;
		}
	}

	auto Game::update(const sf::Time deltaTime) -> void
	{
	}

	auto Game::render(sf::RenderTarget& renderTarget) -> void
	{
		for (auto& [id, sprite] : m_sprites)
		{
			renderTarget.draw(sprite);
		}
	}

} // namespace Client::Game