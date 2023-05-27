#include "Scene.h"

#include "Components.h"
#include "Entity.h"

namespace SnowEngine
{
	Entity Scene::CreateEntity()
	{
		return Entity{ mRegistry.create(), shared_from_this() };
	}

	Entity Scene::CreateEntity(entityId id)
	{
		if (mRegistry.valid(static_cast<entt::entity>(id)))
			return Entity{ static_cast<entt::entity>(id), shared_from_this() };

		return Entity{ mRegistry.create(static_cast<entt::entity>(id)), shared_from_this() };
	}

	b8 Scene::GetEntity(entityId id, Entity& entity)
	{
		if (mRegistry.valid(static_cast<entt::entity>(id)))
		{
			entity = Entity{ static_cast<entt::entity>(id), shared_from_this() };
			return true;
		}

		entity = Entity{ entt::null, nullptr };
		return false;
	}

	void Scene::ExecuteSystem(const std::function<void(Entity)>& system)
	{
		mRegistry.each([&](const entt::entity id)
		{
			system(Entity{ id, shared_from_this() });
		});
	}
}
