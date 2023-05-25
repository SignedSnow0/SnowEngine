#pragma once
#include "Buffers.h"
#include "Image.h"
#include "Shader.h"
#include "Core/Types.h"

namespace SnowEngine
{
	class DescriptorSet
	{
	public:
		static std::shared_ptr<DescriptorSet> Create(const std::shared_ptr<const Shader>& shader, u32 setIndex, u32 frameCount);
		virtual ~DescriptorSet() = default;

		virtual void SetUniform(const std::string& name, const void* data, u32 currentFrame) const = 0;
		virtual void SetImage(const std::string& name, const std::shared_ptr<Image>& image) = 0;
		virtual void SetStorageBuffer(const std::string& name, const std::shared_ptr<StorageBuffer>& buffer) = 0;
	};
}
