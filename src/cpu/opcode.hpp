#pragma once

#include <vector>

struct CPU;

typedef void (CPU::*Func)();

struct Opcode
{
    const char* disassembly;
    const int fetch_length;
    std::vector<Func> op;
};