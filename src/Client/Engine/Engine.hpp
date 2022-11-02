#pragma once

#include "Assets/AssetManager.hpp"
#include "Input/InputManager.hpp"
#include "Network/NetworkManager.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>
#include <stack>

namespace Client
{

	class State;

	/**
	 * \class Engine Engine.hpp "Engine/Engine.hpp"
	 * \brief Manages the game state and runs the main game loop
	 */
	class Engine
	{
	public:
		/**
		 * \brief Construct a new Engine object
		 *
		 * \param executableDir The path of the application directory (usually the parent_path of argv[0])
		 */
		Engine(const std::filesystem::path& executableDir);

		/**
		 * \brief Pushes a state onto the engine's state stack
		 *
		 * \param state A unique_ptr containing the state
		 */
		auto pushState(std::unique_ptr<State>&& state) -> void;

		/**
		 * \brief Get the state at the top of the engine's state stack
		 *
		 * \return State& A reference to the state
		 */
		auto getState() -> State&;

		/**
		 * \brief Sets whether or not the engine should pop the state at the top of the stack
		 *
		 */
		auto setShouldPopState(bool shouldPopState = true) -> void;

		/**
		 * \brief Runs the game loop until the engine is set to exit
		 *
		 */
		auto run() -> void;

		/**
		 * \brief Sets whether or not the game should stop running the main loop
		 *
		 */
		auto setShouldExit(bool shouldExit = true) -> void;

		AssetManager assetManager;
		NetworkManager networkManager;
		InputManager inputManager;
		sf::RenderWindow window;

	private:
		/**
		 * \brief Pops a state from the engine's state stack
		 *
		 */
		auto popState() -> void;

		std::stack<std::unique_ptr<State>> m_stateStack;
		bool m_shouldExit     = false;
		bool m_shouldPopState = false;
		sf::Clock m_clock;
	};

} // namespace Client