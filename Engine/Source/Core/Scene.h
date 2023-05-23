#pragma once
#include <functional>
#include <entt/entt.hpp>

#include "Core/Types.h"

namespace SnowEngine
{
	class Entity;
	using entityId = u32;

	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:
		Entity CreateEntity();
		Entity CreateEntity(entityId id);
		b8 GetEntity(entityId id, Entity& entity);

		void ExecuteSystem(const std::function<void(Entity)>& system);

	private:
		entt::registry mRegistry;

		friend class Entity;
	};
}