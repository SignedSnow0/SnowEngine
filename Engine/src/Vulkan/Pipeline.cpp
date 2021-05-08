#include "Pipeline.h"
#include <stdexcept>
#include <array>

#include "Vulkan/Buffers/Buffer.hpp"

namespace SnowEngine {

    Pipeline::Pipeline(Device& device, PipelineConfig& config, std::string vertexPath, std::string fragmentPath) : device(device), config(config) {
        if (vertexPath != "")
            vertexShader = new Shader(device, vertexPath, VK_SHADER_STAGE_VERTEX_BIT);
        if (fragmentPath != "")
            fragmentShader = new Shader(device, fragmentPath, VK_SHADER_STAGE_FRAGMENT_BIT);
        CreatePipelineLayout(config);
        CreatePipeline(config);
    }

    Pipeline::~Pipeline() {
        delete vertexShader;
        delete fragmentShader;

        vkDestroyPipeline(device, pipeline, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
    
    void Pipeline::CreatePipeline(PipelineConfig& config) {
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
        if (vertexShader != nullptr)
            shaderStages.push_back(vertexShader->GetShaderStage());
        if (fragmentShader != nullptr)
            shaderStages.push_back(fragmentShader->GetShaderStage());

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(config.vAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = config.vAttributes.data();

        auto descs = VertexBuffer::GetBindingDescription();
        auto vertexAttributeDescriptions = VertexBuffer::GetAttributeDescriptions();
        if (config.vBindings.has_value()) {       
            vertexInputInfo.pVertexBindingDescriptions = &config.vBindings.value();
        }
        else {
            vertexInputInfo.pVertexBindingDescriptions = &descs;
        }
        if (config.vAttributes.size() == 0) {          
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
			vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
        }

        //Indica delle possibili parti della pipeline che possono essere ricostruiti senza ricreare l`intera pipeline
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.flags              = 0;
        dynamicState.dynamicStateCount  = static_cast<uint32_t>(config.dynamicStates.size());
        dynamicState.pDynamicStates     = config.dynamicStates.data();
        
        config.colorBlending.pAttachments = &config.colorBlendAttachment;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = shaderStages.size();
        pipelineInfo.pStages                = shaderStages.data();
        pipelineInfo.pVertexInputState      = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState    = &config.inputAssembly;
        pipelineInfo.pViewportState         = &config.viewportInfo;
        pipelineInfo.pRasterizationState    = &config.rasterizer;
        pipelineInfo.pMultisampleState      = &config.multisampling;
        pipelineInfo.pDepthStencilState     = &config.depthStencil; // Optional
        pipelineInfo.pColorBlendState       = &config.colorBlending;
        pipelineInfo.pDynamicState          = &dynamicState;
        pipelineInfo.layout                 = pipelineLayout;
        pipelineInfo.renderPass             = config.renderPass;
        pipelineInfo.subpass                = config.subpass;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex      = -1; // Optional

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline!");
    }

    void Pipeline::CreatePipelineLayout(PipelineConfig& config) {
        
        std::vector<VkDescriptorSetLayout> layouts;
        for (auto layout : config.layouts)
            layouts.push_back(layout.second);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount           = static_cast<uint32_t>(layouts.size());
        pipelineLayoutInfo.pSetLayouts              = layouts.data();
        pipelineLayoutInfo.pushConstantRangeCount   = config.pushConstant ? 1 : 0;
        pipelineLayoutInfo.pPushConstantRanges      = config.pushConstant ? config.pushConstant : nullptr;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    Pipeline::PipelineConfig Pipeline::FillPipelineConfig() {
        PipelineConfig pc{};
        
        pc.inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pc.inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pc.inputAssembly.primitiveRestartEnable = VK_FALSE; //permette di usare un valore speciale 0xFFFF per dividere diversi triangoli nel buffer se vk_true

        pc.viewportInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pc.viewportInfo.viewportCount   = 1;
        pc.viewportInfo.pViewports      = nullptr;
        pc.viewportInfo.scissorCount    = 1;
        pc.viewportInfo.pScissors       = nullptr;

        pc.rasterizer.sType                     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pc.rasterizer.depthClampEnable          = VK_FALSE; //se un fragment è oltre il limite del far plane viene eliminato invece di essere clippato
        pc.rasterizer.rasterizerDiscardEnable   = VK_FALSE; //usato se non sere output sul display
        pc.rasterizer.polygonMode               = VK_POLYGON_MODE_FILL; //metodo di riempimento di un poligono
        pc.rasterizer.lineWidth                 = 1.0f;
        pc.rasterizer.cullMode                  = VK_CULL_MODE_BACK_BIT;
        pc.rasterizer.frontFace                 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pc.rasterizer.depthBiasEnable           = VK_FALSE; //modifica ogni valore per una costante
        pc.rasterizer.depthBiasConstantFactor   = 0.0f; // Optional
        pc.rasterizer.depthBiasClamp            = 0.0f; // Optional
        pc.rasterizer.depthBiasSlopeFactor      = 0.0f;// Optional

        pc.multisampling.sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pc.multisampling.sampleShadingEnable    = VK_FALSE;
        pc.multisampling.rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT;
        pc.multisampling.minSampleShading       = 1.0f; // Optional
        pc.multisampling.pSampleMask            = nullptr; // Optional
        pc.multisampling.alphaToCoverageEnable  = VK_FALSE; // Optional
        pc.multisampling.alphaToOneEnable       = VK_FALSE; // Optional

        //usato per modificare il colore di un pixel in base a un suo componente
        pc.colorBlendAttachment.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        pc.colorBlendAttachment.blendEnable            = VK_FALSE;
        pc.colorBlendAttachment.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        pc.colorBlendAttachment.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        pc.colorBlendAttachment.colorBlendOp           = VK_BLEND_OP_ADD; // Optional
        pc.colorBlendAttachment.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        pc.colorBlendAttachment.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        pc.colorBlendAttachment.alphaBlendOp           = VK_BLEND_OP_ADD; // Optional

        pc.colorBlending.sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pc.colorBlending.logicOpEnable      = VK_FALSE;
        pc.colorBlending.logicOp            = VK_LOGIC_OP_COPY; // Optional
        pc.colorBlending.attachmentCount    = 1;
        pc.colorBlending.blendConstants[0]  = 0.0f; // Optional
        pc.colorBlending.blendConstants[1]  = 0.0f; // Optional
        pc.colorBlending.blendConstants[2]  = 0.0f; // Optional
        pc.colorBlending.blendConstants[3]  = 0.0f; // Optional
        //pAttachments è nella creazione perchè la struttura è passata per valore

        pc.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		pc.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pc.dynamicStates.push_back(VK_DYNAMIC_STATE_FRONT_FACE_EXT);

        pc.depthStencil.sType                   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pc.depthStencil.depthTestEnable         = VK_TRUE;
        pc.depthStencil.depthWriteEnable        = VK_TRUE;
        pc.depthStencil.depthCompareOp          = VK_COMPARE_OP_LESS;
        pc.depthStencil.depthBoundsTestEnable   = VK_FALSE;
        pc.depthStencil.minDepthBounds          = 0.0f; // Optional
        pc.depthStencil.maxDepthBounds          = 1.0f; // Optional
        pc.depthStencil.stencilTestEnable       = VK_FALSE;
        pc.depthStencil.front                   = {}; // Optional
        pc.depthStencil.back                    = {}; // Optional
        
        return pc;
    }
}