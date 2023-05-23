#pragma once
#include <entt/entt.hpp>

#include "Scene.h"

namespace SnowEngine
{
	class Entity
	{
	public:
		Entity() = default;

		b8 IsValid() const;

		template<typename T, typename... Args>
		decltype(auto) AddComponent(Args&&... args)
		{
			return mScene->mRegistry.emplace<T>(mId, std::forward<Args>(args)...);
		}

		template<typename... Comp>
		decltype(auto) GetComponents() const
		{
			return mScene->mRegistry.get<Comp...>(mId);
		}

		template<typename... Comp>
		decltype(auto) GetComponents()
		{
			return mScene->mRegistry.get<Comp...>(mId);
		}

		template<typename... Comp>
		b8 HasComponents() const
		{
			return mScene->mRegistry.all_of<Comp ...>(mId);
		}

		template<typename... Comp>
		void RemoveComponents() const
		{
			mScene->mRegistry.remove<Comp...>(mId);
		}
 
	private:
		Entity(entt::entity id, std::shared_ptr<Scene> scene);

		entt::entity mId{ entt::null };
		std::shared_ptr<Scene> mScene{ nullptr };

		friend class Scene;
	};
}
