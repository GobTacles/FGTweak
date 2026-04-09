// precompiled header, add the big things here that rarely change to speed up compile
#pragma once
#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

#include <optional>
#include <functional>
#include <string>
#include <format>
#include <string_view>

using namespace std::literals; // needed for commonlib build/.../__FGTweakPlugin.cpp ".."sv (std::string_view literal)
