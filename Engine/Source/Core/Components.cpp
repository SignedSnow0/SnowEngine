#include "Components.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace SnowEngine::Component
{
	glm::mat4 Transform::Model()
	{
		return glm::translate(glm::mat4{ 1.0f }, Position)
			 * glm::toMat4(glm::quat{ Rotation })
			 * glm::scale(glm::mat4{ 1.0f }, Scale);
	}

	Mesh::Mesh()
	{
		const auto image = Image::Create("D:/Dev/SnowEngine/Engine/Resources/Images/sus.png");

		const std::vector<SnowEngine::Vertex> vertices = {
			{ { -0.5f, -0.5f,  0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f,  0.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f,  0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

			{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } }
		};

		const std::vector<u32> indices = {
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		};

		Model = std::make_shared<SnowEngine::Mesh>(vertices, indices, 2);
		Model->SetAlbedo(image);
	}
}
