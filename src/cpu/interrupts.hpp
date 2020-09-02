#pragma once

#include "../memory/memory_unit.hpp"

class Interrupts : public MemoryUnit
{
public:
	bool IME{ false };
	bool delay{ false };
	bool maybeModified{ false };
	ushort vector{ 0x0000 };

	union {
		struct {
			ubyte IF_VBLANK : 1;
			ubyte IF_LCDC : 1;
			ubyte IF_TIMER : 1;
			ubyte IF_SERIAL : 1;
			ubyte IF_INPUT : 1;
		};
		ubyte IF{ 0xE1 };
	};

	union {
		struct {
			ubyte IE_VBLANK : 1;
			ubyte IE_LCDC : 1;
			ubyte IE_TIMER : 1;
			ubyte IE_SERIAL : 1;
			ubyte IE_INPUT : 1;
		};
		ubyte IE{ 0 };
	};

	void processIrq(ubyte flags);
	ushort irqVector();

	bool accepts(const ushort address) const override;
	void write(const ushort address, ubyte value) override;
	ubyte read(const ushort address) const override;

private:
	uint many_bits(const ubyte b)
	{
		static const uint setbits[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
		return setbits[b >> 8] + setbits[b & 0xF];
	}

	uint first_bit(const ubyte b)
	{
		static const uint first_set_bit[256] =
		{
			0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x0X
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x1X
			6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x2X
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x3X
			7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x4X
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x5X
			6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x6X
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x7X
			8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x8X
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0x9X
			6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xAX
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xBX
			7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xCX
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xDX
			6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xEX
			5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, // 0xFX
		};
		return first_set_bit[b];
	}
};