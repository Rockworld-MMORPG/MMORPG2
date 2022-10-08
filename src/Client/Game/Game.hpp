#pragma once

#include "Common/Network/ClientID.hpp"
#include "Common/Network/Message.hpp"
#include "Engine/State.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/Texture.hpp"
#include <unordered_map>

namespace Client::Game
{

	class Game : public State
	{
	public:
		Game(Engine& engine);
		virtual ~Game();

		auto parseMessages(std::vector<Common::Network::Message>& messages) -> void override;
		auto handleEvents(sf::Event& event) -> void override;
		auto update(float deltaTime) -> void override;
		auto render(sf::RenderTarget& renderTarget) -> void override;

	private:
		auto parseTCP(Common::Network::Message& message) -> void;
		auto parseUDP(Common::Network::Message& message) -> void;

		std::unordered_map<Common::Network::ClientID, sf::Sprite, Common::Network::ClientIDHash> m_sprites;
		sf::Texture m_playerTexture;
	};

} // namespace Client::Game