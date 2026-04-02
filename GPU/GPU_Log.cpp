#include "GPU_Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> GPU_Log::m_logger;

void GPU_Log::Init()
{
	spdlog::set_pattern("%^[%T] : %v%$");
	m_logger = spdlog::stdout_color_mt("Ed");
	m_logger->set_level(spdlog::level::trace);
}
