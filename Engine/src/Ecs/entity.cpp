#include "entity.h"

namespace SnowEngine {
	Entity::Entity(entt::entity handle, Scene* scene) : entityHandle(handle), scene(scene) {

	}

	Entity::~Entity() {

	}
}