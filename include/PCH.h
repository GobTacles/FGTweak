// precompiled header, add the big things here that rarely change to speed up compile
#pragma once
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <SimpleIni.h> // CSimpleIniA
#include <optional>
#include <functional>
#include <string>
#include <string_view>
#include <format>
#include <set>
#include <map>
#include <deque>
#include <cstdint> // uint64_t
#include <atomic>
#include <chrono>
#include <thread>

using namespace std::literals; // needed for commonlib build/.../__FGTweakPlugin.cpp ".."sv (std::string_view literal)
