#pragma once
#include <string>
#include <string_view>
#include <format>

inline std::string str (const RE::NiPoint3&         o) { return std::format("({},{},{})",o.x,o.y,o.z); }
inline std::string str (const RE::NiTransform&      o) { return std::format("(t={},...)",str(o.translate)); }
inline std::string str (const RE::NiBound&          o) { return std::format("({},r={})",str(o.center),o.radius); }
inline std::string str (const RE::BSFixedString&    o) { return std::format("{}",std::string_view{o}); }
inline std::string str (const RE::FormID&           o) { return std::format("0x{:08X}", static_cast<std::uint32_t>(o)); } // std::uint32_t

std::string str_lower (std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

inline std::vector<std::string> str_split(std::string_view text, std::string_view delim = "\n")
{
    std::vector<std::string> parts;

    if (delim.empty()) {
        parts.emplace_back(text);
        return parts;
    }

    size_t start = 0;
    while (true) {
        const size_t end = text.find(delim, start);
        if (end == std::string_view::npos) {
            parts.emplace_back(text.substr(start));
            return parts;
        }

        parts.emplace_back(text.substr(start, end - start));
        start = end + delim.size();
    }
}
