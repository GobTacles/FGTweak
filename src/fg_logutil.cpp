// log utils

#include "PCH.h"
#include "fg_logutil.h" // myprint, myprint_init

#include <spdlog/sinks/basic_file_sink.h>

void myprint_init() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
    
    slogger::info("SetupLog logsFolder = {}", (*logsFolder).string());
    //slogger::info("SetupLog pluginName = {}", pluginName);
    //slogger::info("SetupLog logFilePath = {}", logFilePath.string());
}
