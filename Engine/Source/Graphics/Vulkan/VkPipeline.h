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

		void Bind(const std::shared_ptr<CommandBuffer>& cmd) const override;
		void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const override;

	private:
		void CreateLayout();
		void CreateFixedFunctions(u32 width, u32 height);

		std::shared_ptr<const VkShader> mShader;
		std::shared_ptr<const VkRenderPass> mRenderPass;
		vk::PipelineLayout mLayout;
		vk::Pipeline mPipeline;
	};

	class VkComputePipeline : public ComputePipeline
	{
	public:
		VkComputePipeline(std::shared_ptr<const VkShader> shader);

		void Dispatch(u32 x, u32 y, u32 z, const std::shared_ptr<CommandBuffer>& cmd) const override;
		void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const override;

	private:
		void CreateLayout();
		void CreatePipeline();

		std::shared_ptr<const VkShader> mShader;
		vk::PipelineLayout mLayout;
		vk::Pipeline mPipeline;
	};
}
