#pragma once

#include "interrupts.hpp"
#include "opcode.hpp"
#include "registers.hpp"

#include "../memory/mmu.hpp"

struct CPU
{
    MMU& mmu;
	Interrupts& irq;
    Registers regs;
	Task instruction;

	uint cyclesLeft;
	bool stopped;
	bool halted;
	bool haltBug;

    CPU(MMU& mmu, Interrupts& irq);
	void execute();

private:
	void fetchOpcode();
	Task interruptCallback();

	void readLow(ushort& dest);
	void readHigh(ushort& dest);
	void readLow(ushort& address, ushort& dest);
	void readHigh(ushort& address, ushort& dest);
	void writeLow(ushort& address, const ushort value);
	void writeHigh(ushort& address, const ushort value);

private:
    const Opcode opcodeTable[256]
	{
		// 0x0X
		{ "NOP", 0, 1, &CPU::nop },
		{ "LD BC, 0x%04X", 2, 3, &CPU::ld_bc_nn },
		{ "LD (BC), A", 0, 2, &CPU::ld_bcp_a },
		{ "INC BC", 0, 2, &CPU::inc_bc },
		{ "INC B", 0, 1, &CPU::inc_b },
		{ "DEC B", 0, 1, &CPU::dec_b },
		{ "LD B, 0x%02X", 1, 2, &CPU::ld_b_n },
		{ "RLCA", 0, 1, &CPU::rlca },
		{ "LD (0x%04X), SP", 2, 5, &CPU::ld_nnp_sp },
		{ "ADD HL, BC", 0, 2, &CPU::add_hl_bc },
		{ "LD A, (BC)", 0, 2, &CPU::ld_a_bcp },
		{ "DEC BC", 0, 2, &CPU::dec_bc },
		{ "INC C", 0, 1, &CPU::inc_c },
		{ "DEC C", 0, 1, &CPU::dec_c },
		{ "LD C, 0x%02X", 1, 2, &CPU::ld_c_n },
		{ "RRCA", 0, 1, &CPU::rrca },

		// 0x1X
		{ "STOP", 0, 1, &CPU::stop },
		{ "LD DE, 0x%04X", 2, 3, &CPU::ld_de_nn },
		{ "LD (DE), A", 0, 2, &CPU::ld_dep_a },
		{ "INC DE", 0, 2, &CPU::inc_de },
		{ "INC D", 0, 1, &CPU::inc_d },
		{ "DEC D", 0, 1, &CPU::dec_d },
		{ "LD D, 0x%02X", 1, 2, &CPU::ld_d_n },
		{ "RLA", 0, 1, &CPU::rla },
		{ "JR 0x%02X", 1, 3, &CPU::jr_n },
		{ "ADD HL, DE", 0, 2, &CPU::add_hl_de },
		{ "LD A, (DE)", 0, 2, &CPU::ld_a_dep },
		{ "DEC DE", 0, 2, &CPU::dec_de },
		{ "INC E", 0, 1, &CPU::inc_e },
		{ "DEC E", 0, 1, &CPU::dec_e },
		{ "LD E, 0x%02X", 1, 2, &CPU::ld_e_n },
		{ "RRA", 0, 1, &CPU::rra },

		// 0x2X
		{ "JR NZ, 0x%02X", 1, 2, &CPU::jr_nz_n },
		{ "LD HL, 0x%04X", 2, 3, &CPU::ld_hl_nn },
		{ "LD (HL+), A", 0, 2, &CPU::ld_hlip_a },
		{ "INC HL", 0, 2, &CPU::inc_hl },
		{ "INC H", 0, 1, &CPU::inc_h },
		{ "DEC H", 0, 1, &CPU::dec_h },
		{ "LD H, 0x%02X", 1, 2, &CPU::ld_h_n },
		{ "DAA", 0, 1, &CPU::daa },
		{ "JR Z, 0x%02X", 1, 2, &CPU::jr_z_n },
		{ "ADD HL, HL", 0, 2, &CPU::add_hl_hl },
		{ "LD A, (HL+)", 0, 2, &CPU::ld_a_hlip },
		{ "DEC HL", 0, 2, &CPU::dec_hl },
		{ "INC L", 0, 1, &CPU::inc_l },
		{ "DEC L", 0, 1, &CPU::dec_l },
		{ "LD L, 0x%02X", 1, 2, &CPU::ld_l_n },
		{ "CPL", 0, 1, &CPU::cpl },

		// 0x3X
		{ "JR NC, 0x%02X", 1, 2, &CPU::jr_nc_n },
		{ "LD SP, 0x%04X", 2, 3, &CPU::ld_sp_nn },
		{ "LD (HL-), A", 0, 2, &CPU::ld_hldp_a },
		{ "INC SP", 0, 2, &CPU::inc_sp },
		{ "INC (HL)", 0, 3, &CPU::inc_hlp },
		{ "DEC (HL)", 0, 3, &CPU::dec_hlp },
		{ "LD (HL), 0x%02X", 1, 3, &CPU::ld_hlp_n },
		{ "SCF", 0, 1, &CPU::scf },
		{ "JR C, 0x%02X", 1, 2, &CPU::jr_c_n },
		{ "ADD HL, SP", 0, 2, &CPU::add_hl_sp },
		{ "LD A, (HL-)", 0, 2, &CPU::ld_a_hldp },
		{ "DEC SP", 0, 2, &CPU::dec_sp },
		{ "INC A", 0, 1, &CPU::inc_a },
		{ "DEC A", 0, 1, &CPU::dec_a },
		{ "LD A, 0x%02X", 1, 2, &CPU::ld_a_n },
		{ "CCF", 0, 1, &CPU::ccf },

		// 0x4X
		{ "LD B, B", 0, 1, &CPU::ld_b_b },
		{ "LD B, C", 0, 1, &CPU::ld_b_c },
		{ "LD B, D", 0, 1, &CPU::ld_b_d },
		{ "LD B, E", 0, 1, &CPU::ld_b_e },
		{ "LD B, H", 0, 1, &CPU::ld_b_h },
		{ "LD B, L", 0, 1, &CPU::ld_b_l },
		{ "LD B, (HL)", 0, 2, &CPU::ld_b_hlp },
		{ "LD B, A", 0, 1, &CPU::ld_b_a },
		{ "LD C, B", 0, 1, &CPU::ld_c_b },
		{ "LD C, C", 0, 1, &CPU::ld_c_c },
		{ "LD C, D", 0, 1, &CPU::ld_c_d },
		{ "LD C, E", 0, 1, &CPU::ld_c_e },
		{ "LD C, H", 0, 1, &CPU::ld_c_h },
		{ "LD C, L", 0, 1, &CPU::ld_c_l },
		{ "LD C, (HL)", 0, 2, &CPU::ld_c_hlp },
		{ "LD C, A", 0, 1, &CPU::ld_c_a },

		// 0x5X
		{ "LD D, B", 0, 1, &CPU::ld_d_b },
		{ "LD D, C", 0, 1, &CPU::ld_d_c },
		{ "LD D, D", 0, 1, &CPU::ld_d_d },
		{ "LD D, E", 0, 1, &CPU::ld_d_e },
		{ "LD D, H", 0, 1, &CPU::ld_d_h },
		{ "LD D, L", 0, 1, &CPU::ld_d_l },
		{ "LD D, (HL)", 0, 2, &CPU::ld_d_hlp },
		{ "LD D, A", 0, 1, &CPU::ld_d_a },
		{ "LD E, B", 0, 1, &CPU::ld_e_b },
		{ "LD E, C", 0, 1, &CPU::ld_e_c },
		{ "LD E, D", 0, 1, &CPU::ld_e_d },
		{ "LD E, E", 0, 1, &CPU::ld_e_e },
		{ "LD E, H", 0, 1, &CPU::ld_e_h },
		{ "LD E, L", 0, 1, &CPU::ld_e_l },
		{ "LD E, (HL)", 0, 2, &CPU::ld_e_hlp },
		{ "LD E, A", 0, 1, &CPU::ld_e_a },

		// 0x6X
		{ "LD H, B", 0, 1, &CPU::ld_h_b },
		{ "LD H, C", 0, 1, &CPU::ld_h_c },
		{ "LD H, D", 0, 1, &CPU::ld_h_d },
		{ "LD H, E", 0, 1, &CPU::ld_h_e },
		{ "LD H, H", 0, 1, &CPU::ld_h_h },
		{ "LD H, L", 0, 1, &CPU::ld_h_l },
		{ "LD H, (HL)", 0, 2, &CPU::ld_h_hlp },
		{ "LD H, A", 0, 1, &CPU::ld_h_a },
		{ "LD L, B", 0, 1, &CPU::ld_l_b },
		{ "LD L, C", 0, 1, &CPU::ld_l_c },
		{ "LD L, D", 0, 1, &CPU::ld_l_d },
		{ "LD L, E", 0, 1, &CPU::ld_l_e },
		{ "LD L, H", 0, 1, &CPU::ld_l_h },
		{ "LD L, L", 0, 1, &CPU::ld_l_l },
		{ "LD L, (HL)", 0, 2, &CPU::ld_l_hlp },
		{ "LD L, A", 0, 1, &CPU::ld_l_a },

		// 0x7X
		{ "LD (HL), B", 0, 2, &CPU::ld_hlp_b },
		{ "LD (HL), C", 0, 2, &CPU::ld_hlp_c },
		{ "LD (HL), D", 0, 2, &CPU::ld_hlp_d },
		{ "LD (HL), E", 0, 2, &CPU::ld_hlp_e },
		{ "LD (HL), H", 0, 2, &CPU::ld_hlp_h },
		{ "LD (HL), L", 0, 2, &CPU::ld_hlp_l },
		{ "HALT", 0, 1, &CPU::halt },
		{ "LD (HL), A", 0, 2, &CPU::ld_hlp_a },
		{ "LD A, B", 0, 1, &CPU::ld_a_b },
		{ "LD A, C", 0, 1, &CPU::ld_a_c },
		{ "LD A, D", 0, 1, &CPU::ld_a_d },
		{ "LD A, E", 0, 1, &CPU::ld_a_e },
		{ "LD A, H", 0, 1, &CPU::ld_a_h },
		{ "LD A, L", 0, 1, &CPU::ld_a_l },
		{ "LD A, (HL)", 0, 2, &CPU::ld_a_hlp },
		{ "LD A, A", 0, 1, &CPU::ld_a_a },

		// 0x8X
		{ "ADD A, B", 0, 1, &CPU::add_a_b },
		{ "ADD A, C", 0, 1, &CPU::add_a_c },
		{ "ADD A, D", 0, 1, &CPU::add_a_d },
		{ "ADD A, E", 0, 1, &CPU::add_a_e },
		{ "ADD A, H", 0, 1, &CPU::add_a_h },
		{ "ADD A, L", 0, 1, &CPU::add_a_l },
		{ "ADD A, (HL)", 0, 2, &CPU::add_a_hlp },
		{ "ADD A, A", 0, 1, &CPU::add_a_a },
		{ "ADC A, B", 0, 1, &CPU::adc_a_b },
		{ "ADC A, C", 0, 1, &CPU::adc_a_c },
		{ "ADC A, D", 0, 1, &CPU::adc_a_d },
		{ "ADC A, E", 0, 1, &CPU::adc_a_e },
		{ "ADC A, H", 0, 1, &CPU::adc_a_h },
		{ "ADC A, L", 0, 1, &CPU::adc_a_l },
		{ "ADC A, (HL)", 0, 2, &CPU::adc_a_hlp },
		{ "ADC A, A", 0, 1, &CPU::adc_a_a },

		// 0x9X
		{ "SUB A, B", 0, 1, &CPU::sub_a_b },
		{ "SUB A, C", 0, 1, &CPU::sub_a_c },
		{ "SUB A, D", 0, 1, &CPU::sub_a_d },
		{ "SUB A, E", 0, 1, &CPU::sub_a_e },
		{ "SUB A, H", 0, 1, &CPU::sub_a_h },
		{ "SUB A, L", 0, 1, &CPU::sub_a_l },
		{ "SUB A, (HL)", 0, 2, &CPU::sub_a_hlp },
		{ "SUB A, A", 0, 1, &CPU::sub_a_a },
		{ "SBC A, B", 0, 1, &CPU::sbc_a_b },
		{ "SBC A, C", 0, 1, &CPU::sbc_a_c },
		{ "SBC A, D", 0, 1, &CPU::sbc_a_d },
		{ "SBC A, E", 0, 1, &CPU::sbc_a_e },
		{ "SBC A, H", 0, 1, &CPU::sbc_a_h },
		{ "SBC A, L", 0, 1, &CPU::sbc_a_l },
		{ "SBC A, (HL)", 0, 2, &CPU::sbc_a_hlp },
		{ "SBC A, A", 0, 1, &CPU::sbc_a_a },

		// 0xAX
		{ "AND B", 0, 1, &CPU::and_b },
		{ "AND C", 0, 1, &CPU::and_c },
		{ "AND D", 0, 1, &CPU::and_d },
		{ "AND E", 0, 1, &CPU::and_e },
		{ "AND H", 0, 1, &CPU::and_h },
		{ "AND L", 0, 1, &CPU::and_l },
		{ "AND (HL)", 0, 2, &CPU::and_hlp },
		{ "AND A", 0, 1, &CPU::and_a },
		{ "XOR B", 0, 1, &CPU::xor_b },
		{ "XOR C", 0, 1, &CPU::xor_c },
		{ "XOR D", 0, 1, &CPU::xor_d },
		{ "XOR E", 0, 1, &CPU::xor_e },
		{ "XOR H", 0, 1, &CPU::xor_h },
		{ "XOR L", 0, 1, &CPU::xor_l },
		{ "XOR (HL)", 0, 2, &CPU::xor_hlp },
		{ "XOR A", 0, 1, &CPU::xor_a },

		// 0xBX
		{ "OR B", 0, 1, &CPU::or_b },
		{ "OR C", 0, 1, &CPU::or_c },
		{ "OR D", 0, 1, &CPU::or_d },
		{ "OR E", 0, 1, &CPU::or_e },
		{ "OR H", 0, 1, &CPU::or_h },
		{ "OR L", 0, 1, &CPU::or_l },
		{ "OR (HL)", 0, 2, &CPU::or_hlp },
		{ "OR A", 0, 1, &CPU::or_a },
		{ "CP B", 0, 1, &CPU::cp_b },
		{ "CP C", 0, 1, &CPU::cp_c },
		{ "CP D", 0, 1, &CPU::cp_d },
		{ "CP E", 0, 1, &CPU::cp_e },
		{ "CP H", 0, 1, &CPU::cp_h },
		{ "CP L", 0, 1, &CPU::cp_l },
		{ "CP (HL)", 0, 2, &CPU::cp_hlp },
		{ "CP A", 0, 1, &CPU::cp_a },

		// 0xCX
		{ "RET NZ", 0, 2, &CPU::ret_nz },
		{ "POP BC", 0, 3, &CPU::pop_bc },
		{ "JP NZ, 0x%04X", 2, 3, &CPU::jp_nz_nn },
		{ "JP 0x%04X", 2, 4, &CPU::jp_nn },
		{ "CALL NZ, 0x%04X", 2, 3, &CPU::call_nz_nn },
		{ "PUSH BC", 0, 4, &CPU::push_bc },
		{ "ADD A, 0x%02X", 1, 2, &CPU::add_a_n },
		{ "RST 00H", 0, 4, &CPU::rst_00h },
		{ "RET Z", 0, 2, &CPU::ret_z },
		{ "RET", 0, 4, &CPU::ret },
		{ "JP Z, 0x%04X", 2, 3, &CPU::jp_z_nn },
		{ "PREFIX CB", 1, 1, &CPU::undefined },
		{ "CALL Z, 0x%04X", 2, 3, &CPU::call_z_nn },
		{ "CALL 0x%04X", 2, 6, &CPU::call_nn },
		{ "ADC A, 0x%02X", 1, 2, &CPU::adc_a_n },
		{ "RST 08H", 0, 4, &CPU::rst_08h },

		// 0xDX
		{ "RET NC", 0, 2, &CPU::ret_nc },
		{ "POP DE", 0, 3, &CPU::pop_de },
		{ "JP NC, 0x%04X", 2, 3, &CPU::jp_nc_nn },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "CALL NC, 0x%04X", 2, 3, &CPU::call_nc_nn },
		{ "PUSH DE", 0, 4, &CPU::push_de },
		{ "SUB A, 0x%02X", 1, 2, &CPU::sub_a_n },
		{ "RST 10H", 0, 4, &CPU::rst_10h },
		{ "RET C", 0, 2, &CPU::ret_c },
		{ "RETI", 0, 4, &CPU::reti },
		{ "JP C, 0x%04X", 2, 3, &CPU::jp_c_nn },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "CALL C, 0x%04X", 2, 3, &CPU::call_c_nn },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "SBC A, 0x%02X", 1, 2, &CPU::sbc_a_n },
		{ "RST 18H", 0, 4, &CPU::rst_18h },

		// 0xEX
		{ "LD ($FF00 + $%02X), A", 1, 3, &CPU::ldh_np_a },
		{ "POP HL", 0, 3, &CPU::pop_hl },
		{ "LD ($FF00 + C), A", 0, 2, &CPU::ld_cp_a },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "PUSH HL", 0, 4, &CPU::push_hl },
		{ "AND 0x%02X", 1, 2, &CPU::and_n },
		{ "RST 20H", 0, 4, &CPU::rst_20h },
		{ "ADD SP, $%02X", 1, 4, &CPU::add_sp_n },
		{ "JP (HL)", 0, 1, &CPU::jp_hlp },
		{ "LD ($%04X), A", 2, 4, &CPU::ld_nnp_a },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "XOR $%02X", 1, 2, &CPU::xor_n },
		{ "RST 28H", 0, 4, &CPU::rst_28h },

		// 0xFX
		{ "LD A, ($FF00 + $%02X)", 1, 3, &CPU::ldh_a_np },
		{ "POP AF", 0, 3, &CPU::pop_af },
		{ "LD A, ($FF00 + C)", 0, 2, &CPU::ld_a_cp },
		{ "DI", 0, 1, &CPU::di },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "PUSH AF", 0, 4, &CPU::push_af },
		{ "OR $%02X", 1, 2, &CPU::or_n },
		{ "RST 30H", 0, 4, &CPU::rst_30h },
		{ "LD HL, SP+$%02X", 1, 3, &CPU::ld_hl_sp_n },
		{ "LD SP, HL", 0, 2, &CPU::ld_sp_hl },
		{ "LD A, ($%04X)", 2, 4, &CPU::ld_a_nnp },
		{ "EI", 0, 1, &CPU::ei },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "UNDEFINED", 0, 1, &CPU::undefined },
		{ "CP $%02X", 1, 2, &CPU::cp_n },
		{ "RST 38H", 0, 4, &CPU::rst_38h }
	};

	const CbOpcode cbOpcodeTable[256]
	{
		// 0x0X
		{ "RLC B", 2, &CPU::rlc_b },
		{ "RLC C", 2, &CPU::rlc_c },
		{ "RLC D", 2, &CPU::rlc_d },
		{ "RLC E", 2, &CPU::rlc_e },
		{ "RLC H", 2, &CPU::rlc_h },
		{ "RLC L", 2, &CPU::rlc_l },
		{ "RLC (HL)", 4, &CPU::rlc_hlp },
		{ "RLC A", 2, &CPU::rlc_a },
		{ "RRC B", 2, &CPU::rrc_b },
		{ "RRC C", 2, &CPU::rrc_c },
		{ "RRC D", 2, &CPU::rrc_d },
		{ "RRC E", 2, &CPU::rrc_e },
		{ "RRC H", 2, &CPU::rrc_h },
		{ "RRC L", 2, &CPU::rrc_l },
		{ "RRC (HL)", 4, &CPU::rrc_hlp },
		{ "RRC A", 2, &CPU::rrc_a },

		// 0x1X
		{ "RL B", 2, &CPU::rl_b },
		{ "RL C", 2, &CPU::rl_c },
		{ "RL D", 2, &CPU::rl_d },
		{ "RL E", 2, &CPU::rl_e },
		{ "RL H", 2, &CPU::rl_h },
		{ "RL L", 2, &CPU::rl_l },
		{ "RL (HL)", 4, &CPU::rl_hlp },
		{ "RL A", 2, &CPU::rl_a },
		{ "RR B", 2, &CPU::rr_b },
		{ "RR C", 2, &CPU::rr_c },
		{ "RR D", 2, &CPU::rr_d },
		{ "RR E", 2, &CPU::rr_e },
		{ "RR H", 2, &CPU::rr_h },
		{ "RR L", 2, &CPU::rr_l },
		{ "RR (HL)", 4, &CPU::rr_hlp },
		{ "RR A", 2, &CPU::rr_a },

		// 0x2X
		{ "SLA B", 2, &CPU::sla_b },
		{ "SLA C", 2, &CPU::sla_c },
		{ "SLA D", 2, &CPU::sla_d },
		{ "SLA E", 2, &CPU::sla_e },
		{ "SLA H", 2, &CPU::sla_h },
		{ "SLA L", 2, &CPU::sla_l },
		{ "SLA (HL)", 4, &CPU::sla_hlp },
		{ "SLA A", 2, &CPU::sla_a },
		{ "SRA B", 2, &CPU::sra_b },
		{ "SRA C", 2, &CPU::sra_c },
		{ "SRA D", 2, &CPU::sra_d },
		{ "SRA E", 2, &CPU::sra_e },
		{ "SRA H", 2, &CPU::sra_h },
		{ "SRA L", 2, &CPU::sra_l },
		{ "SRA (HL)", 4, &CPU::sra_hlp },
		{ "SRA A", 2, &CPU::sra_a },

		// 0x3X
		{ "SWAP B", 2, &CPU::swap_b },
		{ "SWAP C", 2, &CPU::swap_c },
		{ "SWAP D", 2, &CPU::swap_d },
		{ "SWAP E", 2, &CPU::swap_e },
		{ "SWAP H", 2, &CPU::swap_h },
		{ "SWAP L", 2, &CPU::swap_l },
		{ "SWAP (HL)", 4, &CPU::swap_hlp },
		{ "SWAP A", 2, &CPU::swap_a },
		{ "SRL B", 2, &CPU::srl_b },
		{ "SRL C", 2, &CPU::srl_c },
		{ "SRL D", 2, &CPU::srl_d },
		{ "SRL E", 2, &CPU::srl_e },
		{ "SRL H", 2, &CPU::srl_h },
		{ "SRL L", 2, &CPU::srl_l },
		{ "SRL (HL)", 4, &CPU::srl_hlp },
		{ "SRL A", 2, &CPU::srl_a },

		// 0x4X
		{ "BIT 0, B", 2, &CPU::bit_b<0> },
		{ "BIT 0, C", 2, &CPU::bit_c<0> },
		{ "BIT 0, D", 2, &CPU::bit_d<0> },
		{ "BIT 0, E", 2, &CPU::bit_e<0> },
		{ "BIT 0, H", 2, &CPU::bit_h<0> },
		{ "BIT 0, L", 2, &CPU::bit_l<0> },
		{ "BIT 0, (HL)", 3, &CPU::bit_hlp<0> },
		{ "BIT 0, A", 2, &CPU::bit_a<0> },
		{ "BIT 1, B", 2, &CPU::bit_b<1> },
		{ "BIT 1, C", 2, &CPU::bit_c<1> },
		{ "BIT 1, D", 2, &CPU::bit_d<1> },
		{ "BIT 1, E", 2, &CPU::bit_e<1> },
		{ "BIT 1, H", 2, &CPU::bit_h<1> },
		{ "BIT 1, L", 2, &CPU::bit_l<1> },
		{ "BIT 1, (HL)", 3, &CPU::bit_hlp<1> },
		{ "BIT 1, A", 2, &CPU::bit_a<1> },

		// 0x5X
		{ "BIT 2, B", 2, &CPU::bit_b<2> },
		{ "BIT 2, C", 2, &CPU::bit_c<2> },
		{ "BIT 2, D", 2, &CPU::bit_d<2> },
		{ "BIT 2, E", 2, &CPU::bit_e<2> },
		{ "BIT 2, H", 2, &CPU::bit_h<2> },
		{ "BIT 2, L", 2, &CPU::bit_l<2> },
		{ "BIT 2, (HL)", 3, &CPU::bit_hlp<2> },
		{ "BIT 2, A", 2, &CPU::bit_a<2> },
		{ "BIT 3, B", 2, &CPU::bit_b<3> },
		{ "BIT 3, C", 2, &CPU::bit_c<3> },
		{ "BIT 3, D", 2, &CPU::bit_d<3> },
		{ "BIT 3, E", 2, &CPU::bit_e<3> },
		{ "BIT 3, H", 2, &CPU::bit_h<3> },
		{ "BIT 3, L", 2, &CPU::bit_l<3> },
		{ "BIT 3, (HL)", 3, &CPU::bit_hlp<3> },
		{ "BIT 3, A", 2, &CPU::bit_a<3> },

		// 0x6X
		{ "BIT 4, B", 2, &CPU::bit_b<4> },
		{ "BIT 4, C", 2, &CPU::bit_c<4> },
		{ "BIT 4, D", 2, &CPU::bit_d<4> },
		{ "BIT 4, E", 2, &CPU::bit_e<4> },
		{ "BIT 4, H", 2, &CPU::bit_h<4> },
		{ "BIT 4, L", 2, &CPU::bit_l<4> },
		{ "BIT 4, (HL)", 3, &CPU::bit_hlp<4> },
		{ "BIT 4, A", 2, &CPU::bit_a<4> },
		{ "BIT 5, B", 2, &CPU::bit_b<5> },
		{ "BIT 5, C", 2, &CPU::bit_c<5> },
		{ "BIT 5, D", 2, &CPU::bit_d<5> },
		{ "BIT 5, E", 2, &CPU::bit_e<5> },
		{ "BIT 5, H", 2, &CPU::bit_h<5> },
		{ "BIT 5, L", 2, &CPU::bit_l<5> },
		{ "BIT 5, (HL)", 3, &CPU::bit_hlp<5> },
		{ "BIT 5, A", 2, &CPU::bit_a<5> },

		// 0x7X
		{ "BIT 6, B", 2, &CPU::bit_b<6> },
		{ "BIT 6, C", 2, &CPU::bit_c<6> },
		{ "BIT 6, D", 2, &CPU::bit_d<6> },
		{ "BIT 6, E", 2, &CPU::bit_e<6> },
		{ "BIT 6, H", 2, &CPU::bit_h<6> },
		{ "BIT 6, L", 2, &CPU::bit_l<6> },
		{ "BIT 6, (HL)", 3, &CPU::bit_hlp<6> },
		{ "BIT 6, A", 2, &CPU::bit_a<6> },
		{ "BIT 7, B", 2, &CPU::bit_b<7> },
		{ "BIT 7, C", 2, &CPU::bit_c<7> },
		{ "BIT 7, D", 2, &CPU::bit_d<7> },
		{ "BIT 7, E", 2, &CPU::bit_e<7> },
		{ "BIT 7, H", 2, &CPU::bit_h<7> },
		{ "BIT 7, L", 2, &CPU::bit_l<7> },
		{ "BIT 7, (HL)", 3, &CPU::bit_hlp<7> },
		{ "BIT 7, A", 2, &CPU::bit_a<7> },

		// 0x8X
		{ "RES 0, B", 2, &CPU::res_b<0> },
		{ "RES 0, C", 2, &CPU::res_c<0> },
		{ "RES 0, D", 2, &CPU::res_d<0> },
		{ "RES 0, E", 2, &CPU::res_e<0> },
		{ "RES 0, H", 2, &CPU::res_h<0> },
		{ "RES 0, L", 2, &CPU::res_l<0> },
		{ "RES 0, (HL)", 4, &CPU::res_hlp<0> },
		{ "RES 0, A", 2, &CPU::res_a<0> },
		{ "RES 1, B", 2, &CPU::res_b<1> },
		{ "RES 1, C", 2, &CPU::res_c<1> },
		{ "RES 1, D", 2, &CPU::res_d<1> },
		{ "RES 1, E", 2, &CPU::res_e<1> },
		{ "RES 1, H", 2, &CPU::res_h<1> },
		{ "RES 1, L", 2, &CPU::res_l<1> },
		{ "RES 1, (HL)", 4, &CPU::res_hlp<1> },
		{ "RES 1, A", 2, &CPU::res_a<1> },

		// 0x9X
		{ "RES 2, B", 2, &CPU::res_b<2> },
		{ "RES 2, C", 2, &CPU::res_c<2> },
		{ "RES 2, D", 2, &CPU::res_d<2> },
		{ "RES 2, E", 2, &CPU::res_e<2> },
		{ "RES 2, H", 2, &CPU::res_h<2> },
		{ "RES 2, L", 2, &CPU::res_l<2> },
		{ "RES 2, (HL)", 4, &CPU::res_hlp<2> },
		{ "RES 2, A", 2, &CPU::res_a<2> },
		{ "RES 3, B", 2, &CPU::res_b<3> },
		{ "RES 3, C", 2, &CPU::res_c<3> },
		{ "RES 3, D", 2, &CPU::res_d<3> },
		{ "RES 3, E", 2, &CPU::res_e<3> },
		{ "RES 3, H", 2, &CPU::res_h<3> },
		{ "RES 3, L", 2, &CPU::res_l<3> },
		{ "RES 3, (HL)", 4, &CPU::res_hlp<3> },
		{ "RES 3, A", 2, &CPU::res_a<3> },

		// 0xAX
		{ "RES 4, B", 2, &CPU::res_b<4> },
		{ "RES 4, C", 2, &CPU::res_c<4> },
		{ "RES 4, D", 2, &CPU::res_d<4> },
		{ "RES 4, E", 2, &CPU::res_e<4> },
		{ "RES 4, H", 2, &CPU::res_h<4> },
		{ "RES 4, L", 2, &CPU::res_l<4> },
		{ "RES 4, (HL)", 4, &CPU::res_hlp<4> },
		{ "RES 4, A", 2, &CPU::res_a<4> },
		{ "RES 5, B", 2, &CPU::res_b<5> },
		{ "RES 5, C", 2, &CPU::res_c<5> },
		{ "RES 5, D", 2, &CPU::res_d<5> },
		{ "RES 5, E", 2, &CPU::res_e<5> },
		{ "RES 5, H", 2, &CPU::res_h<5> },
		{ "RES 5, L", 2, &CPU::res_l<5> },
		{ "RES 5, (HL)", 4, &CPU::res_hlp<5> },
		{ "RES 5, A", 2, &CPU::res_a<5> },

		// 0xBX
		{ "RES 6, B", 2, &CPU::res_b<6> },
		{ "RES 6, C", 2, &CPU::res_c<6> },
		{ "RES 6, D", 2, &CPU::res_d<6> },
		{ "RES 6, E", 2, &CPU::res_e<6> },
		{ "RES 6, H", 2, &CPU::res_h<6> },
		{ "RES 6, L", 2, &CPU::res_l<6> },
		{ "RES 6, (HL)", 4, &CPU::res_hlp<6> },
		{ "RES 6, A", 2, &CPU::res_a<6> },
		{ "RES 7, B", 2, &CPU::res_b<7> },
		{ "RES 7, C", 2, &CPU::res_c<7> },
		{ "RES 7, D", 2, &CPU::res_d<7> },
		{ "RES 7, E", 2, &CPU::res_e<7> },
		{ "RES 7, H", 2, &CPU::res_h<7> },
		{ "RES 7, L", 2, &CPU::res_l<7> },
		{ "RES 7, (HL)", 4, &CPU::res_hlp<7> },
		{ "RES 7, A", 2, &CPU::res_a<7> },

		// 0xCX
		{ "SET 0, B", 2, &CPU::set_b<0> },
		{ "SET 0, C", 2, &CPU::set_c<0> },
		{ "SET 0, D", 2, &CPU::set_d<0> },
		{ "SET 0, E", 2, &CPU::set_e<0> },
		{ "SET 0, H", 2, &CPU::set_h<0> },
		{ "SET 0, L", 2, &CPU::set_l<0> },
		{ "SET 0, (HL)", 4, &CPU::set_hlp<0> },
		{ "SET 0, A", 2, &CPU::set_a<0> },
		{ "SET 1, B", 2, &CPU::set_b<1> },
		{ "SET 1, C", 2, &CPU::set_c<1> },
		{ "SET 1, D", 2, &CPU::set_d<1> },
		{ "SET 1, E", 2, &CPU::set_e<1> },
		{ "SET 1, H", 2, &CPU::set_h<1> },
		{ "SET 1, L", 2, &CPU::set_l<1> },
		{ "SET 1, (HL)", 4, &CPU::set_hlp<1> },
		{ "SET 1, A", 2, &CPU::set_a<1> },

		// 0xDX
		{ "SET 2, B", 2, &CPU::set_b<2> },
		{ "SET 2, C", 2, &CPU::set_c<2> },
		{ "SET 2, D", 2, &CPU::set_d<2> },
		{ "SET 2, E", 2, &CPU::set_e<2> },
		{ "SET 2, H", 2, &CPU::set_h<2> },
		{ "SET 2, L", 2, &CPU::set_l<2> },
		{ "SET 2, (HL)", 4, &CPU::set_hlp<2> },
		{ "SET 2, A", 2, &CPU::set_a<2> },
		{ "SET 3, B", 2, &CPU::set_b<3> },
		{ "SET 3, C", 2, &CPU::set_c<3> },
		{ "SET 3, D", 2, &CPU::set_d<3> },
		{ "SET 3, E", 2, &CPU::set_e<3> },
		{ "SET 3, H", 2, &CPU::set_h<3> },
		{ "SET 3, L", 2, &CPU::set_l<3> },
		{ "SET 3, (HL)", 4, &CPU::set_hlp<3> },
		{ "SET 3, A", 2, &CPU::set_a<3> },

		// 0xEX
		{ "SET 4, B", 2, &CPU::set_b<4> },
		{ "SET 4, C", 2, &CPU::set_c<4> },
		{ "SET 4, D", 2, &CPU::set_d<4> },
		{ "SET 4, E", 2, &CPU::set_e<4> },
		{ "SET 4, H", 2, &CPU::set_h<4> },
		{ "SET 4, L", 2, &CPU::set_l<4> },
		{ "SET 4, (HL)", 4, &CPU::set_hlp<4> },
		{ "SET 4, A", 2, &CPU::set_a<4> },
		{ "SET 5, B", 2, &CPU::set_b<5> },
		{ "SET 5, C", 2, &CPU::set_c<5> },
		{ "SET 5, D", 2, &CPU::set_d<5> },
		{ "SET 5, E", 2, &CPU::set_e<5> },
		{ "SET 5, H", 2, &CPU::set_h<5> },
		{ "SET 5, L", 2, &CPU::set_l<5> },
		{ "SET 5, (HL)", 4, &CPU::set_hlp<5> },
		{ "SET 5, A", 2, &CPU::set_a<5> },

		// 0xFX
		{ "SET 6, B", 2, &CPU::set_b<6> },
		{ "SET 6, C", 2, &CPU::set_c<6> },
		{ "SET 6, D", 2, &CPU::set_d<6> },
		{ "SET 6, E", 2, &CPU::set_e<6> },
		{ "SET 6, H", 2, &CPU::set_h<6> },
		{ "SET 6, L", 2, &CPU::set_l<6> },
		{ "SET 6, (HL)", 4, &CPU::set_hlp<6> },
		{ "SET 6, A", 2, &CPU::set_a<6> },
		{ "SET 7, B", 2, &CPU::set_b<7> },
		{ "SET 7, C", 2, &CPU::set_c<7> },
		{ "SET 7, D", 2, &CPU::set_d<7> },
		{ "SET 7, E", 2, &CPU::set_e<7> },
		{ "SET 7, H", 2, &CPU::set_h<7> },
		{ "SET 7, L", 2, &CPU::set_l<7> },
		{ "SET 7, (HL)", 4, &CPU::set_hlp<7> },
		{ "SET 7, A", 2, &CPU::set_a<7> },
	};

private:
    Task undefined();

	// 0x0X
	Task nop();
	Task ld_bc_nn();
	Task ld_bcp_a();
	Task inc_bc();
	Task inc_b();
	Task dec_b();
	Task ld_b_n();
	Task rlca();
	Task ld_nnp_sp();
	Task add_hl_bc();
	Task ld_a_bcp();
	Task dec_bc();
	Task inc_c();
	Task dec_c();
	Task ld_c_n();
	Task rrca();

	// 0x1X
	Task stop();
	Task ld_de_nn();
	Task ld_dep_a();
	Task inc_de();
	Task inc_d();
	Task dec_d();
	Task ld_d_n();
	Task rla();
	Task jr_n();
	Task add_hl_de();
	Task ld_a_dep();
	Task dec_de();
	Task inc_e();
	Task dec_e();
	Task ld_e_n();
	Task rra();

	// 0x2X
	Task jr_nz_n();
	Task ld_hl_nn();
	Task ld_hlip_a();
	Task inc_hl();
	Task inc_h();
	Task dec_h();
	Task ld_h_n();
	Task daa();
	Task jr_z_n();
	Task add_hl_hl();
	Task ld_a_hlip();
	Task dec_hl();
	Task inc_l();
	Task dec_l();
	Task ld_l_n();
	Task cpl();

	// 0x3X
	Task jr_nc_n();
	Task ld_sp_nn();
	Task ld_hldp_a();
	Task inc_sp();
	Task inc_hlp();
	Task dec_hlp();
	Task ld_hlp_n();
	Task scf();
	Task jr_c_n();
	Task add_hl_sp();
	Task ld_a_hldp();
	Task dec_sp();
	Task inc_a();
	Task dec_a();
	Task ld_a_n();
	Task ccf();

	// 0x4X
	Task ld_b_b();
	Task ld_b_c();
	Task ld_b_d();
	Task ld_b_e();
	Task ld_b_h();
	Task ld_b_l();
	Task ld_b_hlp();
	Task ld_b_a();
	Task ld_c_b();
	Task ld_c_c();
	Task ld_c_d();
	Task ld_c_e();
	Task ld_c_h();
	Task ld_c_l();
	Task ld_c_hlp();
	Task ld_c_a();

	// 0x5X
	Task ld_d_b();
	Task ld_d_c();
	Task ld_d_d();
	Task ld_d_e();
	Task ld_d_h();
	Task ld_d_l();
	Task ld_d_hlp();
	Task ld_d_a();
	Task ld_e_b();
	Task ld_e_c();
	Task ld_e_d();
	Task ld_e_e();
	Task ld_e_h();
	Task ld_e_l();
	Task ld_e_hlp();
	Task ld_e_a();

	// 0x6X
	Task ld_h_b();
	Task ld_h_c();
	Task ld_h_d();
	Task ld_h_e();
	Task ld_h_h();
	Task ld_h_l();
	Task ld_h_hlp();
	Task ld_h_a();
	Task ld_l_b();
	Task ld_l_c();
	Task ld_l_d();
	Task ld_l_e();
	Task ld_l_h();
	Task ld_l_l();
	Task ld_l_hlp();
	Task ld_l_a();

	// 0x7X
	Task ld_hlp_b();
	Task ld_hlp_c();
	Task ld_hlp_d();
	Task ld_hlp_e();
	Task ld_hlp_h();
	Task ld_hlp_l();
	Task halt();
	Task ld_hlp_a();
	Task ld_a_b();
	Task ld_a_c();
	Task ld_a_d();
	Task ld_a_e();
	Task ld_a_h();
	Task ld_a_l();
	Task ld_a_hlp();
	Task ld_a_a();

	// 0x8X
	Task add_a_b();
	Task add_a_c();
	Task add_a_d();
	Task add_a_e();
	Task add_a_h();
	Task add_a_l();
	Task add_a_hlp();
	Task add_a_a();
	Task adc_a_b();
	Task adc_a_c();
	Task adc_a_d();
	Task adc_a_e();
	Task adc_a_h();
	Task adc_a_l();
	Task adc_a_hlp();
	Task adc_a_a();

	// 0x9X
	Task sub_a_b();
	Task sub_a_c();
	Task sub_a_d();
	Task sub_a_e();
	Task sub_a_h();
	Task sub_a_l();
	Task sub_a_hlp();
	Task sub_a_a();
	Task sbc_a_b();
	Task sbc_a_c();
	Task sbc_a_d();
	Task sbc_a_e();
	Task sbc_a_h();
	Task sbc_a_l();
	Task sbc_a_hlp();
	Task sbc_a_a();

	// 0xAX
	Task and_b();
	Task and_c();
	Task and_d();
	Task and_e();
	Task and_h();
	Task and_l();
	Task and_hlp();
	Task and_a();
	Task xor_b();
	Task xor_c();
	Task xor_d();
	Task xor_e();
	Task xor_h();
	Task xor_l();
	Task xor_hlp();
	Task xor_a();

	// 0xBX
	Task or_b();
	Task or_c();
	Task or_d();
	Task or_e();
	Task or_h();
	Task or_l();
	Task or_hlp();
	Task or_a();
	Task cp_b();
	Task cp_c();
	Task cp_d();
	Task cp_e();
	Task cp_h();
	Task cp_l();
	Task cp_hlp();
	Task cp_a();

	// 0xCX
	Task ret_nz();
	Task pop_bc();
	Task jp_nz_nn();
	Task jp_nn();
	Task call_nz_nn();
	Task push_bc();
	Task add_a_n();
	Task rst_00h();
	Task ret_z();
	Task ret();
	Task jp_z_nn();
	// 0xCB
	Task call_z_nn();
	Task call_nn();
	Task adc_a_n();
	Task rst_08h();

	// 0xDX
	Task ret_nc();
	Task pop_de();
	Task jp_nc_nn();
	// undefined
	Task call_nc_nn();
	Task push_de();
	Task sub_a_n();
	Task rst_10h();
	Task ret_c();
	Task reti();
	Task jp_c_nn();
	// undefined
	Task call_c_nn();
	// undefined
	Task sbc_a_n();
	Task rst_18h();

	// 0xEX
	Task ldh_np_a();
	Task pop_hl();
	Task ld_cp_a();
	// undefined
	// undefined
	Task push_hl();
	Task and_n();
	Task rst_20h();
	Task add_sp_n();
	Task jp_hlp();
	Task ld_nnp_a();
	// undefined
	// undefined
	// undefined
	Task xor_n();
	Task rst_28h();

	// 0xFX
	Task ldh_a_np();
	Task pop_af();
	Task ld_a_cp();
	Task di();
	// undefined
	Task push_af();
	Task or_n();
	Task rst_30h();
	Task ld_hl_sp_n();
	Task ld_sp_hl();
	Task ld_a_nnp();
	Task ei();
	// undefined
	// undefined
	Task cp_n();
	Task rst_38h();


	// *******************************
	// *    CB Opcode declaration    *
	// *******************************

	// 0x0X
	Task rlc_b();
	Task rlc_c();
	Task rlc_d();
	Task rlc_e();
	Task rlc_h();
	Task rlc_l();
	Task rlc_hlp();
	Task rlc_a();
	Task rrc_b();
	Task rrc_c();
	Task rrc_d();
	Task rrc_e();
	Task rrc_h();
	Task rrc_l();
	Task rrc_hlp();
	Task rrc_a();

	// 0x1X
	Task rl_b();
	Task rl_c();
	Task rl_d();
	Task rl_e();
	Task rl_h();
	Task rl_l();
	Task rl_hlp();
	Task rl_a();
	Task rr_b();
	Task rr_c();
	Task rr_d();
	Task rr_e();
	Task rr_h();
	Task rr_l();
	Task rr_hlp();
	Task rr_a();

	// 0x2X
	Task sla_b();
	Task sla_c();
	Task sla_d();
	Task sla_e();
	Task sla_h();
	Task sla_l();
	Task sla_hlp();
	Task sla_a();
	Task sra_b();
	Task sra_c();
	Task sra_d();
	Task sra_e();
	Task sra_h();
	Task sra_l();
	Task sra_hlp();
	Task sra_a();

	// 0x3X
	Task swap_b();
	Task swap_c();
	Task swap_d();
	Task swap_e();
	Task swap_h();
	Task swap_l();
	Task swap_hlp();
	Task swap_a();
	Task srl_b();
	Task srl_c();
	Task srl_d();
	Task srl_e();
	Task srl_h();
	Task srl_l();
	Task srl_hlp();
	Task srl_a();

	#define BIT(b, x) \
		regs.ZF = (x & (1 << b)) ? 0 : 1; \
		regs.NF = 0; \
		regs.HF = 1;

	#define RES(b, x) \
		x &= ~(1 << b);

	#define SET(b, x) \
		x |= (1 << b);

	// 0x4X - 0x7X
	template <unsigned char Bit> Task bit_b() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.B); }
	template <unsigned char Bit> Task bit_c() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.C); }
	template <unsigned char Bit> Task bit_d() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.D); }
	template <unsigned char Bit> Task bit_e() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.E); }
	template <unsigned char Bit> Task bit_h() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.H); }
	template <unsigned char Bit> Task bit_l() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.L); }
	template <unsigned char Bit>
	Task bit_hlp()
	{
		co_await std::experimental::suspend_always{};
		co_await std::experimental::suspend_always{};
		unsigned char n = mmu.read(regs.HL);
		BIT(Bit, n);
	}
	template <unsigned char Bit> Task bit_a() { co_await std::experimental::suspend_always{}; BIT(Bit, regs.A); }

	// 0x8X - 0xBX
	template <unsigned char Bit> Task res_b() { co_await std::experimental::suspend_always{}; RES(Bit, regs.B); }
	template <unsigned char Bit> Task res_c() { co_await std::experimental::suspend_always{}; RES(Bit, regs.C); }
	template <unsigned char Bit> Task res_d() { co_await std::experimental::suspend_always{}; RES(Bit, regs.D); }
	template <unsigned char Bit> Task res_e() { co_await std::experimental::suspend_always{}; RES(Bit, regs.E); }
	template <unsigned char Bit> Task res_h() { co_await std::experimental::suspend_always{}; RES(Bit, regs.H); }
	template <unsigned char Bit> Task res_l() { co_await std::experimental::suspend_always{}; RES(Bit, regs.L); }
	template <unsigned char Bit>
	Task res_hlp()
	{
		co_await std::experimental::suspend_always{};
		co_await std::experimental::suspend_always{};
		unsigned char n = mmu.read(regs.HL);
		co_await std::experimental::suspend_always{};
		RES(Bit, n);
		mmu.write(regs.HL, n);
	}
	template <unsigned char Bit> Task res_a() { co_await std::experimental::suspend_always{}; RES(Bit, regs.A); }

	// 0xCX - 0xFX
	template <unsigned char Bit> Task set_b() { co_await std::experimental::suspend_always{}; SET(Bit, regs.B); }
	template <unsigned char Bit> Task set_c() { co_await std::experimental::suspend_always{}; SET(Bit, regs.C); }
	template <unsigned char Bit> Task set_d() { co_await std::experimental::suspend_always{}; SET(Bit, regs.D); }
	template <unsigned char Bit> Task set_e() { co_await std::experimental::suspend_always{}; SET(Bit, regs.E); }
	template <unsigned char Bit> Task set_h() { co_await std::experimental::suspend_always{}; SET(Bit, regs.H); }
	template <unsigned char Bit> Task set_l() { co_await std::experimental::suspend_always{}; SET(Bit, regs.L); }
	template <unsigned char Bit>
	Task set_hlp()
	{
		co_await std::experimental::suspend_always{};
		co_await std::experimental::suspend_always{};
		unsigned char n = mmu.read(regs.HL);
		co_await std::experimental::suspend_always{};
		SET(Bit, n);
		mmu.write(regs.HL, n);
	}
	template <unsigned char Bit> Task set_a() { co_await std::experimental::suspend_always{}; SET(Bit, regs.A); }
};