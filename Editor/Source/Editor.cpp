#include "Editor.h"
#include <SnowEngine.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <imgui.h>

int main()
{
	SnowEngine::GraphicsCore::Init();
	{
		const auto window = SnowEngine::Window::Create("SnowEngine", 2560, 1440, true, true, true);
		const auto surface = SnowEngine::Surface::Create(window);
		const auto renderPass = SnowEngine::RenderPass::Create(surface->ImageCount(), 1920, 1080);
		const auto shader = SnowEngine::Shader::Create(
		{
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.vert", SnowEngine::ShaderType::Vertex },
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.frag", SnowEngine::ShaderType::Fragment },
			{}
		});

		const auto graphicsCmd = SnowEngine::CommandBuffer::Create(surface->ImageCount(), SnowEngine::CommandBufferUsage::Graphics);

		const auto image = SnowEngine::Image::Create("D:/Dev/SnowEngine/Engine/Resources/Images/sus.png");

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

		const auto gui = SnowEngine::Gui::Create(surface, renderPass);

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (!window->Closing())
		{
			currentTime = std::chrono::high_resolution_clock::now();
			const f32 time = std::chrono::duration<f32, std::chrono::seconds::period>(currentTime - lastTime).count();

			surface->Begin();

			camera.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			camera.Projection = glm::perspective(glm::radians(45.0f), renderPass->Width() / static_cast<f32>(renderPass->Height()), 0.1f, 10.0f);
			camera.Projection[1][1] *= -1;

			transform.Model = glm::mat4(1.0f);

			graphicsCmd->Begin(surface->CurrentFrame());

			renderPass->Begin(graphicsCmd);

			cameraDescriptorSet->SetUniform("Camera", &camera, surface->CurrentFrame());
			modelDescriptorSet->SetUniform("Transform", &transform, surface->CurrentFrame());

			pipeline->Bind(graphicsCmd);
			pipeline->BindDescriptorSet(cameraDescriptorSet.get(), surface->CurrentFrame(), graphicsCmd);
			pipeline->BindDescriptorSet(modelDescriptorSet.get(), surface->CurrentFrame(), graphicsCmd);

			vertexBuffer->Bind(graphicsCmd);
			indexBuffer->Bind(graphicsCmd);
			indexBuffer->Draw(graphicsCmd);

			renderPass->End(graphicsCmd);

			gui->Begin(graphicsCmd);

			if (ImGui::Begin("Entities"))
			{

			}
			ImGui::End();
			if (ImGui::Begin("Components"))
			{

			}
			ImGui::End();

			gui->End(graphicsCmd);

			graphicsCmd->End(surface->CurrentFrame());

			graphicsCmd->Submit(surface->CurrentFrame(), surface);

			surface->End(graphicsCmd);

			SnowEngine::Window::Update();

			lastTime = currentTime;
		}
	}
	SnowEngine::GraphicsCore::Shutdown();

	return 0;
}
