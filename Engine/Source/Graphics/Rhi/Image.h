#pragma once
#include <filesystem>

namespace SnowEngine
{
	class Image
	{
	public:
		static std::shared_ptr<Image> Create(const std::filesystem::path& source);
		virtual ~Image() = default;
	};
}
