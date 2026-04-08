#pragma once

inline std::string str (RE::NiPoint3& o) { return std::format("({},{},{})",o.x,o.y,o.z); }
inline std::string str (RE::NiTransform& o) { return std::format("(t={},...)",str(o.translate)); }
inline std::string str (RE::NiBound& o) { return std::format("({},r={})",str(o.center),o.radius); }
