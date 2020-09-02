#pragma once

#include <types.hpp>
#include <iostream>

struct Registers
{
    union {
        struct {
            union {
				struct {
					unsigned char : 4;
					unsigned char CF : 1;
					unsigned char HF : 1;
					unsigned char NF : 1;
					unsigned char ZF : 1;
				};
				unsigned char F;
			};

            ubyte A;
        };
        ushort AF;
    };

    union {
        struct {
            ubyte C;
            ubyte B;
        };
        ushort BC;
    };

    union {
        struct {
            ubyte E;
            ubyte D;
        };
        ushort DE;
    };

    union {
        struct {
            ubyte L;
            ubyte H;
        };
        ushort HL;
    };

    ushort SP;
    ushort PC;

    void print() const
	{
		std::printf("AF: $%04X, BC: $%04X, DE: $%04X, HL: $%04X, SP: $%04X, PC: $%04X [Z:%d, N:%d, H:%d, C:%d]\n", AF, BC, DE, HL, SP, PC, ZF, NF, HF, CF);
	}
};