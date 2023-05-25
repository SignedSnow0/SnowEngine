#pragma once
#include <string>
#include <glm/glm.hpp>

#include "Graphics/Mesh.h"

namespace SnowEngine
{
	namespace Component
	{
		struct Transform
		{
			glm::vec3 Position{ 0.0f };
			glm::vec3 Rotation{ 0.0f };
			glm::vec3 Scale{ 1.0f };

			glm::mat4 Model();
		};

		struct Tag
		{
			std::string Name;
		};

		struct Mesh
		{
			std::shared_ptr<SnowEngine::Mesh> Model;

			Mesh();
		};
	}
}
