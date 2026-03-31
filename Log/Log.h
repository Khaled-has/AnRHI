#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return m_logger; }

private:
	static std::shared_ptr<spdlog::logger> m_logger;
};



#ifdef GPU_DEBUG

	#define GPU_LOG_TRACE(...)    ::Log::GetLogger()->trace(__VA_ARGS__)
	#define GPU_LOG_INFO(...)     ::Log::GetLogger()->info(__VA_ARGS__)
	#define GPU_LOG_WARN(...)     ::Log::GetLogger()->warn(__VA_ARGS__)
	#define GPU_LOG_ERROR(...)    ::Log::GetLogger()->error(__VA_ARGS__)

	#define GPU_ASSERT(x, ...)		 { if (!x) { ::Editor::Log::GetLogger()->error("Assert error: {0}", __VA_ARGS__); __debugbreak(); } }

#else

	#define GPU_LOG_TRACE(...)    
	#define GPU_LOG_INFO(...)    
	#define GPU_LOG_WARN(...)    
	#define GPU_LOG_ERROR(...)    
	
	#define GPU_ASSERT(x, ...)

#endif

#endif