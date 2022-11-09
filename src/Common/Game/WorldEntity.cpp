#include "Common/Game/WorldEntity.hpp"
#include "Common/Game/WorldEntityGCD.hpp"
#include <SFML/System/Time.hpp>
#include <entt/entity/registry.hpp>

namespace Common::Game
{

	auto createWorldEntity(entt::registry& registry, const WorldEntityData& data) -> entt::entity
	{
		auto entity = registry.create();
		return createWorldEntity(registry, entity, data);
	}

	auto createWorldEntity(entt::registry& registry, entt::entity entity, const WorldEntityData& data) -> entt::entity
	{
		registry.emplace_or_replace<WorldEntityPosition>(entity, data.position);
		registry.emplace_or_replace<WorldEntityType>(entity, data.type);
		registry.emplace_or_replace<WorldEntityName>(entity, data.name);
		registry.emplace_or_replace<WorldEntityStats>(entity, data.stats);
		registry.emplace_or_replace<WorldEntityCollider>(entity, data.collider);
		registry.emplace_or_replace<WorldEntityGCD>(entity, data.gcd);

		return entity;
	}

	auto serialiseWorldEntity(entt::registry& registry, entt::entity entity, Network::MessageData& messageData) -> void
	{
		auto& worldPositionComponent    = registry.get<WorldEntityPosition>(entity);
		auto& worldEntityTypeComponent  = registry.get<WorldEntityType>(entity);
		auto& worldEntityNameComponent  = registry.get<WorldEntityName>(entity);
		auto& worldEntityStatsComponent = registry.get<WorldEntityStats>(entity);
		auto& worldEntityGcdComponent   = registry.get<WorldEntityGCD>(entity);

		worldPositionComponent.serialise(messageData);
		worldEntityTypeComponent.serialise(messageData);
		worldEntityNameComponent.serialise(messageData);
		worldEntityStatsComponent.serialise(messageData);
		worldEntityGcdComponent.serialise(messageData);
	}

	auto deserialiseWorldEntity(Network::MessageData& messageData) -> WorldEntityData
	{
		auto wData = WorldEntityData();

		wData.position.deserialise(messageData);
		wData.type.deserialise(messageData);
		wData.name.deserialise(messageData);
		wData.stats.deserialise(messageData);
		wData.gcd.deserialise(messageData);

		return wData;
	}

} // namespace Common::Game