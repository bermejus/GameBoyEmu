#pragma once

#include <types.hpp>
#include "../utils/task.hpp"

struct CPU;

struct Opcode
{
    const char* disassembly;
    const uint fetch_length;
    const uint cycles;
    Task (CPU::*op)();
};

struct CbOpcode
{
	const char* disassembly;
	const uint cycles;
	Task (CPU::*op)();
};