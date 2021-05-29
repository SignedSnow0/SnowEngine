#include "ShadowMap.h"
#include <array>
#include <glm\gtc\matrix_transform.hpp>

namespace SnowEngine {
	ShadowMap::ShadowMap(Device& device, uint32_t matrixbinding, uint32_t imageBinding, VkShaderStageFlags matrixStage, VkShaderStageFlags imageStage) : device(device), imageLocation(imageBinding) {
		CreateImageResources();
		CreateRenderPass();
		CreateFramebuffer();
		CreateUniforms();
		CreatePipeline();
		CreateBindings(imageBinding, imageStage);

		uBufferOut = new UniformBuffer<InUBO>(device, matrixStage, matrixbinding, inUbo, 3);
	}

	ShadowMap::~ShadowMap() {
		delete uBufferOut;
		delete shadowPipeline;

		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		vmaDestroyImage(device, shadowImage, allocation);
		vkDestroyImageView(device, shadowImageView, nullptr);
		vkFreeDescriptorSets(device, device.GetDescriptorPool(), inSets.size(), inSets.data());

	}

	std::vector<VkWriteDescriptorSet> ShadowMap::GetDescriptorWrites(uint32_t frame, VkDescriptorSet dstSet)
	{
		std::vector<VkWriteDescriptorSet> writes;
		writes.push_back(uBufferOut->CreateDescriptorWrite(frame, dstSet));

		imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		imageInfo.imageView = shadowImageView;
		imageInfo.sampler = shadowSampler;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = dstSet;
		descriptorWrite.dstBinding = imageLocation;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		writes.push_back(descriptorWrite);

		return writes;
	}

	void ShadowMap::BeginRenderPass(uint32_t frame, VkCommandBuffer buffer, glm::mat4 LightVP) {
		inUbo.LightVP = LightVP;
		uBufferIn.Update(frame, inUbo);

		VkClearValue clearValue{};
		clearValue.depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass = renderPass;
		beginInfo.framebuffer = framebuffer;
		beginInfo.renderArea.extent.width = SHADOW_MAP_WIDTH;
		beginInfo.renderArea.extent.height = SHADOW_MAP_HEIGHT;
		beginInfo.clearValueCount = 1;
		beginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(SHADOW_MAP_WIDTH);
		viewport.height = static_cast<float>(SHADOW_MAP_HEIGHT);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT };

		vkCmdSetScissor(buffer, 0, 1, &scissor);

		vkCmdSetDepthBias(buffer, bias, 0.0f, slopeBias);

		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline->GetPipeline());

		vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowPipeline->GetLayout(), 0, 1, &inSets[frame], 0, nullptr);
	}

	void ShadowMap::EndRenderPass(uint32_t frame, VkCommandBuffer buffer) {
		vkCmdEndRenderPass(buffer);

		uBufferOut->Update(frame, inUbo);
	}

	void ShadowMap::RenderShadowMap(VkCommandBuffer buffer, Model* model) {
		model->DrawShadow(buffer);	
	}

	void ShadowMap::CreateImageResources() {
		device.CreateImage(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &shadowImage, &allocation);

		VkImageViewCreateInfo depthStencilView{};
		depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		depthStencilView.format = VK_FORMAT_D32_SFLOAT;
		depthStencilView.subresourceRange = {};
		depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthStencilView.subresourceRange.baseMipLevel = 0;
		depthStencilView.subresourceRange.levelCount = 1;
		depthStencilView.subresourceRange.baseArrayLayer = 0;
		depthStencilView.subresourceRange.layerCount = 1;
		depthStencilView.image = shadowImage;
		if (vkCreateImageView(device, &depthStencilView, nullptr, &shadowImageView) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shadow image view!");

		VkSamplerCreateInfo sampler{};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_NEAREST;
		sampler.minFilter = VK_FILTER_NEAREST;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(device, &sampler, nullptr, &shadowSampler) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shadow texture sampler!");
	}

	void ShadowMap::CreateRenderPass() {
		VkAttachmentDescription attachmentDescription{};
		attachmentDescription.format = VK_FORMAT_D32_SFLOAT;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// Clear depth at beginning of the render pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// We will read from depth, so it's important to store the depth attachment results
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;					// We don't care about initial layout of the attachment
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

		VkAttachmentReference depthReference = {};
		depthReference.attachment = 0;
		depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;			// Attachment will be used as depth/stencil during render pass

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 0;													// No color attachments
		subpass.pDepthStencilAttachment = &depthReference;									// Reference to our depth attachment

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;

		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		VkRenderPassCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &attachmentDescription;
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpass;
		createInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		createInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shadow renderPass!");
	}

	void ShadowMap::CreateFramebuffer() {
		// Create frame buffer
		VkFramebufferCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &shadowImageView;
		createInfo.width = SHADOW_MAP_WIDTH;
		createInfo.height = SHADOW_MAP_HEIGHT;
		createInfo.layers = 1;

		if (vkCreateFramebuffer(device, &createInfo, nullptr, &framebuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to create shadow framebuffer!");
	}

	void ShadowMap::CreateUniforms() {
		std::vector<VkDescriptorSetLayoutBinding> bindings = { uBufferIn.GetLayoutBinding() };

		VkDescriptorSetLayoutCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		createInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &inLayout))
			throw std::runtime_error("Failed to create shadow descriptor set layout!");

		std::vector<VkDescriptorSetLayout> layouts(3, inLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = device.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(3);
		allocInfo.pSetLayouts = layouts.data();

		inSets.resize(3);
		if (vkAllocateDescriptorSets(device, &allocInfo, inSets.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate descriptor sets!");

		for (size_t i = 0; i < 3; i++) {
			std::vector<VkWriteDescriptorSet> descriptorWrites{ uBufferIn.CreateDescriptorWrite(i, inSets[i]) };

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
		}
	}

	void ShadowMap::CreatePipeline() {
		Pipeline::PipelineConfig config = Pipeline::FillPipelineConfig();

		config.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		config.dynamicStates.pop_back();
		config.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

		config.colorBlending.attachmentCount = 0;
		config.colorBlending.pAttachments = nullptr;

		config.rasterizer.depthBiasEnable = VK_TRUE;
		config.rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

		config.renderPass = renderPass;

		config.layouts.insert({ 0, inLayout });

		VkPushConstantRange pushConstant{};
		pushConstant.offset = 0;
		pushConstant.size = sizeof(glm::mat4);
		pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		config.pushConstant = &pushConstant;

		shadowPipeline = new Pipeline(device, config, "resources/shaders/spirv/shadowMapping.vert.spv", "");
	}

	void ShadowMap::CreateBindings(uint32_t binding, VkShaderStageFlags imageStage) {
		imageBinding.binding = binding;
		imageBinding.descriptorCount = 1;
		imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		imageBinding.pImmutableSamplers = nullptr;
		imageBinding.stageFlags = imageStage;
	}

}