#include "pipeline.h"
#include <stdexcept>
#include <array>

#include "Buffers/Buffer.hpp"

namespace SnowEngine {

    Pipeline::Pipeline(Device& device, PipelineConfig& config) : device(device), config(config) {
        CreatePipelineLayout(config);
        CreatePipeline(config);
    }

    Pipeline::~Pipeline() {
        vkDestroyPipeline(device, pipeline, nullptr);

        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
    
    void Pipeline::CreatePipeline(PipelineConfig& config) {
        VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShader.GetShaderStage(), fragmentShader.GetShaderStage() };

        auto vertexBindingDescription       = VertexBuffer::GetBindingDescription();
        auto vertexAttributeDescriptions    = VertexBuffer::GetAttributeDescriptions();
        
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};     
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount    = 1;
        vertexInputInfo.pVertexBindingDescriptions       = &vertexBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount  = static_cast<uint32_t>(vertexAttributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions     = vertexAttributeDescriptions.data();
        
        //Indica delle possibili parti della pipeline che possono essere ricostruiti senza ricreare l`intera pipeline
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.flags              = 0;
        dynamicState.dynamicStateCount  = static_cast<uint32_t>(config.dynamicStates.size());
        dynamicState.pDynamicStates     = config.dynamicStates.data();
        
        config.colorBlending.pAttachments = &config.colorBlendAttachment;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = 2;
        pipelineInfo.pStages                = shaderStages;
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
        pipelineLayoutInfo.pushConstantRangeCount   = 1;
        pipelineLayoutInfo.pPushConstantRanges      = &config.pushConstant;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
    }

    Pipeline::PipelineConfig Pipeline::FillPipelineConfig() {
        PipelineConfig ff{};
        
        ff.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ff.inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        ff.inputAssembly.primitiveRestartEnable = VK_FALSE; //permette di usare un valore speciale 0xFFFF per dividere diversi triangoli nel buffer se vk_true

        ff.viewportInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        ff.viewportInfo.viewportCount   = 1;
        ff.viewportInfo.pViewports      = nullptr;
        ff.viewportInfo.scissorCount    = 1;
        ff.viewportInfo.pScissors       = nullptr;

        ff.rasterizer.sType                     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        ff.rasterizer.depthClampEnable          = VK_FALSE; //se un fragment è oltre il limite del far plane viene eliminato invece di essere clippato
        ff.rasterizer.rasterizerDiscardEnable   = VK_FALSE; //usato se non sere output sul display
        ff.rasterizer.polygonMode               = VK_POLYGON_MODE_FILL; //metodo di riempimento di un poligono
        ff.rasterizer.lineWidth                 = 1.0f;
        ff.rasterizer.cullMode                  = VK_CULL_MODE_BACK_BIT;
        ff.rasterizer.frontFace                 = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        ff.rasterizer.depthBiasEnable           = VK_FALSE; //modifica ogni valore per una costante
        ff.rasterizer.depthBiasConstantFactor   = 0.0f; // Optional
        ff.rasterizer.depthBiasClamp            = 0.0f; // Optional
        ff.rasterizer.depthBiasSlopeFactor      = 0.0f;// Optional

        ff.multisampling.sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ff.multisampling.sampleShadingEnable    = VK_FALSE;
        ff.multisampling.rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT;
        ff.multisampling.minSampleShading       = 1.0f; // Optional
        ff.multisampling.pSampleMask            = nullptr; // Optional
        ff.multisampling.alphaToCoverageEnable  = VK_FALSE; // Optional
        ff.multisampling.alphaToOneEnable       = VK_FALSE; // Optional

        //usato per modificare il colore di un pixel in base a un suo componente
        ff.colorBlendAttachment.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        ff.colorBlendAttachment.blendEnable            = VK_FALSE;
        ff.colorBlendAttachment.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        ff.colorBlendAttachment.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        ff.colorBlendAttachment.colorBlendOp           = VK_BLEND_OP_ADD; // Optional
        ff.colorBlendAttachment.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        ff.colorBlendAttachment.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        ff.colorBlendAttachment.alphaBlendOp           = VK_BLEND_OP_ADD; // Optional

        ff.colorBlending.sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        ff.colorBlending.logicOpEnable      = VK_FALSE;
        ff.colorBlending.logicOp            = VK_LOGIC_OP_COPY; // Optional
        ff.colorBlending.attachmentCount    = 1;
        ff.colorBlending.blendConstants[0]  = 0.0f; // Optional
        ff.colorBlending.blendConstants[1]  = 0.0f; // Optional
        ff.colorBlending.blendConstants[2]  = 0.0f; // Optional
        ff.colorBlending.blendConstants[3]  = 0.0f; // Optional
        
        ff.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
        ff.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);


        ff.depthStencil.sType                   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ff.depthStencil.depthTestEnable         = VK_TRUE;
        ff.depthStencil.depthWriteEnable        = VK_TRUE;
        ff.depthStencil.depthCompareOp          = VK_COMPARE_OP_LESS;
        ff.depthStencil.depthBoundsTestEnable   = VK_FALSE;
        ff.depthStencil.minDepthBounds          = 0.0f; // Optional
        ff.depthStencil.maxDepthBounds          = 1.0f; // Optional
        ff.depthStencil.stencilTestEnable       = VK_FALSE;
        ff.depthStencil.front                   = {}; // Optional
        ff.depthStencil.back                    = {}; // Optional
        
        return ff;
    }
}