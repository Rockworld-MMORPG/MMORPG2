#pragma once

#include "Engine/State.hpp"
#include "UI/UI.hpp"
#include "World/TerrainRenderer.hpp"
#include <Common/Network/Message.hpp>
#include <Common/World/Level.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/View.hpp>
#include <entt/entity/registry.hpp>

namespace Client::States
{

	/**
	 * \class Login Login.hpp "States/Login.hpp"
	 * \brief State implementing the login screen / main menu
	 */
	class Login final : public State
	{
	public:
		/**
		 * \brief Construct a new Login state object
		 *
		 * \param engine A reference to the engine the state is contained in
		 */
		Login(Engine& engine);

		/**
		 * \brief Destroy the Login state object
		 *
		 */
		~Login() override;

		// Login is not copyable or moveable
		Login(Login&)           = delete;
		Login(Login&&)          = delete;
		auto operator=(Login&)  = delete;
		auto operator=(Login&&) = delete;

		/**
		 * \brief Called once per frame to handle any messages the client has received from the server
		 *
		 * \param messages A vector containing all the messages the client has received
		 */
		auto parseMessages(std::vector<Common::Network::Message>& messages) -> void override;

		/**
		 * \brief Called whenever there are events polled from the OS
		 *
		 * \param event The event that has been polled
		 */
		auto handleEvents(sf::Event& event) -> void override;

		/**
		 * \brief Called once per frame to update any game logic
		 *
		 * \param deltaTime How long the previous frame took to update and render
		 */
		auto update(sf::Time deltaTime) -> void override;

		/**
		 * \brief Called once per frame to render any drawables to the render target
		 *
		 * \param renderTarget The target to render to
		 */
		auto render(sf::RenderTarget& renderTarget) -> void override;

		/**
		 * \brief Called when the state becomes the state at the top of the stack
		 *
		 */
		auto onEnter() -> void override;

	private:
		UI::UIRenderer m_uiRenderer;
		entt::entity m_usernameTextEntity;
		entt::entity m_passwordTextEntity;

		entt::registry m_registry;
		sf::Font m_font;
	};

} // namespace Client::States