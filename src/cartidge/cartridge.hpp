#pragma once

#include <types.hpp>
#include <string>
#include <memory/io.hpp>
#include <vector>
#include <array>

enum CartridgeType
{
    ROM_ONLY                    = 0x00,
    MBC1                        = 0x01,
    MBC1_RAM                    = 0x02,
    MBC1_RAM_BATT               = 0x03,
    MBC2                        = 0x05,
    MBC2_BATT                   = 0x06,
    ROM_RAM                     = 0x08,
    ROM_RAM_BATT                = 0x09,
    MMM01                       = 0x0B,
    MMM01_RAM                   = 0x0C,
    MMM01_RAM_BATT              = 0x0D,
    MBC3_TIMER_BATT             = 0x0F,
    MBC3_TIMER_RAM_BATT         = 0x10,
    MBC3                        = 0x11,
    MBC3_RAM                    = 0x12,
    MBC3_RAM_BATT               = 0x13,
    MBC5                        = 0x19,
    MBC5_RAM                    = 0x1A,
    MBC5_RAM_BATT               = 0x1B,
    MBC5_RUMBLE                 = 0x1C,
    MBC5_RUMBLE_RAM             = 0x1D,
    MBC5_RUMBLE_RAM_BATT        = 0x1E,
    MBC6                        = 0x20,
    MBC7_SENSOR_RUMBLE_RAM_BATT = 0x22,
    POCKET_CAMERA               = 0xFC,
    BANDAI_TAMA5                = 0xFD,
    HuC3                        = 0xFE,
    HuC1_RAM_BATT               = 0xFF
};

class Cartridge : IO
{
private:
    std::vector<std::array<ubyte, 0x4000>> rom;

public:
    Cartridge(const std::string& path);

    bool accepts(const ushort addr) const override;
    ubyte read(const ushort addr) const override;
    void write(const ushort addr, const ubyte value) override;
};