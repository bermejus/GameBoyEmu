#pragma once

#include "opcode.hpp"
#include "registers.hpp"

#define load(x) \
    void CPU::load_##x() { \
        regs.x = 0; \
    }

struct CPU
{
    Registers regs;

    CPU();

private:
    void nop();
    void halt();


private:
    const Opcode opcode_table[256];
};