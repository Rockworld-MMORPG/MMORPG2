#pragma once

#include <Common/Network/NetworkEntity.hpp>
#include <entt/entity/registry.hpp>
#include <entt/entt.hpp>
#include <functional>
#include <optional>

namespace Server
{

	class EntityManager
	{
	private:
		enum NetworkEntity : Common::Network::ClientID_t;

	public:
		auto create() -> Common::Network::ClientID
		{
			auto entity = m_registry.create();
			return Common::Network::ClientID(static_cast<Common::Network::ClientID_t>(entity));
		}

		auto destroy(const Common::Network::ClientID entity)
		{
			auto ent = static_cast<NetworkEntity>(entity.get());
			m_registry.destroy(ent);
		}

		template<typename Component>
		auto addComponent(const Common::Network::ClientID entity) -> void
		{
			auto ent = static_cast<NetworkEntity>(entity.get());
			if (!m_registry.all_of<Component>(ent))
			{
				m_registry.emplace<Component>(ent);
			}
		}

		template<typename Component>
		auto removeComponent(const Common::Network::ClientID entity) -> void
		{
			auto ent = static_cast<NetworkEntity>(entity.get());
			if (m_registry.all_of<Component>(ent))
			{
				m_registry.erase<Component>(ent);
			}
		}

		template<typename Component>
		auto getComponent(const Common::Network::ClientID entity) -> std::optional<std::reference_wrapper<Component>>
		{
			auto ent = static_cast<NetworkEntity>(entity.get());
			if (m_registry.all_of<Component>(ent))
			{
				return {m_registry.get<Component>(ent)};
			}
			return {};
		}

		template<typename Component>
		auto getComponent(const NetworkEntity entity) -> Component&
		{
			return m_registry.get<Component>(entity);
		}

		template<typename Component>
		auto view()
		{
			return m_registry.view<Component>();
		}

		auto getRegistry() -> entt::basic_registry<NetworkEntity>&
		{
			return m_registry;
		}

	private:
		entt::basic_registry<NetworkEntity> m_registry;
	};

} // namespace Server