#pragma once

#include "common/Network/ClientID.hpp"
#include "entt/entt.hpp"

namespace Server
{

	using EntityManager = entt::basic_registry<Common::Network::ClientID_t>;

	extern EntityManager g_entityManager;

} // namespace Server