#include <iostream>

#include "cpu/cpu.hpp"
#include "memory/mmu.hpp"
#include "utils/task.hpp"
#include "cpu/interrupts.hpp"

Task foo()
{
    co_await std::experimental::suspend_always{};
}

int main()
{
    foo();
    MMU mmu;
    Interrupts irq;
    CPU cpu(mmu, irq);

    cpu.regs.ZF = 0x1;
    std::printf("0x%X\n", cpu.regs.AF);

    return 0;
}