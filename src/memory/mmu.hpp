#pragma once

#include "memory_unit.hpp"
#include <vector>

class MMU
{
private:
    std::vector<MemoryUnit*> memory_map;

public:
    MMU();
    void load(MemoryUnit* mem_unit);

    ubyte read(const ushort address) const;
    void write(const ushort address, const ubyte value);
};