#include "Input/InputManager.hpp"
#include "Common/Input/ActionType.hpp"
#include "SFML/System/Time.hpp"

namespace Client
{

	auto InputManager::bindAction(sf::Keyboard::Key keyCode, Common::Input::ActionType actionType) -> void
	{
		m_keyBindings.emplace(keyCode, actionType);
		m_actionStates.emplace(actionType, Input::InputState());
	}

	auto InputManager::unbindAction(sf::Keyboard::Key keyCode) -> void
	{
		m_keyBindings.erase(keyCode);
	}

	auto InputManager::parseEvents(sf::Event& event) -> void
	{
		switch (event.type)
		{
			case sf::Event::KeyPressed:
			{
				auto iterator = m_keyBindings.find(event.key.code);
				if (iterator != m_keyBindings.end())
				{
					auto action                         = iterator->second;
					m_actionStates.at(action).isPressed = true;
					m_actionStates.at(action).time      = sf::milliseconds(0);
					m_changedStates.emplace(action);
				}
			}
			break;
			case sf::Event::KeyReleased:
			{
				auto iterator = m_keyBindings.find(event.key.code);
				if (iterator != m_keyBindings.end())
				{
					auto action                         = iterator->second;
					m_actionStates.at(action).isPressed = false;
					m_changedStates.emplace(action);
				}
			}
			break;
			default:
				break;
		}
	}

	auto InputManager::update(sf::Time deltaTime) -> void
	{
		for (auto& [action, state] : m_actionStates)
		{
			if (state.isPressed)
			{
				state.time += deltaTime;
			}
		}
	}

	auto InputManager::getState(Common::Input::ActionType actionType) -> Input::InputState
	{
		if (m_actionStates.contains(actionType))
		{
			return m_actionStates.at(actionType);
		}

		return {};
	}

	auto InputManager::getChangedStates() -> std::unordered_set<Common::Input::ActionType>
	{
		auto temp = m_changedStates;
		m_changedStates.clear();
		return temp;
	}

} // namespace Client