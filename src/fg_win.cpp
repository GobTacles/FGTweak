#include "PCH.h"
#include "PCH_win.h"
#include "fg_win.h"

std::optional<fg_memory_info> win_get_memory_info()
{
    PERFORMANCE_INFORMATION pi{};
    if (!GetPerformanceInfo(&pi, sizeof(pi)))
        return std::nullopt; // couldn't query

    fg_memory_info res;
    res.total_virtual_memory    = pi.CommitLimit * pi.PageSize;     // total virtual memory
    res.physical_memory         = pi.PhysicalTotal * pi.PageSize;   // installed RAM
    res.page_file_size          = res.total_virtual_memory - res.physical_memory;   // actual pagefile size
    return res;
}

// GameOverlayRenderer64.dll , GameOverlayRenderer.dll
std::optional<std::set<std::string>> win_list_processes()
{
    HMODULE modules[1024];
    DWORD needed;
    if (!EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed)) return std::nullopt;

    size_t count = needed / sizeof(HMODULE);
    std::set<std::string> res;
    for (size_t i = 0; i < count; i++) {
        char name[MAX_PATH];
        if (GetModuleFileNameA(modules[i], name, MAX_PATH)) res.emplace(name);
    }
    return res;
}
