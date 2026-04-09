#pragma once

class fg_memory_info { public:
    uint64_t total_virtual_memory;
    uint64_t physical_memory; // installed RAM
    uint64_t page_file_size;
};

std::optional<fg_memory_info> win_get_memory_info();
