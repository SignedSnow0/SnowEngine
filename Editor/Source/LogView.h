#pragma once
#include <SnowEngine.h>

namespace SnowEditor
{
	class LogView
	{
	public:
		LogView();

		void Draw();

	private:
		static std::string GetSeverityString(SnowEngine::LogSeverity severity);
		static ImVec4 GetSeverityColor(SnowEngine::LogSeverity severity);

		std::string mSearch;
		b8 mShowTrace = true;
		b8 mShowWarning = true;
		b8 mShowError = true;
		b8 mShowDebug = true;
	};
}