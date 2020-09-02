#include "mmu.hpp"

MMU::MMU() {}

void MMU::load(MemoryUnit* mem_unit)
{
    memory_map.push_back(mem_unit);
}

ubyte MMU::read(const ushort address) const
{
    for (const auto unit : memory_map)
        if (unit->accepts(address))
            return unit->read(address);
    return 0xFF;
}

void MMU::write(const ushort address, const ubyte value)
{
    for (const auto unit : memory_map)
        if (unit->accepts(address))
            unit->write(address, value);
}