#pragma once
#include "VkShader.h"
#include "VkRenderPass.h"
#include "Graphics/Rhi/Pipeline.h"

namespace SnowEngine
{
	class VkPipeline : public Pipeline
	{
	public:
		VkPipeline(const PipelineSettings& settings);

		void Bind(const std::shared_ptr<CommandBuffer>& cmd) const override;
		void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const override;

	private:
		void CreateLayout();
		void CreateFixedFunctions(const PipelineSettings& settings);

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
