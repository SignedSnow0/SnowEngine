#include "SceneRenderer.h"

#include "Core/Components.h"
#include "Core/Entity.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SnowEngine
{
	static std::vector<Vertex> sCubeVertices
	{
		{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //TLF
		{ {  1.0f,  1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //TRF
		{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //BLF
		{ {  1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //BRF

		{ { -1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //TLB
		{ {  1.0f,  1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //TRB
		{ { -1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //BLB
		{ {  1.0f, -1.0f,  1.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }, //BRB
	};

	static std::vector<u32> sCubeIndices
	{
		//Front
		0, 1, 2,
		2, 1, 3,

		//Right
		1, 5, 3,
		3, 5, 7,

		//Back
		5, 7, 4,
		4, 7, 6,

		//Left
		4, 0, 6,
		6, 0, 2,

		//Bottom
		2, 3, 6,
		6, 3, 7,

		//Top
		0, 1, 4,
		4, 1, 5
	};

	SceneRenderer::SceneRenderer(const std::shared_ptr<Surface>& surface)
	{
		mRenderPass = RenderPass::Create(surface->ImageCount(), 1920, 1080, true);
		mShader = Shader::Create(
		{
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.vert", ShaderType::Vertex },
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/default.frag", ShaderType::Fragment },
			{}
		}, "default");
		mPipeline = Pipeline::Create({ mShader, mRenderPass, 2560, 1440 });
		mCmdBuffer = CommandBuffer::Create(surface->ImageCount(), CommandBufferUsage::Graphics);

		mGlobalDescriptorSet = DescriptorSet::Create(mShader, 0, 2);

		mSkyboxShader = Shader::Create(
		{
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/skybox.vert", ShaderType::Vertex },
			{ "D:/Dev/SnowEngine/Engine/Resources/Shaders/skybox.frag", ShaderType::Fragment },
			{}
		}, "skybox");

		PipelineSettings settings{ mSkyboxShader, mRenderPass, 2560, 1440 };
		settings.BackfaceCulling = false;
		settings.DepthWrite = false;

		mSkyboxPipeline = Pipeline::Create(settings);

		mSkyboxImage = Image::Create(
		{
			"D:/Dev/SnowEngine/Engine/Resources/Images/right.jpg",
			"D:/Dev/SnowEngine/Engine/Resources/Images/left.jpg",
			"D:/Dev/SnowEngine/Engine/Resources/Images/top.jpg",
			"D:/Dev/SnowEngine/Engine/Resources/Images/bottom.jpg",
			"D:/Dev/SnowEngine/Engine/Resources/Images/front.jpg",
			"D:/Dev/SnowEngine/Engine/Resources/Images/back.jpg"
		});

		mSkyboxDescriptorSet = DescriptorSet::Create(mSkyboxShader, 0, 2);
		mSkyboxDescriptorSet->SetImage("skybox", mSkyboxImage);

		mSkyboxVertexBuffer = VertexBuffer::Create(sCubeVertices.data(), static_cast<u32>(sCubeVertices.size()));
		mSkyboxIndexBuffer = IndexBuffer::Create(sCubeIndices.data(), static_cast<u32>(sCubeIndices.size()));
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

		camera.View = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		camera.Projection = glm::perspective(glm::radians(45.0f), mRenderPass->Width() / static_cast<f32>(mRenderPass->Height()), 0.1f, 10.0f);
		camera.Projection[1][1] *= -1;

		mCmdBuffer->Begin(surface->CurrentFrame());

		mRenderPass->Begin(mCmdBuffer);

		mSkyboxDescriptorSet->SetUniform("Camera", &camera, surface->CurrentFrame());

		mSkyboxPipeline->Bind(mCmdBuffer);
		mSkyboxPipeline->BindDescriptorSet(mSkyboxDescriptorSet.get(), surface->CurrentFrame(), mCmdBuffer);

		mSkyboxVertexBuffer->Bind(mCmdBuffer);
		mSkyboxIndexBuffer->Bind(mCmdBuffer);
		mSkyboxIndexBuffer->Draw(mCmdBuffer);

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
