#pragma once

#include <SFML/System/Time.hpp>

namespace Client::Input
{

	struct InputState
	{
		bool isPressed = false;
		sf::Time time  = sf::milliseconds(0);
	};

} // namespace Client::Input