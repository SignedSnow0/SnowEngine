#pragma once
#include <memory>

#include "Surface.h"

namespace SnowEngine
{
	class RenderPass
	{
	public:
		static std::shared_ptr<RenderPass> Create(const std::shared_ptr<const Surface>& surface, b8 depth);
		static std::shared_ptr<RenderPass> Create(u32 frameCount, u32 width, u32 height, b8 depth);
		virtual ~RenderPass() = default;

		virtual u32 Width() const = 0;
		virtual u32 Height() const = 0;

		virtual void Begin(const std::shared_ptr<CommandBuffer>& cmd) = 0;
		virtual void End(const std::shared_ptr<CommandBuffer>& cmd) const = 0;
	};
}
