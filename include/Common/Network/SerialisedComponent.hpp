#pragma once

#include "Common/Export.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Network
{

	/**
	 * \class SerialisedComponent SerialisedComponent.hpp <Common/Network/SerialisedComponent.hpp>
	 * \brief A base class for any component which should be serialisable
	 *
	 * \tparam DerivedComponent The type of a class deriving this, in order to support static polymorphism
	 */
	template<class DerivedComponent>
	class SerialisedComponent
	{
	public:
		/**
		 * \brief Serialise the component into a message data struct
		 *
		 * \param data The MessageData struct to serialise in to
		 */
		auto serialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent*>(this)->serialise(data);
		}

		/**
		 * \brief Deserialise the component from a message data struct
		 *
		 * \param data The MessageData struct to deserialise from
		 */
		auto deserialise(MessageData& data) -> void
		{
			static_cast<DerivedComponent*>(this)->deserialise(data);
		}
	};

} // namespace Common::Network