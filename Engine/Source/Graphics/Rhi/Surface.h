#pragma once
#include <memory>

#include "Core/Window.h"

namespace SnowEngine
{
	class CommandBuffer;

	class Surface
	{
	public:
		static std::shared_ptr<Surface> Create(std::shared_ptr<const Window> window);
		virtual ~Surface() = default;

		virtual u32 ImageCount() const = 0;
		virtual u32 CurrentFrame() const = 0;

		virtual void Begin() = 0;
		virtual void End(const std::shared_ptr<const CommandBuffer>& commandBuffer) = 0;
	};
}
