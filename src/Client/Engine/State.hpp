#pragma once

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

	class State
	{
	public:
		State(Engine& engine) :
		    engine(engine){};
		virtual ~State() = default;

		virtual auto parseMessages(std::vector<Common::Network::Message>& messages) -> void {}
		virtual auto handleEvents(sf::Event& event) -> void{};
		virtual auto update(float deltaTime) -> void{};
		virtual auto render(sf::RenderTarget& renderTarget) -> void{};

	protected:
		Engine& engine;
	};

} // namespace Client