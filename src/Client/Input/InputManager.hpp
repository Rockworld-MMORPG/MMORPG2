#pragma once
#include "Input/InputState.hpp"
#include <Common/Input/ActionType.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>
#include <unordered_set>

namespace Client
{

	/**
	 * \class InputManager InputManager.hpp "Input/InputManager.hpp"
	 * \brief Receives player inputs and maps them to actions
	 */
	class InputManager
	{
	public:
		/**
		 * \brief Bind a key code to a specific action
		 *
		 * \param keyCode The key code to bind to the action
		 * \param actionType The type of action the key press should trigger
		 */
		auto bindAction(sf::Keyboard::Key keyCode, Common::Input::ActionType actionType) -> void;

		/**
		 * \brief Unbind a key code from all of its actions
		 *
		 * \param keyCode The key code to unbind
		 */
		auto unbindAction(sf::Keyboard::Key keyCode) -> void;

		/**
		 * \brief Parse an event and perform any actions it may trigger
		 *
		 * \param event The event to parse
		 */
		auto parseEvents(sf::Event& event) -> void;

		/**
		 * \brief Update all input states
		 *
		 * \param deltaTime The time between taken for the last frame to update and render
		 */
		auto update(sf::Time deltaTime) -> void;

		/**
		 * \brief Get the state of an action
		 *
		 * \param actionType The type of action to get the state of
		 * \return Input::InputState An InputState containing the action state data
		 */
		auto getState(Common::Input::ActionType actionType) -> Input::InputState;

		/**
		 * \brief Get the a list of action types that have changed state since the last frame
		 *
		 * \return std::unordered_set<Common::Input::ActionType> A set containing all the ActionTypes that have changed since the last frame
		 */
		auto getChangedStates() -> std::unordered_set<Common::Input::ActionType>;

	private:
		std::unordered_map<sf::Keyboard::Key, Common::Input::ActionType> m_keyBindings;
		std::unordered_map<Common::Input::ActionType, Input::InputState> m_actionStates;
		std::unordered_set<Common::Input::ActionType> m_changedStates;
	};

} // namespace Client