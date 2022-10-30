#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Network
{

	template<class DerivedComponent>
	class SerialisedComponent
	{
	public:
		auto serialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent*>(this)->serialise(data);
		}

		auto deserialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent*>(this)->deserialise(data);
		}
	};

} // namespace Common::Network