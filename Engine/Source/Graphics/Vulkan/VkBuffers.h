#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "Core/Types.h"
#include "Graphics/Rhi/Buffers.h"

namespace SnowEngine
{
	class VkBuffer
	{
	public:
		VkBuffer(u32 size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		virtual ~VkBuffer();

		vk::Buffer GetBuffer() const;
		u32 GetSize() const;

		void InsertData(const void* data, u32 size = 0, u32 offset = 0) const;

	protected:
		static void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);

		vk::Buffer mBuffer;
		VmaAllocation mAllocation;
		u32 mSize;
	};

	class VkVertexBuffer : public VkBuffer, public VertexBuffer
	{
	public:
		VkVertexBuffer(const Vertex* vertices, u32 vertexCount);

		void Bind() const override;
		void Draw() const override;

		static vk::VertexInputBindingDescription BindingDescription();
		static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

	private:
		u32 mCount;
	};

	class VkIndexBuffer : public VkBuffer, public IndexBuffer
	{
	public:
		VkIndexBuffer(const u32* indices, u32 indexCount);

		void Bind() const override;
		void Draw() const override;

	private:
		u32 mCount;
	};

	class VkUniformBuffer
	{
	public:
		VkUniformBuffer(u32 size, u32 frameCount);
		~VkUniformBuffer();

		const std::vector<VkBuffer*>& GetBuffers() const;

		void InsertData(const void* data, u32 frameIndex) const;

	private:
		std::vector<VkBuffer*> mBuffers;
	};
}
