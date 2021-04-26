#pragma once
#include <entt.hpp>
#include <vulkan/vulkan.h>

#include "pipeline.h"

namespace SnowEngine {
	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity& CreateEntity(const std::string& name = "");
		void DeleteEntity(Entity entity);

		void Draw(uint32_t frame, VkCommandBuffer buffer);
		void AddPipeline(Pipeline* pipeline) { pipelines.push_back(pipeline); }
	private:	
		bool Update(uint32_t frame, float deltaTime);
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
		template<typename T>
		void DrawComponent(Entity entity, std::string label);
		template<typename T>
		void DrawMenuItem(const std::string label);
	private:
		std::vector<Pipeline*> pipelines;
		entt::registry m_registry;
		friend class Entity;
	};


}