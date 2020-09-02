#include "boot.hpp"

Boot::Boot() : done(false) {}

void Boot::finished()
{
    done = true;
}

bool Boot::accepts(const ushort address) const
{
    return !done && address < 0x900;
}

ubyte Boot::read(const ushort address) const
{
    return boot_data[address];
}

void Boot::write(const ushort address, const ubyte value) {}