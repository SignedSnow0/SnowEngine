#include "shader.h"
#include <fstream>
#include <filesystem>

namespace SnowEngine
{
    Shader::Shader(Device& device, const std::string& path, VkShaderStageFlagBits shaderType) : device(device) {
        auto abs = std::filesystem::absolute(path);
        auto code = ReadFile(abs.string());
        CreateShaderModule(code);
        CreateShaderStage(shaderType);
    }

    Shader::~Shader() {
        vkDestroyShaderModule(device, shaderModule, nullptr);
    }

    void Shader::CreateShaderStage(VkShaderStageFlagBits shaderType) {
        shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStage.stage   = shaderType;
        shaderStage.module  = shaderModule;
        shaderStage.pName   = "main";
    }

    std::vector<char> Shader::ReadFile(const std::string& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("Failed to open file at " + path);

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    void Shader::CreateShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module!");
    }
}