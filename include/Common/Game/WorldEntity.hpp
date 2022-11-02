#pragma once

#include "Common/Game/WorldEntityName.hpp"
#include "Common/Game/WorldEntityPosition.hpp"
#include "Common/Game/WorldEntityStats.hpp"
#include "Common/Game/WorldEntityType.hpp"
#include "Common/Network/MessageData.hpp"

namespace Common::Game
{
	/**
	 * \struct WorldEntityData WorldEntity.hpp <Common/Game/WorldEntity.hpp>
	 * \brief Data possessed by all entities that exist in the world
	 */
	struct WorldEntityData
	{
		WorldEntityName name;
		WorldEntityType type;
		WorldEntityPosition position;
		WorldEntityStats stats;
	};

	COMMON_API auto createWorldEntity(entt::registry& registry, const WorldEntityData& data) -> entt::entity;
	COMMON_API auto createWorldEntity(entt::registry& registry, entt::entity entity, const WorldEntityData& data) -> entt::entity;

	COMMON_API auto serialiseWorldEntity(entt::registry& registry, entt::entity entity, Network::MessageData& messageData) -> void;
	COMMON_API auto deserialiseWorldEntity(Network::MessageData& messageData) -> WorldEntityData;

} // namespace Common::Game