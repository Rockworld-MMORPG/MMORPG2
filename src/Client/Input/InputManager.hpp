#pragma once
#include "Input/InputState.hpp"
#include <Common/Input/ActionType.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <unordered_map>
#include <unordered_set>

namespace Client
{

	class InputManager
	{
	public:
		auto bindAction(sf::Keyboard::Key keyCode, Common::Input::ActionType actionType) -> void;
		auto unbindAction(sf::Keyboard::Key keyCode) -> void;

		auto parseEvents(sf::Event& event) -> void;
		auto update(sf::Time deltaTime) -> void;
		auto getState(Common::Input::ActionType actionType) -> Input::InputState;

		auto getChangedStates() -> std::unordered_set<Common::Input::ActionType>;

	private:
		std::unordered_map<sf::Keyboard::Key, Common::Input::ActionType> m_keyBindings;
		std::unordered_map<Common::Input::ActionType, Input::InputState> m_actionStates;
		std::unordered_set<Common::Input::ActionType> m_changedStates;
	};

} // namespace Client