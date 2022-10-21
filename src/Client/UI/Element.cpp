#include "UI/Element.hpp"
#include <entt/entity/registry.hpp>


namespace Client::UI
{

	auto createElement(entt::registry& registry, std::string identifier, Layer layer) -> entt::entity
	{
		auto entity = registry.create();

		registry.emplace<entt::hashed_string>(entity, identifier.c_str(), identifier.size());
		registry.emplace<Layer>(entity, layer);

		return entity;
	}

} // namespace Client::UI