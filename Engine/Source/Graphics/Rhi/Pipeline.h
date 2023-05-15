#pragma once
#include "DescriptorSet.h"
#include "Shader.h"
#include "RenderPass.h"
#include "Core/Types.h"

namespace SnowEngine
{
	class Pipeline
	{
	public:
		static std::shared_ptr<Pipeline> Create(const std::shared_ptr<const Shader>& shader, const std::shared_ptr<const RenderPass>& renderPass, u32 width, u32 height);
		virtual ~Pipeline() = default;

		virtual void Bind(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
		virtual void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const = 0;
	};

	class ComputePipeline
	{
	public:
		static std::shared_ptr<ComputePipeline> Create(const std::shared_ptr<const Shader>& shader);
		virtual ~ComputePipeline() = default;

		virtual void Dispatch(u32 x, u32 y, u32 z, const std::shared_ptr<CommandBuffer>& cmd) const = 0;
		virtual void BindDescriptorSet(const DescriptorSet* set, u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const = 0;
	};
}
