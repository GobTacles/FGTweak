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
