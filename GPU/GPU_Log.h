#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <memory>

#if __has_include(<spdlog.h>)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#define ENABLE_GPU_LOG
#elif __has_include(<spdlog/spdlog.h>)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#define ENABLE_GPU_LOG
#endif

#ifdef ENABLE_GPU_LOG
class GPU_Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return m_logger; }

private:
	static std::shared_ptr<spdlog::logger> m_logger;
};

#define GPU_LOG_TRACE(...)    ::GPU_Log::GetLogger()->trace(__VA_ARGS__)
#define GPU_LOG_INFO(...)     ::GPU_Log::GetLogger()->info(__VA_ARGS__)
#define GPU_LOG_WARN(...)     ::GPU_Log::GetLogger()->warn(__VA_ARGS__)
#define GPU_LOG_ERROR(...)    ::GPU_Log::GetLogger()->error(__VA_ARGS__)

#define GPU_ASSERT(x, ...)		 { if (!x) { ::Editor::GPU_Log::GetLogger()->error("Assert error: {0}", __VA_ARGS__); __debugbreak(); } }

#else

#define GPU_LOG_TRACE(...)
#define GPU_LOG_INFO(...) 
#define GPU_LOG_WARN(...) 
#define GPU_LOG_ERROR(...)

#define GPU_ASSERT(x, ...)

#endif

#endif