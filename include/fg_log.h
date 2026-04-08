#pragma once
#include <string>

namespace slogger = SKSE::log;

class FGLogger { public:
    inline static constexpr std::string_view prefix{"[FGTweak] "};

    static void logger_init();

    // usage:  logger.info("bla = {}", bla);  like std::format
    template<typename... Args>
    void info(std::string_view fmt, Args&&... args)
    {
        auto* console = RE::ConsoleLog::GetSingleton();
        if constexpr (sizeof...(args) == 0) {
            if (console) console->Print("%.*s%.*s", static_cast<int>(prefix.size()), prefix.data(), static_cast<int>(fmt.size()), fmt.data());
            slogger::info("{}", fmt);
        } else {
            std::string message = std::vformat(fmt, std::make_format_args(args...));
            if (console) console->Print("%.*s%s", static_cast<int>(prefix.size()), prefix.data(), message.c_str());
            slogger::info("{}", message);
        }
    }
};
