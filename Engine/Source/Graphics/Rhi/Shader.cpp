#include "Shader.h"
#include "Graphics/Vulkan/VkShader.h"

namespace SnowEngine
{
	std::shared_ptr<Shader> Shader::Create(const GraphicShaderSource& source)
	{
		return std::make_shared<VkShader>(source);
	}

	std::shared_ptr<Shader> Shader::Create(const ComputeShaderSource& source)
	{
		return std::make_shared<VkShader>(source);
	}
}
