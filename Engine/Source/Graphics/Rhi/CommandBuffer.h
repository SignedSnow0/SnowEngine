#pragma once
#include <memory>

#include "Surface.h"
#include "Core/Types.h"

namespace SnowEngine
{
	enum class CommandBufferUsage
	{
		Compute,
		Graphics,
		Copy
	};

	class CommandBuffer
	{
	public:
		static std::shared_ptr<CommandBuffer> Create(u32 frameCount, CommandBufferUsage usage);
		virtual ~CommandBuffer() = default;

		virtual void Begin(u32 currentFrame) const = 0;
		virtual void End(u32 currentFrame) const = 0;
		virtual void Submit(u32 currentFrame, const std::shared_ptr<const CommandBuffer>& previousCmd) const = 0;
		virtual void Submit(u32 currentFrame, const std::shared_ptr<const Surface>& surface) const = 0;
	};
}
