#pragma once

#include "SFML/System/Time.hpp"
#include <vector>

namespace Common::Network
{
	class Message;
}

namespace sf
{
	class Event;
	class RenderTarget;
} // namespace sf

namespace Client
{
	class Engine;

	/**
	 * \class State State.hpp "Engine/State.hpp"
	 * \brief Base class for any class implementing a game state
	 */
	class State
	{
	public:
		/**
		 * \brief Construct a new State object
		 *
		 * \param engine A reference to the engine the state is contained in
		 */
		State(Engine& engine) :
		    engine(engine){};

		/**
		 * \brief Destroy the State object
		 *
		 */
		virtual ~State() = default;

		/**
		 * \brief Called once per frame to handle any messages the client has received from the server
		 *
		 * \param messages A vector containing all the messages the client has received
		 */
		virtual auto parseMessages(std::vector<Common::Network::Message>& messages) -> void {}

		/**
		 * \brief Called whenever there are events polled from the OS
		 *
		 * \param event The event that has been polled
		 */
		virtual auto handleEvents(sf::Event& event) -> void{};

		/**
		 * \brief Called once per frame to update any game logic
		 *
		 * \param deltaTime How long the previous frame took to update and render
		 */
		virtual auto update(sf::Time deltaTime) -> void{};

		/**
		 * \brief Called once per frame to render any drawables to the render target
		 *
		 * \param renderTarget The target to render to
		 */
		virtual auto render(sf::RenderTarget& renderTarget) -> void{};

	protected:
		Engine& engine;
	};

} // namespace Client