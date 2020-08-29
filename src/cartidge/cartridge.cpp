#include "cartridge.hpp"

Cartridge::Cartridge(const std::string& path)
{

}

bool Cartridge::accepts(const ushort addr) const
{
    return addr < 0x8000 || (addr >= 0xA000 && addr < 0xC000);
}

ubyte Cartridge::read(const ushort addr) const
{
    
}

void Cartridge::write(const ushort addr, const ubyte value)
{

}