#include "Components.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace SnowEngine
{
	glm::mat4 Transform::Model()
	{
		return glm::translate(glm::mat4{ 1.0f }, Position)
			 * glm::toMat4(glm::quat{ Rotation })
			 * glm::scale(glm::mat4{ 1.0f }, Scale);
	}
}
