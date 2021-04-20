#pragma once
#include <string>
#include <vector>

#include <vulkan\vulkan.h>

#include "device.h"

namespace SnowEngine {

    class Shader
    {
    public:
        Shader(Device& device, const std::string& path, VkShaderStageFlagBits shaderType);
        ~Shader();

        inline VkPipelineShaderStageCreateInfo GetShaderStage() { return shaderStage; }
    private:
        void                CreateShaderModule(const std::vector<char>& code);
        void                CreateShaderStage(VkShaderStageFlagBits shaderType);
        std::vector<char>   ReadFile(const std::string& path);
     

    private:
        Device& device;

        VkShaderModule                  shaderModule;
        VkPipelineShaderStageCreateInfo shaderStage{};
    };
}