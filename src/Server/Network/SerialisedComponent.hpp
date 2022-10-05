#pragma once

namespace sf
{
	class Packet;
}

namespace Server
{

	template<class DerivedComponent>
	class SerialisedComponent
	{
	public:
		auto serialise(sf::Packet& packet) -> void
		{
			static_cast<DerivedComponent>(this)->serialise(packet);
		}
	};

} // namespace Server