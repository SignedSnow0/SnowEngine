#include "SceneRenderer.h"

#include "Core/Components.h"
#include "Core/Entity.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SnowEngine
{
	SceneRenderer::SceneRenderer(const std::shared_ptr<Surface>& surface)
	{
		mRenderPass = RenderPass::Create(surface->ImageCount(), 1920, 1080, true);
		mShader = Shader::Create(
		{
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.vert", ShaderType::Vertex },
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.frag", ShaderType::Fragment },
			{}
		}, "default");
		mPipeline = Pipeline::Create(mShader, mRenderPass, 2560, 1440);
		mCmdBuffer = CommandBuffer::Create(surface->ImageCount(), CommandBufferUsage::Graphics);

		mGlobalDescriptorSet = DescriptorSet::Create(mShader, 0, 2);
	}

	const std::shared_ptr<RenderPass>& SceneRenderer::GetRenderPass() const { return mRenderPass; }

	const std::shared_ptr<CommandBuffer>& SceneRenderer::GetCommandBuffer() const { return mCmdBuffer; }

	void SceneRenderer::SetScene(const std::shared_ptr<Scene>& scene) { mScene = scene;	}

	void SceneRenderer::Draw(const std::shared_ptr<Surface>& surface) const
	{
		struct Camera
		{
			glm::mat4 View;
			glm::mat4 Projection;
		}
		static camera{};

		camera.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		camera.Projection = glm::perspective(glm::radians(45.0f), mRenderPass->Width() / static_cast<f32>(mRenderPass->Height()), 0.1f, 10.0f);
		camera.Projection[1][1] *= -1;

		mCmdBuffer->Begin(surface->CurrentFrame());

		mRenderPass->Begin(mCmdBuffer);

		mGlobalDescriptorSet->SetUniform("Camera", &camera, surface->CurrentFrame());

		mPipeline->Bind(mCmdBuffer);
		mPipeline->BindDescriptorSet(mGlobalDescriptorSet.get(), surface->CurrentFrame(), mCmdBuffer);

		mScene->ExecuteSystem([&](const Entity& e)
		{
			if (e.HasComponents<Component::Transform, Component::Mesh>())
			{
				const auto [transform, mesh] = e.GetComponents<Component::Transform, Component::Mesh>();

				mesh.Model->SetTransform(transform.Model(), surface->CurrentFrame());

				mesh.Model->Draw(mPipeline, mCmdBuffer, surface->CurrentFrame());
			}
		});

		mRenderPass->End(mCmdBuffer);
	}
}
