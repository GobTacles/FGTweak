#pragma once
#include <string>

namespace slogger = SKSE::log;

void myprint_init();

// usage:  myprint("bla = {}", bla);  like std::format
template<typename... Args>
void myprint(std::string_view fmt, Args&&... args)
{
    auto* console = RE::ConsoleLog::GetSingleton();
    if constexpr (sizeof...(args) == 0) {
        if (console) console->Print("%.*s", static_cast<int>(fmt.size()), fmt.data());
        slogger::info("{}", fmt);
    } else {
        std::string message = std::vformat(fmt, std::make_format_args(args...));
        if (console) console->Print("%s", message.c_str());
        slogger::info("{}", message);
    }
}
