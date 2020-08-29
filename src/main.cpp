#include <iostream>

#include "cpu/cpu.hpp"

int main()
{
    CPU cpu;

    cpu.regs.ZF = 0x1;
    std::printf("0x%X\n", cpu.regs.AF);

    return 0;
}