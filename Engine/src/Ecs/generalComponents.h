#pragma once
#include "Graphics/Model.h"

namespace SnowEngine {
	struct Component {
		Component() = default;
		Component(const Component&) = default;

		virtual void ImguiDraw() {}
		virtual void Update() {}
	};

	struct TagComponent : Component {
		std::string Tag;
		
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}

		operator const std::string() { return Tag; }
	};

	struct TransformComponent : Component {
		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 Rotation = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		operator const glm::mat4() { return GetTransform(); }

		void ImGuiDraw();
		glm::mat4 GetTransform();
	};

	struct ModelComponent : Component {
		Model* model = nullptr;

		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;
		ModelComponent(Model* model) : model(model) {}

		operator Model*() { return model; }

		void ImGuiDraw();
	};
}