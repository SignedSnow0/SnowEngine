#include "VkDescriptorSet.h"

#include "VkCore.h"
#include "VkImage.h"

namespace SnowEngine
{
	VkDescriptorSet::VkDescriptorSet(const VkDescriptorSetLayout& layout, const u32 frameCount)
		: mLayout{ layout }, mFrameCount{ frameCount }
	{
		CreatePool();
		CreateSets();
		CreateBuffers();
	}

	const std::vector<vk::DescriptorSet>& VkDescriptorSet::Sets() const {	return mSets; }

	set VkDescriptorSet::SetIndex() const { return mLayout.SetIndex; }

	void VkDescriptorSet::SetUniform(const std::string& name, const void* data, const u32 currentFrame) const
	{
		for (const auto& [binding, resource] : mLayout.Resources)
		{
			if (resource.Name == name && resource.Type == VkResourceType::Uniform)
				mUniforms.at(binding)->InsertData(data, currentFrame);
		}
	}

	void VkDescriptorSet::SetImage(const std::string& name, const std::shared_ptr<Image>& image)
	{
		const auto vkImage{ reinterpret_cast<const VkImage*>(image.get()) };
		for (const auto& [binding, resource] : mLayout.Resources)
		{
			if (resource.Name == name && resource.Type == VkResourceType::Image)
			{
				vk::DescriptorImageInfo imageInfo{};
				imageInfo.imageLayout = vkImage->Layout();
				imageInfo.imageView = vkImage->View();
				imageInfo.sampler = mImages.at(binding).second;

				for (const auto set : mSets)
				{
					vk::WriteDescriptorSet write{};
					write.dstSet = set;
					write.dstBinding = binding;
					write.dstArrayElement = 0;
					write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
					write.descriptorCount = 1;
					write.pImageInfo = &imageInfo;

					VkCore::Get()->Device().updateDescriptorSets(write, nullptr);
				}

				mImages.at(binding).first = image;
			}
		}
	}

	void VkDescriptorSet::SetStorageBuffer(const std::string& name, const std::shared_ptr<StorageBuffer>& buffer)
	{
		for (const auto& [binding, resource] : mLayout.Resources)
		{
			if (resource.Name == name && resource.Type == VkResourceType::StorageBuffer)
			{
				mStorageBuffers[binding] = std::static_pointer_cast<VkStorageBuffer>(buffer);

 				for (u32 i{ 0 }; i < mSets.size(); i++)
				{
					vk::DescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = mStorageBuffers[binding]->Buffers()->Buffer();
					bufferInfo.offset = 0;
					bufferInfo.range = mStorageBuffers[binding]->Buffers()->Size();

					vk::WriteDescriptorSet descriptorWrite{};
					descriptorWrite.pBufferInfo = &bufferInfo;
					descriptorWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.dstBinding = binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.dstSet = mSets.at(i);

					VkCore::Get()->Device().updateDescriptorSets(descriptorWrite, nullptr);
				}
			}
		}

	}

	void VkDescriptorSet::CreatePool()
	{
		std::vector<vk::DescriptorPoolSize> sizes{};
		u32 uniformCount{ 0 }, imageCount{ 0 }, storageBufferCount{ 0 };
		for(const auto& [binding, resource] : mLayout.Resources)
		{
			if (resource.Type == VkResourceType::Uniform)
				uniformCount++;
			else if (resource.Type == VkResourceType::Image)
				imageCount++;
			else if (resource.Type == VkResourceType::StorageBuffer)
				storageBufferCount++;
		}

		uniformCount *= mFrameCount;
		imageCount *= mFrameCount;
		storageBufferCount *= mFrameCount;

		if (uniformCount)
			sizes.emplace_back(vk::DescriptorType::eUniformBuffer, uniformCount);
		if (imageCount)
			sizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, imageCount);
		if (storageBufferCount)
			sizes.emplace_back(vk::DescriptorType::eStorageBuffer, storageBufferCount);

		vk::DescriptorPoolCreateInfo createInfo{};
		createInfo.poolSizeCount = static_cast<u32>(sizes.size());
		createInfo.pPoolSizes = sizes.data();
		createInfo.maxSets = mFrameCount;

		mPool = VkCore::Get()->Device().createDescriptorPool(createInfo);
	}

	void VkDescriptorSet::CreateSets()
	{
		const std::vector<vk::DescriptorSetLayout> layouts{ mFrameCount, mLayout.CreateLayout() };
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = mPool;
		allocInfo.descriptorSetCount = static_cast<u32>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();

		mSets = VkCore::Get()->Device().allocateDescriptorSets(allocInfo);
	}

	void VkDescriptorSet::CreateBuffers()
	{
		for (const auto& [binding, resource] : mLayout.Resources)
		{
			if (resource.Type == VkResourceType::Uniform)
			{
				mUniforms.insert({ binding, std::make_unique<VkUniformBuffer>(resource.Size, mFrameCount) });

				u32 i{ 0 };
				for (const auto& buffer : mUniforms.at(binding)->Buffers())
				{
					vk::DescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = buffer->Buffer();
					bufferInfo.offset = 0;
					bufferInfo.range = buffer->Size();

					vk::WriteDescriptorSet descriptorWrite{};
					descriptorWrite.pBufferInfo = &bufferInfo;
					descriptorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.dstBinding = binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.dstSet = mSets.at(i);

					VkCore::Get()->Device().updateDescriptorSets(descriptorWrite, nullptr);

					i++;
				}
			}

			if (resource.Type == VkResourceType::Image)
			{
				vk::SamplerCreateInfo createInfo{};
				createInfo.magFilter = vk::Filter::eLinear;
				createInfo.minFilter = vk::Filter::eLinear;
				createInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
				createInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
				createInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
				createInfo.mipLodBias = 0.0f;
				createInfo.anisotropyEnable = VK_FALSE;
				createInfo.maxAnisotropy = 1.0f;
				createInfo.compareEnable = VK_FALSE;
				createInfo.compareOp = vk::CompareOp::eAlways;
				createInfo.minLod = 0.0f;
				createInfo.maxLod = 1.0f;
				createInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

				mImages.insert({ binding, { nullptr, VkCore::Get()->Device().createSampler(createInfo) } });
			}

			if (resource.Type == VkResourceType::StorageBuffer)
			{
				mStorageBuffers.insert({ binding, std::make_unique<VkStorageBuffer>(64) });

				for (u32 i{ 0 }; i < mSets.size(); i++)
				{
					vk::DescriptorBufferInfo bufferInfo{};
					bufferInfo.buffer = mStorageBuffers[binding]->Buffers()->Buffer();
					bufferInfo.offset = 0;
					bufferInfo.range = mStorageBuffers[binding]->Buffers()->Size();

					vk::WriteDescriptorSet descriptorWrite{};
					descriptorWrite.pBufferInfo = &bufferInfo;
					descriptorWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.dstBinding = binding;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.dstSet = mSets.at(i);

					VkCore::Get()->Device().updateDescriptorSets(descriptorWrite, nullptr);
				}
			}
		}
	}
}
