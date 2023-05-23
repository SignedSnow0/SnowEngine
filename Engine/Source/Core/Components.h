#pragma once
#include <string>
#include <glm/glm.hpp>

namespace SnowEngine
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
}