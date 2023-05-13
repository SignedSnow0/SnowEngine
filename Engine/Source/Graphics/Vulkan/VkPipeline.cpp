#include "VkPipeline.h"

#include "VkCore.h"
#include "VkBuffers.h"
#include "VkDescriptorSet.h"
#include "VkCommandBuffer.h"

namespace SnowEngine
{
	VkPipeline::VkPipeline(std::shared_ptr<const VkShader> shader, std::shared_ptr<const VkRenderPass> renderPass, const u32 width, const u32 height)
		: mShader{ std::move(shader) }, mRenderPass{ std::move(renderPass) }
	{
		CreateLayout();
		CreateFixedFunctions(width, height);
	}

	void VkPipeline::Bind(const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline);
	}

	void VkPipeline::BindDescriptorSet(const DescriptorSet* set, const u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		const auto vkDescriptorSet{ reinterpret_cast<const VkDescriptorSet*>(set) };
		vkCmd->CurrentBuffer().bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mLayout, vkDescriptorSet->SetIndex(), vkDescriptorSet->Sets().at(currentFrame), nullptr);
	}

	void VkPipeline::CreateLayout()
	{
		std::vector<vk::DescriptorSetLayout> layouts{};
		layouts.reserve(mShader->Layouts().size());
		for (const auto& [set, layout] : mShader->Layouts())
			layouts.emplace_back(layout.CreateLayout());

		vk::PipelineLayoutCreateInfo createInfo;
		createInfo.setLayoutCount = static_cast<u32>(layouts.size());
		createInfo.pSetLayouts = layouts.data();
		createInfo.pushConstantRangeCount = 0;
		createInfo.pPushConstantRanges = nullptr;

		mLayout = VkCore::Get()->Device().createPipelineLayout(createInfo);
	}

	void VkPipeline::CreateFixedFunctions(const u32 width, const u32 height)
	{
		const std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

		vk::PipelineDynamicStateCreateInfo dynamicStateInfo;
		dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicStateInfo.pDynamicStates = dynamicStates.data();

		const auto bindingDescription{ VkVertexBuffer::BindingDescription() };
		const auto attributeDescriptions{ VkVertexBuffer::AttributeDescriptions() };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyInfo.primitiveRestartEnable = false;

		vk::Viewport viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<f32>(width);
		viewport.height = static_cast<f32>(height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vk::Rect2D scissor;
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = vk::Extent2D{ width, height };

		vk::PipelineViewportStateCreateInfo viewportInfo;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
		rasterizerInfo.depthClampEnable = false;
		rasterizerInfo.rasterizerDiscardEnable = false;
		rasterizerInfo.polygonMode = vk::PolygonMode::eFill;
		rasterizerInfo.lineWidth = 1.0f;
		rasterizerInfo.cullMode = vk::CullModeFlagBits::eBack;
		rasterizerInfo.frontFace = vk::FrontFace::eClockwise;
		rasterizerInfo.depthBiasEnable = false;
		rasterizerInfo.depthBiasConstantFactor = 0.0f;
		rasterizerInfo.depthBiasClamp = 0.0f;
		rasterizerInfo.depthBiasSlopeFactor = 0.0f;

		vk::PipelineMultisampleStateCreateInfo multisampleInfo;
		multisampleInfo.sampleShadingEnable = false;
		multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.pSampleMask = nullptr;
		multisampleInfo.alphaToCoverageEnable = false;
		multisampleInfo.alphaToOneEnable = false;

		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		colorBlendAttachment.blendEnable = false;
		colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
		colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo colorBlendInfo;
		colorBlendInfo.logicOpEnable = false;
		colorBlendInfo.logicOp = vk::LogicOp::eCopy;
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &colorBlendAttachment;
		colorBlendInfo.blendConstants[0] = 0.0f;
		colorBlendInfo.blendConstants[1] = 0.0f;
		colorBlendInfo.blendConstants[2] = 0.0f;
		colorBlendInfo.blendConstants[3] = 0.0f;

		const auto shaderStages = mShader->ShaderStageInfos();

		vk::GraphicsPipelineCreateInfo createInfo;
		createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		createInfo.pStages = shaderStages.data();
		createInfo.pVertexInputState = &vertexInputInfo;
		createInfo.pInputAssemblyState = &inputAssemblyInfo;
		createInfo.pViewportState = &viewportInfo;
		createInfo.pRasterizationState = &rasterizerInfo;
		createInfo.pMultisampleState = &multisampleInfo;
		createInfo.pDepthStencilState = nullptr;
		createInfo.pColorBlendState = &colorBlendInfo;
		createInfo.pDynamicState = &dynamicStateInfo;
		createInfo.layout = mLayout;
		createInfo.renderPass = mRenderPass->RenderPass();
		createInfo.subpass = 0;
		createInfo.basePipelineHandle = nullptr;
		createInfo.basePipelineIndex = -1;

		mPipeline = VkCore::Get()->Device().createGraphicsPipeline(nullptr, createInfo).value;
	}

	VkComputePipeline::VkComputePipeline(std::shared_ptr<const VkShader> shader)
		: mShader{ std::move(shader) }
	{
		CreateLayout();
		CreatePipeline();
	}

	void VkComputePipeline::Dispatch(const u32 x, const u32 y, const u32 z, const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		vkCmd->CurrentBuffer().bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);
		vkCmd->CurrentBuffer().dispatch(x, y, z);
	}

	void VkComputePipeline::BindDescriptorSet(const DescriptorSet* set, const u32 currentFrame, const std::shared_ptr<CommandBuffer>& cmd) const
	{
		const auto& vkCmd = std::static_pointer_cast<VkCommandBuffer>(cmd);

		const auto vkDescriptorSet{ reinterpret_cast<const VkDescriptorSet*>(set) };
		vkCmd->CurrentBuffer().bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, vkDescriptorSet->SetIndex(), vkDescriptorSet->Sets().at(currentFrame), nullptr);
	}

	void VkComputePipeline::CreateLayout()
	{
		std::vector<vk::DescriptorSetLayout> layouts{};
		layouts.reserve(mShader->Layouts().size());
		for (const auto& [set, layout] : mShader->Layouts())
			layouts.emplace_back(layout.CreateLayout());

		vk::PipelineLayoutCreateInfo createInfo;
		createInfo.setLayoutCount = static_cast<u32>(layouts.size());
		createInfo.pSetLayouts = layouts.data();
		createInfo.pushConstantRangeCount = 0;
		createInfo.pPushConstantRanges = nullptr;

		mLayout = VkCore::Get()->Device().createPipelineLayout(createInfo);
	}

	void VkComputePipeline::CreatePipeline()
	{
		vk::ComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.layout = mLayout;
		pipelineInfo.stage = mShader->ShaderStageInfos().front();

		mPipeline = VkCore::Get()->Device().createComputePipeline(nullptr, pipelineInfo).value;
	}
}
