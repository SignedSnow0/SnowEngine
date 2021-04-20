#pragma once
#include "shader.h"

namespace SnowEngine {

    class Pipeline {
    public:
        //Structure containing all the options for the pipeline fixed stages
        struct PipelineConfig {
            //Descrive come unire i vertici in linee
            VkPipelineInputAssemblyStateCreateInfo  inputAssembly       {};
            //Descrive come un primitivo viene trasformato in un insieme di pixel
            VkPipelineRasterizationStateCreateInfo  rasterizer          {};
            //Descrive come colorare un pixel contando anche i suoi vicini
            VkPipelineMultisampleStateCreateInfo    multisampling       {};
            //Descrive un insime di operazioni possibili per modificare il colore di un pixel a seconda delle sue caratteristiche
            VkPipelineColorBlendStateCreateInfo     colorBlending       {};
            VkPipelineViewportStateCreateInfo       viewportInfo        {};
            VkPipelineColorBlendAttachmentState     colorBlendAttachment{};
            VkPipelineDepthStencilStateCreateInfo   depthStencil        {};
            
            VkRenderPass                            renderPass          = nullptr;
            //VkPipelineLayout                        pipelineLayout      = nullptr;
            uint32_t                                subpass             = 0;
            std::vector<VkDynamicState>             dynamicStates;
            std::map<int, VkDescriptorSetLayout>    layouts;
            VkPushConstantRange                     pushConstant;
        };
    public:
        Pipeline(Device& device, PipelineConfig& pipelineFixedLayout);
        ~Pipeline();

        inline VkPipelineLayout GetLayout()     { return pipelineLayout; }
        inline VkPipeline       GetPipeline()   { return pipeline; }

        //Fills the PipelineConfig struct
        static PipelineConfig FillPipelineConfig();

    private:
        void CreatePipeline(PipelineConfig& config);
        void CreatePipelineLayout(PipelineConfig& config);
        
    private:
        Device& device;

        VkPipelineLayout    pipelineLayout;
        PipelineConfig      config;
        VkPipeline          pipeline;

        Shader  vertexShader    { device, "C:\\dev\\SnowEngine\\Engine\\resources\\shaders\\spirv\\shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT };
        Shader  fragmentShader  { device, "C:\\dev\\SnowEngine\\Engine\\resources\\shaders\\spirv\\shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT };       
    };
}