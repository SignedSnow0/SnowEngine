#pragma once
#include "VkShader.h"
#include "VkRenderPass.h"
#include "Graphics/Rhi/Pipeline.h"

namespace SnowEngine
{
	class VkPipeline : public Pipeline
	{
	public:
		VkPipeline(std::shared_ptr<const VkShader> shader, std::shared_ptr<const VkRenderPass> renderPass, u32 width, u32 height);

		void Bind() const override;
		void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame) const override;

	private:
		void CreateLayout();
		void CreateFixedFunctions(u32 width, u32 height);

		std::shared_ptr<const VkShader> mShader;
		std::shared_ptr<const VkRenderPass> mRenderPass;
		vk::PipelineLayout mLayout;
		vk::Pipeline mPipeline;
	};
}
