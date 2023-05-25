#pragma once
#include <filesystem>
#include <map>

#include "Core/Types.h"

namespace SnowEngine
{
	enum class ShaderType
	{
		Vertex,
		Fragment,
		Compute
	};

	using shaderSource = std::tuple<std::filesystem::path, ShaderType>;

	struct GraphicShaderSource
	{
		shaderSource Vertex;
		shaderSource Fragment;

		std::vector<shaderSource> Others;
	};

	struct ComputeShaderSource
	{
		shaderSource Comp;
	};

	class Shader
	{
	public:
		static std::shared_ptr<Shader> Create(const GraphicShaderSource& source, const std::string& name);
		static std::shared_ptr<Shader> Create(const ComputeShaderSource& source, const std::string& name);

		static b8 GetShader(const std::string& name, std::shared_ptr<Shader>& shader);

	private:
		static std::map<std::string, std::shared_ptr<Shader>> sShaders;
	};
}
