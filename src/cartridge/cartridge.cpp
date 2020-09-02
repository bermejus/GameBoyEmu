#include "cartridge.hpp"

#include "../utils/loader.hpp"
#include <cstring>

Cartridge::Cartridge(const std::string& path)
{
    std::vector<ubyte> file = read_file(path);

    ubyte buffer[16];
    std::memcpy(&buffer[0], &file[0x134], 16);
    title = std::string((char*)&buffer[0], 16);

    type = static_cast<CartridgeType>(file[0x147]);

    ubyte rom_size = file[0x148];
    ubyte n_banks = 2 << (rom_size & 0xF);
        if (rom_size >> 4) n_banks += 2 << (rom_size >> 4);
    rom.resize(n_banks);

    build_ram(file[0x149]);
}

bool Cartridge::accepts(const ushort addr) const
{
    return addr < 0x8000 || (addr >= 0xA000 && addr < 0xC000);
}

ubyte Cartridge::read(const ushort addr) const
{
    switch (type)
    {
        case ROM_ONLY:      return readRomOnly(addr);
        case MBC1:          return readMBC1(addr);
        default:            return 0xFF;
    }
}

void Cartridge::write(const ushort addr, const ubyte value)
{
    switch (type)
    {
        case ROM_ONLY:      writeRomOnly(addr, value);
        case MBC1:          writeMBC1(addr, value);
    }
}

ubyte Cartridge::readRomOnly(const ushort addr) const
{
    return (addr < 0x8000) ? rom[0][addr] : 0xFF;
}

ubyte Cartridge::readMBC1(const ushort addr) const { return 0xFF; }

void Cartridge::writeRomOnly(const ushort addr, const ubyte value) {}

void Cartridge::writeMBC1(const ushort addr, const ubyte value) {}

void Cartridge::build_ram(ubyte ram_size)
{
    switch (ram_size)
    {
        case 0x0: ram.resize(0); break;
        case 0x1: ram.resize(1); break;
        case 0x2: ram.resize(1); break;
        case 0x3: ram.resize(4); break;
        case 0x4: ram.resize(16); break;
        case 0x5: ram.resize(8); break;
    }
}