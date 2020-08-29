#include "cpu.hpp"

CPU::CPU()
{
    regs.AF = 0x11B0;
    regs.BC = 0x0013;
    regs.DE = 0x00D8;
    regs.HL = 0x014D;

    regs.PC = 0x0100;
    regs.SP = 0xFFFE;
}