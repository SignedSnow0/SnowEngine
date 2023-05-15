#include "VkBuffers.h"

#include "VkCore.h"
#include "VkSurface.h"
#include "VkCommandBuffer.h"

namespace SnowEngine
{
	VkBuffer::VkBuffer(const u32 size, const vk::BufferUsageFlags usage, const VmaMemoryUsage memoryUsage)
		: mSize{ size }
	{
		vk::BufferCreateInfo createInfo{};
		createInfo.size = size;
		createInfo.usage = usage;
		createInfo.sharingMode = vk::SharingMode::eExclusive;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;

		vmaCreateBuffer(VkCore::Get()->Allocator(), reinterpret_cast<VkBufferCreateInfo*>(&createInfo), &allocInfo, reinterpret_cast<::VkBuffer*>(&mBuffer), &mAllocation, nullptr);
	}

	VkBuffer::~VkBuffer()
	{
		vmaDestroyBuffer(VkCore::Get()->Allocator(), mBuffer, mAllocation);
	}

	vk::Buffer VkBuffer::Buffer() const { return mBuffer; }

	u32 VkBuffer::Size() const { return mSize; }

	void VkBuffer::InsertData(const void* data, const u32 size, const u32 offset) const
	{
		void* gpuMemory;
		vmaMapMemory(VkCore::Get()->Allocator(), mAllocation, &gpuMemory);
		memcpy(static_cast<u8*>(gpuMemory) + offset, data, size ? size : mSize);
		vmaUnmapMemory(VkCore::Get()->Allocator(), mAllocation);
	}

	void VkBuffer::CopyBuffer(const vk::Buffer src, const vk::Buffer dst, const vk::DeviceSize size)
    {
		VkCore::Get()->SubmitInstantCommand([&](const vk::CommandBuffer cmd)
		{
			vk::BufferCopy copyRegion{};
			copyRegion.size = size;
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;

			cmd.copyBuffer(src, dst, copyRegion);
		});
    }

	VkVertexBuffer::VkVertexBuffer(const Vertex* vertices, const u32 vertexCount)
		: VkBuffer(vertexCount * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY),
		  mCount{ vertexCount }
	{
		const VkBuffer staging(vertexCount * sizeof(Vertex), vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.InsertData(vertices);

		CopyBuffer(staging.Buffer(), mBuffer, mSize);
	}

	void VkVertexBuffer::Bind(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().bindVertexBuffers(0, mBuffer, { 0 });
	}

	void VkVertexBuffer::Draw(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().draw(mCount, 1, 0, 0);
	}

	vk::VertexInputBindingDescription VkVertexBuffer::BindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		return bindingDescription;
	}

	std::vector<vk::VertexInputAttributeDescription> VkVertexBuffer::AttributeDescriptions()
	{
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
		attributeDescriptions.resize(3);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(Vertex, Color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[2].offset = offsetof(Vertex, Uv);

		return attributeDescriptions;
	}

	VkIndexBuffer::VkIndexBuffer(const u32* indices, const u32 indexCount)
		: VkBuffer(indexCount * sizeof(u32), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY),
		  mCount{ indexCount }
	{
		const VkBuffer staging(indexCount * sizeof(u32), vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY);
		staging.InsertData(indices);

		CopyBuffer(staging.Buffer(), mBuffer, mSize);
	}

	void VkIndexBuffer::Bind(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().bindIndexBuffer(mBuffer, 0, vk::IndexType::eUint32);
	}

	void VkIndexBuffer::Draw(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().drawIndexed(mCount, 1, 0, 0, 0);
	}

	VkUniformBuffer::VkUniformBuffer(const u32 size, const u32 frameCount)
	{
		for (u32 i{ 0 }; i < frameCount; i++)
			mBuffers.push_back(std::make_unique<VkBuffer>(size, vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU));
	}

	const std::vector<std::unique_ptr<VkBuffer>>& VkUniformBuffer::Buffers() const { return mBuffers; }

	void VkUniformBuffer::InsertData(const void* data, const u32 frameIndex) const
	{
		mBuffers.at(frameIndex)->InsertData(data);
	}

	VkStorageBuffer::VkStorageBuffer(const u32 size)
		: mSize{ size }
	{
		mBuffers = std::make_unique<VkBuffer>(
			mSize,
			vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eStorageBuffer,
			VMA_MEMORY_USAGE_GPU_ONLY);
	}

	u32 VkStorageBuffer::Size() const { return mSize; }

	const std::unique_ptr<VkBuffer>& VkStorageBuffer::Buffers() const {	return mBuffers; }

	void VkStorageBuffer::SetData(const std::shared_ptr<StorageBuffer>& other) const
	{
		const auto& buffers = std::static_pointer_cast<VkStorageBuffer>(other)->Buffers();

		const vk::DeviceSize deviceSize{ mSize };
		VkBuffer::CopyBuffer(buffers->Buffer(), mBuffers->Buffer(), deviceSize);
	}

	void VkStorageBuffer::SetData(const void* data) const
	{
		const VkBuffer staging{ mSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_TO_GPU };
		staging.InsertData(data);

		const vk::DeviceSize deviceSize{ staging.Size() };
		VkBuffer::CopyBuffer(staging.Buffer(), mBuffers->Buffer(), deviceSize);
	}
}
