#pragma once
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

#include "Core/Types.h"
#include "Graphics/Rhi/Buffers.h"
#include "Graphics/Rhi/CommandBuffer.h"

namespace SnowEngine
{
	class VkBuffer
	{
	public:
		VkBuffer(u32 size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		virtual ~VkBuffer();

		vk::Buffer Buffer() const;
		u32 Size() const;

		void InsertData(const void* data, u32 size = 0, u32 offset = 0) const;

		static void CopyBuffer(vk::Buffer src, vk::Buffer dst, vk::DeviceSize size);

	protected:
		vk::Buffer mBuffer;
		VmaAllocation mAllocation;
		u32 mSize;
	};

	class VkVertexBuffer : public VkBuffer, public VertexBuffer
	{
	public:
		VkVertexBuffer(const Vertex* vertices, u32 vertexCount);

		void Bind(const std::shared_ptr<CommandBuffer>& cmd) const override;
		void Draw(const std::shared_ptr<CommandBuffer>& cmd) const override;

		static vk::VertexInputBindingDescription BindingDescription();
		static std::vector<vk::VertexInputAttributeDescription> AttributeDescriptions();

	private:
		u32 mCount;
	};

	class VkIndexBuffer : public VkBuffer, public IndexBuffer
	{
	public:
		VkIndexBuffer(const u32* indices, u32 indexCount);

		void Bind(const std::shared_ptr<CommandBuffer>& cmd) const override;
		void Draw(const std::shared_ptr<CommandBuffer>& cmd) const override;

	private:
		u32 mCount;
	};

	class VkUniformBuffer
	{
	public:
		VkUniformBuffer(u32 size, u32 frameCount);

		const std::vector<std::unique_ptr<VkBuffer>>& Buffers() const;

		void InsertData(const void* data, u32 frameIndex) const;

	private:
		std::vector<std::unique_ptr<VkBuffer>> mBuffers;
	};

	class VkStorageBuffer : public StorageBuffer
	{
	public:
		VkStorageBuffer(u32 size);

		u32 Size() const;
		const std::unique_ptr<VkBuffer>& Buffers() const;

		void SetData(const std::shared_ptr<StorageBuffer>& other) const override;
		void SetData(const void* data) const override;

	private:
		std::unique_ptr<VkBuffer> mBuffers;
		u32 mSize{ 0 };
	};
}
