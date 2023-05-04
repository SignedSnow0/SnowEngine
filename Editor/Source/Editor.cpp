#include "Editor.h"
#include <SnowEngine.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

int main()
{
	SnowEngine::GraphicsCore::Init();
	{
		const auto window = SnowEngine::Window::Create("SnowEngine", 2560, 1440);
		const auto surface = SnowEngine::Surface::Create(window);
		const auto renderPass = SnowEngine::RenderPass::Create(surface);
		const auto shader = SnowEngine::Shader::Create(
			{
				{ "C:/Dev/SnowEngine/Engine/Resources/Shaders/default.vert", SnowEngine::ShaderType::Vertex },
				{ "C:/Dev/SnowEngine/Engine/Resources/Shaders/default.frag", SnowEngine::ShaderType::Fragment },
			});

		const auto image = SnowEngine::Image::Create("C:/Dev/SnowEngine/Engine/Resources/Images/sus.png");

		const auto pipeline = SnowEngine::Pipeline::Create(shader, renderPass, 2560, 1440);

		const std::vector<SnowEngine::Vertex> vertices =
		{
			{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } },
			{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
			{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } }
		};

		const std::vector<u32> indices =
		{
			3, 2, 1,
			1, 0, 3
		};

		const auto vertexBuffer = SnowEngine::VertexBuffer::Create(vertices.data(), vertices.size());
		const auto indexBuffer = SnowEngine::IndexBuffer::Create(indices.data(), indices.size());

		const auto cameraDescriptorSet = SnowEngine::DescriptorSet::Create(shader, 0, 2);
		const auto modelDescriptorSet = SnowEngine::DescriptorSet::Create(shader, 1, 2);

		struct Camera
		{
			glm::mat4 View;
			glm::mat4 Projection;
		} camera{};

		struct Transform
		{
			glm::mat4 Model;
		} transform{};

		modelDescriptorSet->SetImage("albedo", image.get());//TODO: descriptor set should take image when using it

		static auto startTime = std::chrono::high_resolution_clock::now();
		while (!window->Closing())
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

			camera.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			camera.Projection = glm::perspective(glm::radians(45.0f), 2560 / static_cast<float>(1440), 0.1f, 10.0f);
			camera.Projection[1][1] *= -1;

			transform.Model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

			surface->Begin();
			renderPass->Begin();

			cameraDescriptorSet->SetUniform("Camera", &camera, surface->GetCurrentFrame());
			modelDescriptorSet->SetUniform("Transform", &transform, surface->GetCurrentFrame());

			pipeline->Bind();
			pipeline->BindDescriptorSet(cameraDescriptorSet.get(), surface->GetCurrentFrame());
			pipeline->BindDescriptorSet(modelDescriptorSet.get(), surface->GetCurrentFrame());

			vertexBuffer->Bind();
			indexBuffer->Bind();
			indexBuffer->Draw();

			renderPass->End();
			surface->End();

			SnowEngine::Window::Update();
		}
	}
	SnowEngine::GraphicsCore::Shutdown();

	return 0;
}
