#include "Shader.h"
#include "Graphics/Vulkan/VkShader.h"

namespace SnowEngine
{
	std::map<std::string, std::shared_ptr<Shader>> Shader::sShaders;

	std::shared_ptr<Shader> Shader::Create(const GraphicShaderSource& source, const std::string& name)
	{
		sShaders[name] = std::make_shared<VkShader>(source);
		return sShaders[name];
	}

	std::shared_ptr<Shader> Shader::Create(const ComputeShaderSource& source, const std::string& name)
	{
		sShaders[name] = std::make_shared<VkShader>(source);
		return sShaders[name];
	}

	b8 Shader::GetShader(const std::string& name, std::shared_ptr<Shader>& shader)
	{
		if (sShaders.contains(name))
		{
			shader = sShaders[name];
			return true;
		}
		return false;
	}
}
