#pragma once
#include <memory>

#include "Mesh.h"
#include "Core/Scene.h"
#include "Rhi/Pipeline.h"
#include "Rhi/RenderPass.h"
#include "Rhi/Shader.h"

namespace SnowEngine
{
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<Surface>& surface);

		const std::shared_ptr<RenderPass>& GetRenderPass() const;
		const std::shared_ptr<CommandBuffer>& GetCommandBuffer() const;
		void SetScene(const std::shared_ptr<Scene>& scene);

		void Draw(const std::shared_ptr<Surface>& surface) const;

	private:
		std::shared_ptr<RenderPass> mRenderPass{ nullptr };
		std::shared_ptr<Shader> mShader{ nullptr };
		std::shared_ptr<Pipeline> mPipeline{ nullptr };
		std::shared_ptr<DescriptorSet> mGlobalDescriptorSet{ nullptr };
		std::shared_ptr<CommandBuffer> mCmdBuffer{ nullptr };

		std::shared_ptr<Shader> mSkyboxShader{ nullptr };
		std::shared_ptr<Pipeline> mSkyboxPipeline{ nullptr };
		std::shared_ptr<Image> mSkyboxImage{ nullptr };
		std::shared_ptr<DescriptorSet> mSkyboxDescriptorSet{ nullptr };
		std::shared_ptr<VertexBuffer> mSkyboxVertexBuffer{ nullptr };
		std::shared_ptr<IndexBuffer> mSkyboxIndexBuffer{ nullptr };

		std::shared_ptr<Scene> mScene;
	};
}
