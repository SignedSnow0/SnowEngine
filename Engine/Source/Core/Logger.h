#pragma once
#include <format>
#include <queue>
#include <string>

#include "Types.h"

namespace SnowEngine
{
	enum class LogSeverity
	{
		Trace,
		Warning,
		Error,
		Debug
	};

	struct LogMessage
	{
		std::string Message;

		LogSeverity Severity;

		std::string File;
		u32 Line;

		u32 RepeatCount = 1;
	};

	class Logger
	{
	public:
		template<typename... Args>
		static void Log(const std::string& message, const LogSeverity severity, const std::string& file, const u32 line, Args&& ... args)
		{
			LogMessage msg;
			const u32 size = std::snprintf(nullptr, 0, message.c_str(), std::forward<Args>(args)...) + 1;
			msg.Message.resize(size);
			std::snprintf(msg.Message.data(), size, message.c_str(), std::forward<Args>(args)...);
			msg.Severity = severity;
			msg.File = file;
			msg.Line = line;

			if (!sMessages.empty() && sMessages.front().Message == msg.Message)
			{
				sMessages.front().RepeatCount++;
				return;
			}

			if (sMessages.size() == sMessageCount)
				sMessages.pop_back();

			sMessages.push_front(msg);
		}

		static const std::deque<LogMessage>& GetMessages() { return sMessages; }

	private:
		inline static std::deque<LogMessage> sMessages{};
		inline static u32 sMessageCount = 1024;
	};

#define LOG_TRACE(message, ...) ::SnowEngine::Logger::Log(message, ::SnowEngine::LogSeverity::Trace, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(message, ...) ::SnowEngine::Logger::Log(message, ::SnowEngine::LogSeverity::Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(message, ...) ::SnowEngine::Logger::Log(message, ::SnowEngine::LogSeverity::Error, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(message, ...) ::SnowEngine::Logger::Log(message, ::SnowEngine::LogSeverity::Debug, __FILE__, __LINE__, __VA_ARGS__)
}
