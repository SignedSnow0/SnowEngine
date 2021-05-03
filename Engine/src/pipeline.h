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
            //Informazioni riguardo l`alrea di rendering nella finestra
            VkPipelineViewportStateCreateInfo       viewportInfo        {};
            VkPipelineColorBlendAttachmentState     colorBlendAttachment{};
            VkPipelineDepthStencilStateCreateInfo   depthStencil        {};
			uint32_t                                subpass = 0;
			std::vector<VkDynamicState>             dynamicStates;

            VkRenderPass                            renderPass          = nullptr;
            //Pair of destriptor layout and its order in shader bindings
            std::map<int, VkDescriptorSetLayout>    layouts;
            VkPushConstantRange*                    pushConstant = nullptr;
        };
    public:
        Pipeline(Device& device, PipelineConfig& pipelineFixedLayout, std::string vertexPath, std::string fragmentPath);
        ~Pipeline();

		inline operator VkPipeline() { return pipeline; }
		inline operator VkPipelineLayout() { return pipelineLayout; }

        inline VkPipelineLayout GetLayout()     { return pipelineLayout; }
        inline VkPipeline       GetPipeline()   { return pipeline; }
        inline PipelineConfig GetConfig() { return config; }
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

        Shader*  vertexShader = nullptr;
        Shader*  fragmentShader = nullptr;
    };
}