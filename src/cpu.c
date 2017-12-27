#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cpu.h"


extern uint16_t rom_nrombanks;
extern uint8_t rom_data[0x4000 * 512];


static const uint8_t clock_table[256] = {
/*     0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
/*0*/  4, 12,  8,  8,  4,  4,  8,  4, 20,  8,  8,  8,  4,  4,  8,  4,
/*1*/  0, 12,  8,  8,  4,  4,  8,  4, 12,  8,  8,  8,  4,  4,  8,  4,
/*2*/  8, 12,  8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, 
/*3*/  8, 12,  8,  8, 12, 12, 12,  4,  8,  8,  8,  8,  4,  4,  8,  4,
/*4*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*5*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*6*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, 
/*7*/  8,  8,  8,  8,  8,  8,  0,  8,  4,  4,  4,  4,  4,  4,  8,  4,
/*8*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*9*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*A*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*B*/  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
/*C*/  8, 12, 12, 16, 12, 16,  8, 16,  8, 16, 12,  0, 12, 24,  8, 16, 
/*D*/  8, 12, 12,  0, 12, 16,  8, 16,  8, 16, 12,  0, 12,  0,  8, 16, 
/*E*/ 12, 12,  8,  0,  0, 16,  8, 16, 16,  4, 16,  0,  0,  0,  8, 16,
/*F*/ 12, 12,  8,  4,  0, 16,  8, 16, 12,  8, 16,  4,  0,  0,  8, 16
};


static struct { 
	uint8_t z : 1; 
	uint8_t n : 1; 
	uint8_t h : 1; 
	uint8_t c : 1; 
} fgs;

static struct {
	uint16_t pc;
	uint16_t sp;

	union {
		struct {
			uint8_t f, a;
		};
		uint16_t af;
	};

	union {
		struct {
			uint8_t c, b;
		};
		uint16_t bc;
	};

	union {
		struct {
			uint8_t e, d;
		};
		uint16_t de;
	};

	union {
		struct {
			uint8_t l, h;
		};
		uint16_t hl;
	};

} rgs;


static uint8_t memread(const uint16_t addr)
{
	if (addr <= 0x7FFF) {
		return rom_data[addr];
	} else {
		fprintf(stderr, "memread Unknown Address: $%X", addr);
	}
	return 0x00;
}

static void memwrite(const uint8_t val, const uint16_t addr)
{
	fprintf(stderr, "memwrite at Unknown Address: $%X value $%X", addr, val);
}

static uint16_t memread16(const uint16_t addr)
{
	return (memread(addr + 1)<<8)|memread(addr);
}


void resetcpu(void)
{
	memset(&fgs, 0, sizeof fgs);
	memset(&rgs, 0, sizeof rgs);
	rgs.pc = 0x0100;
	rgs.sp = 0xFFFE;
	rgs.af = 0x01B0;
	rgs.bc = 0x0013;
	rgs.de = 0x00D8;
	rgs.hl = 0x014D;
}

uint8_t stepcpu(void)
{
	const uint8_t opcode = rom_data[rgs.pc++];

	switch (opcode) {
	case 0x00: break;                                              // NOP
	case 0xC3: rgs.pc = memread16(rgs.pc); break;                  // JP a16
	default:
		fprintf(stderr, "Unknown Opcode: $%X\n", opcode);
		exit(EXIT_FAILURE);
		break;
	};

	return clock_table[opcode];
}

