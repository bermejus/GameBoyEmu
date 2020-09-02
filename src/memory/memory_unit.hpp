#pragma once

#include <types.hpp>

class MemoryUnit
{
public:
    virtual bool accepts(const ushort addr) const = 0;
    virtual ubyte read(const ushort addr) const = 0;
    virtual void write(const ushort addr, const ubyte value) = 0;
};