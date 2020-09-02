#include "cpu.hpp"

#include <iostream>
using namespace std::experimental;

CPU::CPU(MMU& mmu, Interrupts& irq)
    : mmu(mmu), irq(irq), cyclesLeft(0), stopped(false), halted(false), haltBug(false)
{
    regs.AF = 0x11B0;
    regs.BC = 0x0013;
    regs.DE = 0x00D8;
    regs.HL = 0x014D;

    regs.PC = 0x0100;
    regs.SP = 0xFFFE;
}

void CPU::execute()
{
    if (!cyclesLeft) { // Check interrupts and fetch a new instruction
        if (irq.IME) {
            if (halted) {
                if (irq.IF & irq.IE & 0x1F)
                    halted = false;
                else
                    return;
            }

            if (ubyte fire_irq = irq.IF & irq.IE & 0x1F; fire_irq) {
				cyclesLeft = 5;

				irq.processIrq(fire_irq);
				instruction = interruptCallback();
			} else
				fetchOpcode();
        } else {
            if (halted) {
				if (irq.IF & irq.IE & 0x1F)
					halted = false;
				else
					return;
			}

			fetchOpcode();

			if (irq.delay) {
				irq.delay = false;
				irq.IME = true;
			}
        }
    }

    cyclesLeft--;
    instruction();
}

void CPU::fetchOpcode()
{
    static bool debug = false;

    const ubyte op_data = mmu.read(regs.PC);
    haltBug ? (haltBug = false) : regs.PC++;

    if (op_data == 0xCB)
    {
        const ubyte cb_data = mmu.read(regs.PC++);
        const CbOpcode& opcode = cbOpcodeTable[cb_data];
        instruction = (this->*(opcode.op))();
        cyclesLeft = opcode.cycles;

        if (debug) {
            std::printf("%s\n", opcode.disassembly);
			regs.PC -= 2;
			regs.print();
			regs.PC += 2;
        }
    } else {
        const Opcode& opcode = opcodeTable[op_data];
        instruction = (this->*(opcode.op))();
        cyclesLeft = opcode.cycles;

        if (debug) {
            switch (opcode.fetch_length) {
                case 0: std::printf("%s", opcode.disassembly); break;
                case 1: std::printf(opcode.disassembly, mmu.read(regs.PC)); break;
                case 2: std::printf(opcode.disassembly, (ushort)(mmu.read(regs.PC) | (mmu.read(regs.PC+1) << 8))); break;
            }

            std::printf("\n");
            regs.PC--;
            regs.print();
            regs.PC++;
        }
    }
}

Task CPU::interruptCallback()
{
    irq.IME = false;
	co_await suspend_always{};

	co_await suspend_always{};

	co_await suspend_always{};

	irq.maybeModified = true;
	writeHigh(regs.SP, regs.PC);
	irq.maybeModified = false;
	co_await suspend_always{};
	
	writeLow(regs.SP, regs.PC);
	regs.PC = irq.irqVector();
    co_return;
}

void CPU::readLow(ushort& dest)
{
    dest = mmu.read(regs.PC++);
}

void CPU::readHigh(ushort& dest)
{
    dest |= (mmu.read(regs.PC++) << 8);
}

void CPU::readLow(ushort& address, ushort& dest)
{
    dest = mmu.read(address++);
}

void CPU::readHigh(ushort& address, ushort& dest)
{
    dest |= (mmu.read(address++) << 8);
}

void CPU::writeLow(ushort& address, const ushort value)
{
    mmu.write(--address, value & 0xFF);
}

void CPU::writeHigh(ushort& address, const ushort value)
{
    mmu.write(--address, value >> 8);
}

#define LD_RR_NN(x) \
    co_await suspend_always{}; \
    readLow(regs.x); \
    co_await suspend_always{}; \
    readHigh(regs.x); \
    co_return;

#define LD_R_N(x) \
    co_await suspend_always{}; \
    regs.x = mmu.read(regs.PC++); \
    co_return;

#define LD_RRP_A(x) \
    co_await suspend_always{}; \
    mmu.write(regs.x, regs.A); \
    co_return;

#define LD_A_RRP(x) \
    co_await suspend_always{}; \
    regs.A = mmu.read(regs.x); \
    co_return;

#define LD_R_R(x, y) \
    regs.x = regs.y; \
    co_return;

#define LD_R_HLP(x) \
    co_await suspend_always{}; \
    regs.x = mmu.read(regs.HL); \
    co_return;

#define LD_HLP_R(x) \
    co_await suspend_always{}; \
    mmu.write(regs.HL, regs.x); \
    co_return;

#define INC_RR(x) \
    co_await suspend_always{}; \
    regs.x++; \
    co_return;

#define DEC_RR(x) \
    co_await suspend_always{}; \
    regs.x--; \
    co_return;

#define INC_R(x) \
    regs.NF = 0; \
    regs.HF = ((regs.x & 0xF)+1) >> 4; \
    regs.x++; \
    regs.ZF = regs.x == 0; \
    co_return;

#define DEC_R(x) \
    regs.NF = 1; \
    regs.HF = ((regs.x & 0xF)-1) >> 4; \
    regs.x--; \
    regs.ZF = regs.x == 0; \
    co_return;

#define ADD_HL_RR(x) \
    co_await suspend_always{}; \
	const uint sum = regs.HL + regs.x; \
	const uint info = sum ^ (regs.HL ^ regs.x); \
	regs.HL = sum & 0xFFFF; \
	regs.F = (regs.F & 0x80) | ((info & 0x10000) >> 12) | ((info & 0x1000) >> 7); \
    co_return;

#define ADD_A_N(x) \
    const uint sum = regs.A + x; \
    const uint info = sum ^ (regs.A ^ x); \
    regs.A = sum & 0xFF; \
	regs.F = ((!regs.A) << 7) | ((info & 0x100) >> 4) | ((info & 0x10) << 1); \
	co_return;

#define ADC_A_N(x) \
    const uint sum = regs.A + x + regs.CF; \
	const uint info = sum ^ (regs.A ^ x); \
	regs.A = sum & 0xFF; \
	regs.F = ((!regs.A) << 7) | ((info & 0x100) >> 4) | ((info & 0x10) << 1); \
	co_return;

#define SUB_A_N(x) \
	const uint res = regs.A - x; \
	const uint info = res ^ (regs.A ^ x); \
	regs.A = res & 0xFF; \
	regs.F = ((!regs.A) << 7) | ((info & 0x100) >> 4) | ((info & 0x10) << 1) | 0x40; \
	co_return;

#define SBC_A_N(x) \
	const uint res = regs.A - (x + regs.CF); \
	const uint info = res ^ (regs.A ^ x); \
	regs.A = res & 0xFF; \
	regs.F = ((!regs.A) << 7) | ((info & 0x100) >> 4) | ((info & 0x10) << 1) | 0x40; \
	co_return;

#define AND_N(x) \
	regs.F = (!(regs.A &= x) << 7) | 0x20; \
	co_return;

#define OR_N(x) \
	regs.F = !(regs.A |= x) << 7; \
	co_return;

#define XOR_N(x) \
	regs.F = !(regs.A ^= x) << 7; \
	co_return;

#define CP_N(x) \
	regs.F = 0x40; \
	if (regs.A == x) \
		regs.ZF = 1; \
	else { \
		if (x > regs.A) regs.CF = 1; \
		if ((x & 0xF) > (regs.A & 0xF)) regs.HF = 1; \
	} \
	co_return;

#define JR_N(cond) \
	co_await suspend_always{}; \
	if (cond) { \
		cyclesLeft += 1; \
		regs.PC += static_cast<byte>(mmu.read(regs.PC++)); \
		co_await suspend_always{}; \
	} else \
		regs.PC++; \
    co_return;

#define RET(cond) \
	co_await suspend_always{}; \
	if (cond) { \
		cyclesLeft += 3; \
		co_await suspend_always{}; \
		readLow(regs.SP, regs.PC); \
		co_await suspend_always{}; \
		readHigh(regs.SP, regs.PC); \
		co_await suspend_always{}; \
	} \
    co_return;

#define JP(cond) \
	co_await suspend_always{}; \
	ushort nn; \
	readLow(nn); \
	co_await suspend_always{}; \
	readHigh(nn); \
	if (cond) { \
		cyclesLeft += 1; \
		regs.PC = nn; \
		co_await suspend_always{}; \
	} \
    co_return;

#define CALL(cond) \
	co_await suspend_always{}; \
	ushort nn; \
	readLow(nn); \
	co_await suspend_always{}; \
	readHigh(nn); \
	if (cond) { \
		cyclesLeft += 3; \
		co_await suspend_always{}; \
		co_await suspend_always{}; \
		writeHigh(regs.SP, regs.PC); \
		co_await suspend_always{}; \
		writeLow(regs.SP, regs.PC); \
		regs.PC = nn; \
	} \
    co_return;

#define POP(x) \
	co_await suspend_always{}; \
	readLow(regs.SP, regs.x); \
	co_await suspend_always{}; \
	readHigh(regs.SP, regs.x); \
    co_return;

#define PUSH(x) \
	co_await suspend_always{}; \
	co_await suspend_always{}; \
	writeHigh(regs.SP, regs.x); \
	co_await suspend_always{}; \
	writeLow(regs.SP, regs.x); \
    co_return;

#define RST(addr) \
	co_await suspend_always{}; \
	co_await suspend_always{}; \
	writeHigh(regs.SP, regs.PC); \
	co_await suspend_always{}; \
	writeLow(regs.SP, regs.PC); \
	regs.PC = addr; \
    co_return;

Task CPU::undefined()
{
    std::cout << "[CPU]: Unimplemented instruction executed! -> ";
    std::printf("0x%01X, PC: 0x%04X\n", mmu.read(regs.PC-1), regs.PC-1);
    co_return;
}

// 0x0X
Task CPU::nop()
{
    co_return;
}

Task CPU::ld_bc_nn()
{
    LD_RR_NN(BC);
}

Task CPU::ld_bcp_a()
{
    LD_RRP_A(BC);
}

Task CPU::inc_bc()
{
    INC_RR(BC);
}

Task CPU::inc_b()
{
    INC_R(B);
}

Task CPU::dec_b()
{
    DEC_R(B);
}

Task CPU::ld_b_n()
{
    LD_R_N(B);
}

Task CPU::rlca()
{
    regs.F = (regs.A & 0x80) >> 3;
	regs.A = (regs.A << 1) | (regs.F >> 4);
	co_return;
}

Task CPU::ld_nnp_sp()
{
    co_await suspend_always{};

	ushort nn;
	readLow(nn);
	co_await suspend_always{};

	readHigh(nn);
	co_await suspend_always{};

	mmu.write(nn, regs.SP & 0xFF);
	co_await suspend_always{};

	mmu.write(nn + 1, regs.SP >> 8);
    co_return;
}

Task CPU::add_hl_bc()
{
    ADD_HL_RR(BC);
}

Task CPU::ld_a_bcp()
{
    LD_A_RRP(BC);
}

Task CPU::dec_bc()
{
    DEC_RR(BC);
}

Task CPU::inc_c()
{
    INC_R(C);
}

Task CPU::dec_c()
{
    DEC_R(C);
}

Task CPU::ld_c_n()
{
    LD_R_N(C);
}

Task CPU::rrca()
{
    regs.F = (regs.A & 0x1) << 4;
	regs.A = (regs.A >> 1) | (regs.F << 3);
	co_return;
}

// 0x1X
Task CPU::stop()
{
    stopped = true;
	co_return;
}

Task CPU::ld_de_nn()
{
    LD_RR_NN(DE);
}

Task CPU::ld_dep_a()
{
    LD_RRP_A(DE);
}

Task CPU::inc_de()
{
    INC_RR(DE);
}

Task CPU::inc_d()
{
    INC_R(D);
}

Task CPU::dec_d()
{
    DEC_R(D);
}

Task CPU::ld_d_n()
{
    LD_R_N(D);
}

Task CPU::rla()
{
    const ubyte carry = regs.CF;
	regs.F = (regs.A & 0x80) >> 3;
	regs.A = (regs.A << 1) | carry;
	co_return;
}

Task CPU::jr_n()
{
    co_await suspend_always{};
	co_await suspend_always{};
	regs.PC += static_cast<byte>(mmu.read(regs.PC++));
}

Task CPU::add_hl_de()
{
    ADD_HL_RR(DE);
}

Task CPU::ld_a_dep()
{
    LD_A_RRP(DE);
}

Task CPU::dec_de()
{
    DEC_RR(DE);
}

Task CPU::inc_e()
{
    INC_R(E);
}

Task CPU::dec_e()
{
    DEC_R(E);
}

Task CPU::ld_e_n()
{
    LD_R_N(E);
}

Task CPU::rra()
{
    const ubyte carry = regs.CF;
	regs.F = (regs.A & 0x1) << 4;
	regs.A = (regs.A >> 1) | (carry << 7);
	co_return;
}

// 0x2X
Task CPU::jr_nz_n()
{
    JR_N(!regs.ZF);
}

Task CPU::ld_hl_nn()
{
    LD_RR_NN(HL);
}

Task CPU::ld_hlip_a()
{
    LD_RRP_A(HL++);
}

Task CPU::inc_hl()
{
    INC_RR(HL);
}

Task CPU::inc_h()
{
    INC_R(H);
}

Task CPU::dec_h()
{
    DEC_R(H);
}

Task CPU::ld_h_n()
{
    LD_R_N(H);
}

Task CPU::daa()
{
    int s = regs.A;

	if (regs.NF) { 
		if (regs.HF) { s = (s - 0x6) & 0xFF; }
		if (regs.CF) s -= 0x60;
	} else {
		if (regs.HF || (s & 0xF) > 9) s += 0x6;
		if (regs.CF || s > 0x9F) s += 0x60;
	}

	regs.F &= ~(0x80 | 0x20);
	if (s & 0x100) regs.CF = 1;

	regs.A = s & 0xFF;
	if (!regs.A) regs.ZF = 1;

	co_return;
}

Task CPU::jr_z_n()
{
    JR_N(regs.ZF);
}

Task CPU::add_hl_hl()
{
    ADD_HL_RR(HL);
}

Task CPU::ld_a_hlip()
{
    LD_A_RRP(HL++);
}

Task CPU::dec_hl()
{
    DEC_RR(HL);
}

Task CPU::inc_l()
{
    INC_R(L);
}

Task CPU::dec_l()
{
    DEC_R(L);
}

Task CPU::ld_l_n()
{
    LD_R_N(L);
}

Task CPU::cpl()
{
    regs.A = ~regs.A;
	regs.F |= 0x60;
	co_return;
}

// 0x3X
Task CPU::jr_nc_n()
{
    JR_N(!regs.CF);
}

Task CPU::ld_sp_nn()
{
    LD_RR_NN(SP);
}

Task CPU::ld_hldp_a()
{
    LD_RRP_A(HL--);
}

Task CPU::inc_sp()
{
    INC_RR(SP);
}

Task CPU::inc_hlp()
{
    co_await suspend_always{};

	ubyte a = mmu.read(regs.HL);
	co_await suspend_always{};

	regs.NF = 0;
	regs.HF = ((a & 0xF)+1) >> 4;

	a++;
	regs.ZF = (a == 0);

	mmu.write(regs.HL, a);

    co_return;
}

Task CPU::dec_hlp()
{
    co_await suspend_always{};

	unsigned char a = mmu.read(regs.HL);
	co_await suspend_always{};

	regs.NF = 1;
	regs.HF = ((a & 0xF)-1) >> 4;

	a--;
	regs.ZF = (a == 0);
	
	mmu.write(regs.HL, a);
    co_return;
}

Task CPU::ld_hlp_n()
{
    co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	co_await suspend_always{};

	mmu.write(regs.HL, n);
    co_return;
}

Task CPU::scf()
{
    regs.F = (regs.F & 0x80) | 0x10;
	co_return;
}

Task CPU::jr_c_n()
{
    JR_N(regs.CF);
}

Task CPU::add_hl_sp()
{
    ADD_HL_RR(SP);
}

Task CPU::ld_a_hldp()
{
    LD_A_RRP(HL--);
}

Task CPU::dec_sp()
{
    DEC_RR(SP);
}

Task CPU::inc_a()
{
    INC_R(A);
}

Task CPU::dec_a()
{
    DEC_R(A);
}

Task CPU::ld_a_n()
{
    LD_R_N(A);
}

Task CPU::ccf()
{
    regs.F ^= 0x10;
	regs.F &= 0x90;
	co_return;
}

// 0x40
Task CPU::ld_b_b()
{
	co_return;
}

// 0x41
Task CPU::ld_b_c()
{
	LD_R_R(B, C);
}

// 0x42
Task CPU::ld_b_d()
{
	LD_R_R(B, D);
}

// 0x43
Task CPU::ld_b_e()
{
	LD_R_R(B, E);
}

// 0x44
Task CPU::ld_b_h()
{
	LD_R_R(B, H);
}

// 0x45
Task CPU::ld_b_l()
{
	LD_R_R(B, L);
}

// 0x46
Task CPU::ld_b_hlp()
{
	LD_R_HLP(B);
}

// 0x47
Task CPU::ld_b_a()
{
	LD_R_R(B, A);
}

// 0x48
Task CPU::ld_c_b()
{
	LD_R_R(C, B);
}

// 0x49
Task CPU::ld_c_c()
{
	co_return;
}

// 0x4A
Task CPU::ld_c_d()
{
	LD_R_R(C, D);
}

// 0x4B
Task CPU::ld_c_e()
{
	LD_R_R(C, E);
}

// 0x4C
Task CPU::ld_c_h()
{
	LD_R_R(C, H);
}

// 0x4D
Task CPU::ld_c_l()
{
	LD_R_R(C, L);
}

// 0x4E
Task CPU::ld_c_hlp()
{
	LD_R_HLP(C);
}

// 0x4F
Task CPU::ld_c_a()
{
	LD_R_R(C, A);
}

// 0x50
Task CPU::ld_d_b()
{
	LD_R_R(D, B);
}

// 0x51
Task CPU::ld_d_c()
{
	LD_R_R(D, C);
}

// 0x52
Task CPU::ld_d_d()
{
	co_return;
}

// 0x53
Task CPU::ld_d_e()
{
	LD_R_R(D, E);
}

// 0x54
Task CPU::ld_d_h()
{
	LD_R_R(D, H);
}

// 0x55
Task CPU::ld_d_l()
{
	LD_R_R(D, L);
}

// 0x56
Task CPU::ld_d_hlp()
{
	LD_R_HLP(D);
}

// 0x57
Task CPU::ld_d_a()
{
	LD_R_R(D, A);
}

// 0x58
Task CPU::ld_e_b()
{
	LD_R_R(E, B);
}

// 0x59
Task CPU::ld_e_c()
{
	LD_R_R(E, C);
}

// 0x5A
Task CPU::ld_e_d()
{
	LD_R_R(E, D);
}

// 0x5B
Task CPU::ld_e_e()
{
	co_return;
}

// 0x5C
Task CPU::ld_e_h()
{
	LD_R_R(E, H);
}

// 0x5D
Task CPU::ld_e_l()
{
	LD_R_R(E, L);
}

// 0x5E
Task CPU::ld_e_hlp()
{
	LD_R_HLP(E);
}

// 0x5F
Task CPU::ld_e_a()
{
	LD_R_R(E, A);
}

// 0x60
Task CPU::ld_h_b()
{
	LD_R_R(H, B);
}

// 0x61
Task CPU::ld_h_c()
{
	LD_R_R(H, C);
}

// 0x62
Task CPU::ld_h_d()
{
	LD_R_R(H, D);
}

// 0x63
Task CPU::ld_h_e()
{
	LD_R_R(H, E);
}

// 0x64
Task CPU::ld_h_h()
{
	co_return;
}

// 0x65
Task CPU::ld_h_l()
{
	LD_R_R(H, L);
}

// 0x66
Task CPU::ld_h_hlp()
{
	LD_R_HLP(H);
}

// 0x67
Task CPU::ld_h_a()
{
	LD_R_R(H, A);
}

// 0x68
Task CPU::ld_l_b()
{
	LD_R_R(L, B);
}

// 0x69
Task CPU::ld_l_c()
{
	LD_R_R(L, C);
}

// 0x6A
Task CPU::ld_l_d()
{
	LD_R_R(L, D);
}

// 0x6B
Task CPU::ld_l_e()
{
	LD_R_R(L, E);
}

// 0x6C
Task CPU::ld_l_h()
{
	LD_R_R(L, H);
}

// 0x6D
Task CPU::ld_l_l()
{
	co_return;
}

// 0x6E
Task CPU::ld_l_hlp()
{
	LD_R_HLP(L);
}

// 0x6F
Task CPU::ld_l_a()
{
	LD_R_R(L, A);
}

// 0x70
Task CPU::ld_hlp_b()
{
	LD_HLP_R(B);
}

// 0x71
Task CPU::ld_hlp_c()
{
	LD_HLP_R(C);
}

// 0x72
Task CPU::ld_hlp_d()
{
	LD_HLP_R(D);
}

// 0x73
Task CPU::ld_hlp_e()
{
	LD_HLP_R(E);
}

// 0x74
Task CPU::ld_hlp_h()
{
	LD_HLP_R(H);
}

// 0x75
Task CPU::ld_hlp_l()
{
	LD_HLP_R(L);
}

// 0x76
Task CPU::halt()
{
	if (irq.IME)
	{
		halted = true;
	}
	else
	{
		if (irq.IF & irq.IE & 0x1F)
		{
			haltBug = true;
		}
		else
		{
			// Now IME is 0, but next cycle could be 1, so this is needs a fix
			halted = true;
		}
	}

	co_return;
}

// 0x77
Task CPU::ld_hlp_a()
{
	LD_HLP_R(A);
}

// 0x78
Task CPU::ld_a_b()
{
	LD_R_R(A, B);
}

// 0x79
Task CPU::ld_a_c()
{
	LD_R_R(A, C);
}

// 0x7A
Task CPU::ld_a_d()
{
	LD_R_R(A, D);
}

// 0x7B
Task CPU::ld_a_e()
{
	LD_R_R(A, E);
}

// 0x7C
Task CPU::ld_a_h()
{
	LD_R_R(A, H);
}

// 0x7D
Task CPU::ld_a_l()
{
	LD_R_R(A, L);
}

// 0x7E
Task CPU::ld_a_hlp()
{
	LD_R_HLP(A);
}

// 0x7F
Task CPU::ld_a_a()
{
	co_return;
}

// 0x80
Task CPU::add_a_b()
{
	ADD_A_N(regs.B);
}

// 0x81
Task CPU::add_a_c()
{
	ADD_A_N(regs.C);
}

// 0x82
Task CPU::add_a_d()
{
	ADD_A_N(regs.D);
}

// 0x83
Task CPU::add_a_e()
{
	ADD_A_N(regs.E);
}

// 0x84
Task CPU::add_a_h()
{
	ADD_A_N(regs.H);
}

// 0x85
Task CPU::add_a_l()
{
	ADD_A_N(regs.L);
}

// 0x86
Task CPU::add_a_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	ADD_A_N(n);
}

// 0x87
Task CPU::add_a_a()
{
	ADD_A_N(regs.A);
}

// 0x88
Task CPU::adc_a_b()
{
	ADC_A_N(regs.B);
}

// 0x89
Task CPU::adc_a_c()
{
	ADC_A_N(regs.C);
}

// 0x8A
Task CPU::adc_a_d()
{
	ADC_A_N(regs.D);
}

// 0x8B
Task CPU::adc_a_e()
{
	ADC_A_N(regs.E);
}

// 0x8C
Task CPU::adc_a_h()
{
	ADC_A_N(regs.H);
}

// 0x8D
Task CPU::adc_a_l()
{
	ADC_A_N(regs.L);
}

// 0x8E
Task CPU::adc_a_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	ADC_A_N(n);
}

// 0x8F
Task CPU::adc_a_a()
{
	ADC_A_N(regs.A);
}

// 0x90
Task CPU::sub_a_b()
{
	SUB_A_N(regs.B);
}

// 0x91
Task CPU::sub_a_c()
{
	SUB_A_N(regs.C);
}

// 0x92
Task CPU::sub_a_d()
{
	SUB_A_N(regs.D);
}

// 0x93
Task CPU::sub_a_e()
{
	SUB_A_N(regs.E);
}

// 0x94
Task CPU::sub_a_h()
{
	SUB_A_N(regs.H);
}

// 0x95
Task CPU::sub_a_l()
{
	SUB_A_N(regs.L);
}

// 0x96
Task CPU::sub_a_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	SUB_A_N(n);
}

// 0x97
Task CPU::sub_a_a()
{
	SUB_A_N(regs.A);
}

// 0x98
Task CPU::sbc_a_b()
{
	SBC_A_N(regs.B);
}

// 0x99
Task CPU::sbc_a_c()
{
	SBC_A_N(regs.C);
}

// 0x9A
Task CPU::sbc_a_d()
{
	SBC_A_N(regs.D);
}

// 0x9B
Task CPU::sbc_a_e()
{
	SBC_A_N(regs.E);
}

// 0x9C
Task CPU::sbc_a_h()
{
	SBC_A_N(regs.H);
}

// 0x9D
Task CPU::sbc_a_l()
{
	SBC_A_N(regs.L);
}

// 0x9E
Task CPU::sbc_a_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	SBC_A_N(n);
}

// 0x9F
Task CPU::sbc_a_a()
{
	SBC_A_N(regs.A);
}

// 0xA0
Task CPU::and_b()
{
	AND_N(regs.B);
}

// 0xA1
Task CPU::and_c()
{
	AND_N(regs.C);
}

// 0xA2
Task CPU::and_d()
{
	AND_N(regs.D);
}

// 0xA3
Task CPU::and_e()
{
	AND_N(regs.E);
}

// 0xA4
Task CPU::and_h()
{
	AND_N(regs.H);
}

// 0xA5
Task CPU::and_l()
{
	AND_N(regs.L);
}

// 0xA6
Task CPU::and_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	AND_N(n);
}

// 0xA7
Task CPU::and_a()
{
	AND_N(regs.A);
}

// 0xA8
Task CPU::xor_b()
{
	XOR_N(regs.B);
}

// 0xA9
Task CPU::xor_c()
{
	XOR_N(regs.C);
}

// 0xAA
Task CPU::xor_d()
{
	XOR_N(regs.D);
}

// 0xAB
Task CPU::xor_e()
{
	XOR_N(regs.E);
}

// 0xAC
Task CPU::xor_h()
{
	XOR_N(regs.H);
}

// 0xAD
Task CPU::xor_l()
{
	XOR_N(regs.L);
}

// 0xAE
Task CPU::xor_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	XOR_N(n);
}

// 0xAF
Task CPU::xor_a()
{
	XOR_N(regs.A);
}

// 0xB0
Task CPU::or_b()
{
	OR_N(regs.B);
}

// 0xB1
Task CPU::or_c()
{
	OR_N(regs.C);
}

// 0xB2
Task CPU::or_d()
{
	OR_N(regs.D);
}

// 0xB3
Task CPU::or_e()
{
	OR_N(regs.E);
}

// 0xB4
Task CPU::or_h()
{
	OR_N(regs.H);
}

// 0xB5
Task CPU::or_l()
{
	OR_N(regs.L);
}

// 0xB6
Task CPU::or_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	OR_N(n);
}

// 0xB7
Task CPU::or_a()
{
	OR_N(regs.A);
}

// 0xB8
Task CPU::cp_b()
{
	CP_N(regs.B);
}

// 0xB9
Task CPU::cp_c()
{
	CP_N(regs.C);
}

// 0xBA
Task CPU::cp_d()
{
	CP_N(regs.D);
}

// 0xBB
Task CPU::cp_e()
{
	CP_N(regs.E);
}

// 0xBC
Task CPU::cp_h()
{
	CP_N(regs.H);
}

// 0xBD
Task CPU::cp_l()
{
	CP_N(regs.L);
}

// 0xBE
Task CPU::cp_hlp()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.HL);
	CP_N(n);
}

// 0xBF
Task CPU::cp_a()
{
	CP_N(regs.A);
}

// 0xC0
Task CPU::ret_nz()
{
	RET(!regs.ZF);
}

// 0xC1
Task CPU::pop_bc()
{
	POP(BC);
}

// 0xC2
Task CPU::jp_nz_nn()
{
	JP(!regs.ZF);
}

// 0xC3
Task CPU::jp_nn()
{
	co_await suspend_always{};

	ushort nn;
	readLow(nn);
	co_await suspend_always{};

	readHigh(nn);
	regs.PC = nn;
	co_await suspend_always{};

    co_return;
}

// 0xC4
Task CPU::call_nz_nn()
{
	CALL(!regs.ZF);
}

// 0xC5
Task CPU::push_bc()
{
	PUSH(BC);
}

// 0xC6
Task CPU::add_a_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	ADD_A_N(n);
}

// 0xC7
Task CPU::rst_00h()
{
	RST(0x0000);
}

// 0xC8
Task CPU::ret_z()
{
	RET(regs.ZF);
}

// 0xC9
Task CPU::ret()
{
	co_await suspend_always{};

	readLow(regs.SP, regs.PC);
	co_await suspend_always{};

	readHigh(regs.SP, regs.PC);
	co_await suspend_always{};

    co_return;
}

// 0xCA
Task CPU::jp_z_nn()
{
	JP(regs.ZF);
}

// 0xCC
Task CPU::call_z_nn()
{
	CALL(regs.ZF);
}

// 0xCD
Task CPU::call_nn()
{
	co_await suspend_always{};

	ushort nn;
	readLow(nn);
	co_await suspend_always{};

	readHigh(nn);
	co_await suspend_always{};

	co_await suspend_always{};

	writeHigh(regs.SP, regs.PC);
	co_await suspend_always{};

	writeLow(regs.SP, regs.PC);
	regs.PC = nn;
    co_return;
}

// 0xCE
Task CPU::adc_a_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	ADC_A_N(n);
}

// 0xCF
Task CPU::rst_08h()
{
	RST(0x0008);
}

// 0xD0
Task CPU::ret_nc()
{
	RET(!regs.CF);
}

// 0xD1
Task CPU::pop_de()
{
	POP(DE);
}

// 0xD2
Task CPU::jp_nc_nn()
{
	JP(!regs.CF);
}

//0xD4
Task CPU::call_nc_nn()
{
	CALL(!regs.CF);
}

// 0xD5
Task CPU::push_de()
{
	PUSH(DE);
}

// 0xD6
Task CPU::sub_a_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	SUB_A_N(n);
}

// 0xD7
Task CPU::rst_10h()
{
	RST(0x0010);
}

// 0xD8
Task CPU::ret_c()
{
	RET(regs.CF);
}

// 0xD9
Task CPU::reti()
{
	irq.IME = true;
	co_await suspend_always{};

	readLow(regs.SP, regs.PC);
	co_await suspend_always{};

	readHigh(regs.SP, regs.PC);
	co_await suspend_always{};
    co_return;
}

// 0xDA
Task CPU::jp_c_nn()
{
	JP(regs.CF);
}

// 0xDC
Task CPU::call_c_nn()
{
	CALL(regs.CF);
}

// 0xDE
Task CPU::sbc_a_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	SBC_A_N(n);
}

// 0xDF
Task CPU::rst_18h()
{
	RST(0x0018);
}

// 0xE0
Task CPU::ldh_np_a()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	co_await suspend_always{};

	mmu.write(0xFF00 + n, regs.A);
    co_return;
}

// 0xE1
Task CPU::pop_hl()
{
	POP(HL);
}

// 0xE2
Task CPU::ld_cp_a()
{
	co_await suspend_always{};

	mmu.write(0xFF00 + regs.C, regs.A);
    co_return;
}


// 0xE5
Task CPU::push_hl()
{
	PUSH(HL);
}

// 0xE6
Task CPU::and_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	AND_N(n);
}

// 0xE7
Task CPU::rst_20h()
{
	RST(0x0020);
}

// 0xE8
Task CPU::add_sp_n()
{
	co_await suspend_always{};

	const byte n = static_cast<byte>(mmu.read(regs.PC++));
	const uint sum = regs.SP + n;
	const uint info = sum ^ (regs.SP ^ n);
	regs.F = ((info & 0x100) >> 4) | ((info & 0x10) << 1);
	regs.SP = sum & 0xFFFF;

	co_await suspend_always{};
	co_await suspend_always{};
    co_return;
}

// 0xE9
Task CPU::jp_hlp()
{
	regs.PC = regs.HL;
	co_return;
}

// 0xEA
Task CPU::ld_nnp_a()
{
	co_await suspend_always{};

	ushort nn;
	readLow(nn);
	co_await suspend_always{};

	readHigh(nn);
	co_await suspend_always{};

	mmu.write(nn, regs.A);
    co_return;
}

// 0xEE
Task CPU::xor_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	XOR_N(n);
}

// 0xEF
Task CPU::rst_28h()
{
	RST(0x0028);
}

// 0xF0
Task CPU::ldh_a_np()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	co_await suspend_always{};

	regs.A = mmu.read(0xFF00 + n);
    co_return;
}

// 0xF1
Task CPU::pop_af()
{
	co_await suspend_always{};
	readLow(regs.SP, regs.AF);
	regs.F &= 0xF0;
	co_await suspend_always{};
	readHigh(regs.SP, regs.AF);
    co_return;
}

// 0xF2
Task CPU::ld_a_cp()
{
	co_await suspend_always{};

	regs.A = mmu.read(0xFF00 + regs.C);
}

// 0xF3
Task CPU::di()
{
	irq.IME = false;
	co_return;
}

// 0xF5
Task CPU::push_af()
{
	PUSH(AF);
}

// 0xF6
Task CPU::or_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	OR_N(n);
}

// 0xF7
Task CPU::rst_30h()
{
	RST(0x0030);
}

// 0xF8
Task CPU::ld_hl_sp_n()
{
	co_await suspend_always{};

	const byte n = static_cast<byte>(mmu.read(regs.PC++));
	const uint sum = regs.SP + n;
	const uint carry = sum ^ (regs.SP ^ n);
	regs.F = ((carry & 0x100) >> 4) | ((carry & 0x10) << 1);
	regs.HL = sum & 0xFFFF;

	co_await suspend_always{};
    co_return;
}

// 0xF9
Task CPU::ld_sp_hl()
{
	co_await suspend_always{};
	regs.SP = regs.HL;
    co_return;
}

// 0xFA
Task CPU::ld_a_nnp()
{
	co_await suspend_always{};

	ushort nn;
	readLow(nn);
	co_await suspend_always{};

	readHigh(nn);
	co_await suspend_always{};

	regs.A = mmu.read(nn);
    co_return;
}

// 0xFB
Task CPU::ei()
{
	irq.delay = true;
	co_return;
}

// 0xFE
Task CPU::cp_n()
{
	co_await suspend_always{};

	const ubyte n = mmu.read(regs.PC++);
	CP_N(n);
}

// 0xFF
Task CPU::rst_38h()
{
	RST(0x0038);
}


// *******************************
// *  CB Opcode implementation   *
// *******************************

#define RLC(x) \
	regs.F = (x & 0x80) >> 3; \
	x = (x << 1) | (regs.F >> 4); \
	if (!x) regs.ZF = 1; \
    co_return;

#define RRC(x) \
	regs.F = (x & 0x1) << 4; \
	x = (x >> 1) | (regs.F << 3); \
	if (!x) regs.ZF = 1; \
    co_return;

#define RL(x) \
	unsigned char carry = regs.CF; \
	regs.F = (x & 0x80) >> 3; \
	x = (x << 1) | carry; \
	if (!x) regs.ZF = 1; \
    co_return;

#define RR(x) \
	unsigned char carry = regs.CF; \
	regs.F = (x & 0x1) << 4; \
	x = (x >> 1) | (carry << 7); \
	if (!x) regs.ZF = 1; \
    co_return;

#define SLA(x) \
	regs.F = (x & 0x80) >> 3; \
	x <<= 1; \
	if (!x) regs.ZF = 1; \
    co_return;

#define SRA(x) \
	regs.F = (x & 0x1) << 4; \
	x = (x & 0x80) | (x >> 1); \
	if (!x) regs.ZF = 1; \
    co_return;

#define SWAP(x) \
	regs.F = 0; \
	x = ((x & 0xF) << 4) | ((x & 0xF0) >> 4); \
	if (!x) regs.ZF = 1; \
    co_return;

#define SRL(x) \
	regs.F = (x & 0x1) << 4; \
	x >>= 1; \
	if (!x) regs.ZF = 1; \
    co_return;

// 0x0X
Task CPU::rlc_b() { co_await suspend_always{}; RLC(regs.B); }
Task CPU::rlc_c() { co_await suspend_always{}; RLC(regs.C); }
Task CPU::rlc_d() { co_await suspend_always{}; RLC(regs.D); }
Task CPU::rlc_e() { co_await suspend_always{}; RLC(regs.E); }
Task CPU::rlc_h() { co_await suspend_always{}; RLC(regs.H); }
Task CPU::rlc_l() { co_await suspend_always{}; RLC(regs.L); }
Task CPU::rlc_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	RLC(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::rlc_a() { co_await suspend_always{}; RLC(regs.A); }
Task CPU::rrc_b() { co_await suspend_always{}; RRC(regs.B); }
Task CPU::rrc_c() { co_await suspend_always{}; RRC(regs.C); }
Task CPU::rrc_d() { co_await suspend_always{}; RRC(regs.D); }
Task CPU::rrc_e() { co_await suspend_always{}; RRC(regs.E); }
Task CPU::rrc_h() { co_await suspend_always{}; RRC(regs.H); }
Task CPU::rrc_l() { co_await suspend_always{}; RRC(regs.L); }
Task CPU::rrc_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	RRC(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::rrc_a() { co_await suspend_always{}; RRC(regs.A); }

// 0x1X
Task CPU::rl_b() { co_await suspend_always{}; RL(regs.B); }
Task CPU::rl_c() { co_await suspend_always{}; RL(regs.C); }
Task CPU::rl_d() { co_await suspend_always{}; RL(regs.D); }
Task CPU::rl_e() { co_await suspend_always{}; RL(regs.E); }
Task CPU::rl_h() { co_await suspend_always{}; RL(regs.H); }
Task CPU::rl_l() { co_await suspend_always{}; RL(regs.L); }
Task CPU::rl_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	RL(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::rl_a() { co_await suspend_always{}; RL(regs.A); }
Task CPU::rr_b() { co_await suspend_always{}; RR(regs.B); }
Task CPU::rr_c() { co_await suspend_always{}; RR(regs.C); }
Task CPU::rr_d() { co_await suspend_always{}; RR(regs.D); }
Task CPU::rr_e() { co_await suspend_always{}; RR(regs.E); }
Task CPU::rr_h() { co_await suspend_always{}; RR(regs.H); }
Task CPU::rr_l() { co_await suspend_always{}; RR(regs.L); }
Task CPU::rr_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	RR(n);
	mmu.write(regs.HL, n);
}
Task CPU::rr_a() { co_await suspend_always{}; RR(regs.A); }

// 0x2X
Task CPU::sla_b() { co_await suspend_always{}; SLA(regs.B); }
Task CPU::sla_c() { co_await suspend_always{}; SLA(regs.C); }
Task CPU::sla_d() { co_await suspend_always{}; SLA(regs.D); }
Task CPU::sla_e() { co_await suspend_always{}; SLA(regs.E); }
Task CPU::sla_h() { co_await suspend_always{}; SLA(regs.H); }
Task CPU::sla_l() { co_await suspend_always{}; SLA(regs.L); }
Task CPU::sla_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	SLA(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::sla_a() { co_await suspend_always{}; SLA(regs.A); }
Task CPU::sra_b() { co_await suspend_always{}; SRA(regs.B); }
Task CPU::sra_c() { co_await suspend_always{}; SRA(regs.C); }
Task CPU::sra_d() { co_await suspend_always{}; SRA(regs.D); }
Task CPU::sra_e() { co_await suspend_always{}; SRA(regs.E); }
Task CPU::sra_h() { co_await suspend_always{}; SRA(regs.H); }
Task CPU::sra_l() { co_await suspend_always{}; SRA(regs.L); }
Task CPU::sra_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	SRA(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::sra_a() { co_await suspend_always{}; SRA(regs.A); }

// 0x3X
Task CPU::swap_b() { co_await suspend_always{}; SWAP(regs.B); }
Task CPU::swap_c() { co_await suspend_always{}; SWAP(regs.C); }
Task CPU::swap_d() { co_await suspend_always{}; SWAP(regs.D); }
Task CPU::swap_e() { co_await suspend_always{}; SWAP(regs.E); }
Task CPU::swap_h() { co_await suspend_always{}; SWAP(regs.H); }
Task CPU::swap_l() { co_await suspend_always{}; SWAP(regs.L); }
Task CPU::swap_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	SWAP(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::swap_a() { co_await suspend_always{}; SWAP(regs.A); }
Task CPU::srl_b() { co_await suspend_always{}; SRL(regs.B); }
Task CPU::srl_c() { co_await suspend_always{}; SRL(regs.C); }
Task CPU::srl_d() { co_await suspend_always{}; SRL(regs.D); }
Task CPU::srl_e() { co_await suspend_always{}; SRL(regs.E); }
Task CPU::srl_h() { co_await suspend_always{}; SRL(regs.H); }
Task CPU::srl_l() { co_await suspend_always{}; SRL(regs.L); }
Task CPU::srl_hlp()
{
	co_await suspend_always{};
	co_await suspend_always{};
	ubyte n = mmu.read(regs.HL);
	co_await suspend_always{};
	SRL(n);
	mmu.write(regs.HL, n);
    co_return;
}
Task CPU::srl_a() { co_await suspend_always{}; SRL(regs.A); }