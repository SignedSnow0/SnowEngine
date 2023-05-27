#pragma once
#include <filesystem>

namespace SnowEngine
{
	class Image
	{
	public:
		static std::shared_ptr<Image> Create(const std::filesystem::path& source);
		static std::shared_ptr<Image> Create(const std::array<std::filesystem::path, 6>& sources);
		virtual ~Image() = default;
	};
}
