#pragma once
#include "Scene.h"

namespace SnowEngine {
	class Entity {
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		~Entity();

		template<typename T, typename ... Args>
		T& AddComponent(Args&& ... args) { return scene->m_registry.emplace<T>(entityHandle, std::forward<Args>(args) ...); } //forward non processa gli argomenti ma li passa solamente
		template<typename T>
		T& GetComponent() { return scene->m_registry.get<T>(entityHandle); }
		template<typename T>
		inline bool HasComponent() { return scene->m_registry.has<T>(entityHandle); }
		template<typename T>
		void RemoveComponent() { scene->m_registry.remove<T>(entityHandle); }

		operator bool() const { return entityHandle != entt::null; }
		operator entt::entity() { return entityHandle; }
		operator uint32_t() const { return (uint32_t)entityHandle; }
		bool operator ==(const Entity& other) const { return entityHandle == other.entityHandle && scene == other.scene; }
	private:
		entt::entity entityHandle{ entt::null };
		Scene* scene = nullptr;
	};
}