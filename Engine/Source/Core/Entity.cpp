#include "Entity.h"

namespace SnowEngine
{
	b8 Entity::IsValid() const { return mScene != nullptr && mScene->mRegistry.valid(mId); }

	Entity::Entity(const entt::entity id, std::shared_ptr<Scene> scene)
		: mId{ id }, mScene{ std::move(scene) } {}
}
