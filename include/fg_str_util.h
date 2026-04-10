#pragma once
#include <string>
#include <string_view>
#include <format>

inline std::string str (const RE::NiPoint3&         o) { return std::format("({},{},{})",o.x,o.y,o.z); }
inline std::string str (const RE::NiTransform&      o) { return std::format("(t={},...)",str(o.translate)); }
inline std::string str (const RE::NiBound&          o) { return std::format("({},r={})",str(o.center),o.radius); }
inline std::string str (const RE::BSFixedString&    o) { return std::format("{}",std::string_view{o}); }

std::string str_lower (std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}
