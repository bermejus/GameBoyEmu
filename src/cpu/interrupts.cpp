#include "interrupts.hpp"

void Interrupts::processIrq(ubyte flags)
{
    switch (first_bit(flags))
    {
        case 0: vector = 0x0000; break;
        case 1: vector = 0x0040; break;
        case 2: vector = 0x0048; break;
        case 3: vector = 0x0050; break;
        case 4: vector = 0x0058; break;
        case 5: vector = 0x0060; break;
        default: break;
    }
}

ushort Interrupts::irqVector()
{
    switch (vector)
    {
    case 0x0000: break;
    case 0x0040: IF_VBLANK = 0; break;
    case 0x0048: IF_LCDC = 0; break;
    case 0x0050: IF_TIMER = 0; break;
    case 0x0058: IF_SERIAL = 0; break;
    case 0x0060: IF_INPUT = 0; break;
    default: break;
    }

    return vector;
}

bool Interrupts::accepts(const ushort address) const
{
    return (address == 0xFF0F) || (address == 0xFFFF);
}

void Interrupts::write(const ushort address, const ubyte value)
{
    if (address == 0xFF0F)
        IF = 0xE0 | (value & 0x1F);

    else
    {
        if (maybeModified)
            processIrq(IE & value & 0x1F);
        IE = value;
    }
}

ubyte Interrupts::read(const ushort address) const
{
    return (address == 0xFF0F) ? IF : IE;
}