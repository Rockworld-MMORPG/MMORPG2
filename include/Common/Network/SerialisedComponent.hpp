#pragma once

namespace Common::Network
{

	class MessageData;

	template<class DerivedComponent>
	class SerialisedComponent
	{
	public:
		auto serialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent>(this)->serialise(data);
		}

		auto deserialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent>(this)->deserialise(data);
		}
	};

} // namespace Common::Network