#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

std::shared_ptr<spdlog::logger> Log::m_logger;

void Log::Init()
{
	spdlog::set_pattern("%^[%T] : %v%$");
	m_logger = spdlog::stdout_color_mt("Ed");
	m_logger->set_level(spdlog::level::trace);
}
