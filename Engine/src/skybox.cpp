#include "skybox.h"
#include <filesystem>

#include <stb_image.h>

#include "application.h"

namespace SnowEngine {
	static const std::vector<float> skyboxVertices = {
		-1000.0f,-1000.0f,-1000.0f,  // -X side
		-1000.0f,-1000.0f, 1000.0f,
		-1000.0f, 1000.0f, 1000.0f,
		-1000.0f, 1000.0f, 1000.0f,
		-1000.0f, 1000.0f,-1000.0f,
		-1000.0f,-1000.0f,-1000.0f,

		-1000.0f,-1000.0f,-1000.0f,  // -Z side
		 1000.0f, 1000.0f,-1000.0f,
		 1000.0f,-1000.0f,-1000.0f,
		-1000.0f,-1000.0f,-1000.0f,
		-1000.0f, 1000.0f,-1000.0f,
		 1000.0f, 1000.0f,-1000.0f,

		-1000.0f,-1000.0f,-1000.0f,  // -Y side
		 1000.0f,-1000.0f,-1000.0f,
		 1000.0f,-1000.0f, 1000.0f,
		-1000.0f,-1000.0f,-1000.0f,
		 1000.0f,-1000.0f, 1000.0f,
		-1000.0f,-1000.0f, 1000.0f,

		-1000.0f, 1000.0f,-1000.0f,  // +Y side
		-1000.0f, 1000.0f, 1000.0f,
		 1000.0f, 1000.0f, 1000.0f,
		-1000.0f, 1000.0f,-1000.0f,
		 1000.0f, 1000.0f, 1000.0f,
		 1000.0f, 1000.0f,-1000.0f,

		 1000.0f, 1000.0f,-1000.0f,  // +X side
		 1000.0f, 1000.0f, 1000.0f,
		 1000.0f,-1000.0f, 1000.0f,
		 1000.0f,-1000.0f, 1000.0f,
		 1000.0f,-1000.0f,-1000.0f,
		 1000.0f, 1000.0f,-1000.0f,

		-1000.0f, 1000.0f, 1000.0f,  // +Z side
		-1000.0f,-1000.0f, 1000.0f,
		 1000.0f, 1000.0f, 1000.0f,
		-1000.0f,-1000.0f, 1000.0f,
		 1000.0f,-1000.0f, 1000.0f,
		 1000.0f, 1000.0f, 1000.0f,
	};

	Skybox::Skybox(Device& device, std::vector<std::string> textures, uint32_t binding, VkShaderStageFlags shaderTarget) : device(device), binding(binding) {
		for(size_t i = 0; i < textures.size();  i++)
			textures[i] = std::filesystem::absolute(textures[i]).string();
		CreateImage(textures);
		CreateSampler();
		CreateImageView();
		CreateBinding(shaderTarget);
		CreatePipeline();
		vBuffer = new SkyVertexBuffer(device, skyboxVertices);
	}

	std::vector<VkWriteDescriptorSet> Skybox::GetDescriptorWrite(uint32_t i, VkDescriptorSet descriptorSet) {
		std::vector<VkWriteDescriptorSet> writes;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = imageSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;
		writes.push_back(descriptorWrite);

		writes.push_back(uBuffer.CreateDescriptorWrite(i, descriptorSet));
		return writes;
	}

	void Skybox::Draw(uint32_t frame, VkCommandBuffer buffer, const glm::mat4& view, const glm::mat4& proj) {
		uBuffer.Update(frame, { glm::mat4(glm::mat3(view)), proj });

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());

		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetLayout(), 0, 1, &descriptorSets[frame], 0, nullptr);

		vBuffer->Bind(buffer);
		vkCmdDraw(buffer, 36, 1, 0, 0);
	}

	void Skybox::CreateImage(const std::vector<std::string >& textures) {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(textures[0].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * 6 * texHeight * 4;

		if (!pixels)
			throw std::runtime_error("Failed to load cubemap images!");

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		stbi_image_free(pixels);
		for (uint32_t i = 0; i < textures.size(); i++) {
			stbi_uc* pixels = stbi_load(textures[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
			void* tmp; //metto i pixel dell`immagine in un buffer
			uint32_t offset = texWidth * texHeight * 4 * i;
			vkMapMemory(device, stagingBufferMemory, offset, VK_WHOLE_SIZE, 0, &tmp);
			memcpy(tmp, pixels, static_cast<size_t>(imageSize) / 6);
			vkUnmapMemory(device, stagingBufferMemory);
			stbi_image_free(pixels);
		}

		device.CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory, VK_IMAGE_TYPE_2D, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

		TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		TransitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void Skybox::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 6;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else
			throw std::invalid_argument("Unsupported layout transition!");

		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		device.EndSingleTimeCommands(commandBuffer);
	}

	void Skybox::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 6;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		device.EndSingleTimeCommands(commandBuffer);
	}

	void Skybox::CreateImageView() {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		createInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 6;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.image = image;
		
		if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create cubemap view!");
	}

	void Skybox::CreateSampler() {
		auto properties = device.GetDeviceProperties();

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = VK_FILTER_LINEAR;
		createInfo.minFilter = VK_FILTER_LINEAR;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		createInfo.mipLodBias = 0.0f;
		createInfo.compareOp = VK_COMPARE_OP_NEVER;
		createInfo.minLod = 0.0f;
		createInfo.maxLod = 0.0f;
		createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		createInfo.anisotropyEnable = VK_TRUE;
		createInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		
		if (vkCreateSampler(device, &createInfo, nullptr, &imageSampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create cubemap sampler!");
	}

	void Skybox::CreateBinding(VkShaderStageFlags shaderTarget) {
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = shaderTarget;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { uBuffer.GetLayoutBinding(), layoutBinding };

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &layout))
			throw std::runtime_error("Failed to create descriptor set layout!");

		std::vector<VkDescriptorSetLayout> layouts(3, layout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(3);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(3);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!");

		for (size_t i = 0; i < 3; i++) {
			std::vector<VkWriteDescriptorSet> descriptorWrites{ GetDescriptorWrite(i, descriptorSets[i]) };
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void Skybox::CreatePipeline()
	{
		Pipeline::PipelineConfig config = Pipeline::FillPipelineConfig();
		config.renderPass = Application::Get().GetSwapchain().GetRenderPass();

		config.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
		config.layouts.insert({ 0, layout });
		config.vAttributes = SkyVertexBuffer::GetAttributeDescriptions();
		auto descs = SkyVertexBuffer::GetBindingDescription();
		config.vBindings = descs;

		pipeline = new Pipeline(device, config, "resources/shaders/spirv/skybox.vert.spv", "resources/shaders/spirv/skybox.frag.spv");
	}

	VkVertexInputBindingDescription Skybox::SkyVertexBuffer::GetBindingDescription() {
		VkVertexInputBindingDescription description{};
		description.binding = 0;
		description.stride = sizeof(float) * 3;
		description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return description;
	}

	std::vector<VkVertexInputAttributeDescription> Skybox::SkyVertexBuffer::GetAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		VkVertexInputAttributeDescription description{};
		description.binding = 0;
		description.location = 0;
		description.format = VK_FORMAT_R32G32B32_SFLOAT;
		description.offset = 0;

		attributeDescriptions.push_back(description);
		return attributeDescriptions;		
	}
}