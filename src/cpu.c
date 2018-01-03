#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "log.h"
#include "cpu.h"

// rom.c externals
extern uint16_t rom_nrombanks;
extern const uint8_t* rom_data;


// cpu.c
static const int8_t clock_table[256] = {
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

static const char* const name_table[256] = {
	[0x00] = "NOP", [0x05] = "DEC B", [0x06] = "LD B, d8",
	[0x0E] = "LD C, d8", [0x0D] = "DEC C", [0x20] = "JR NZ, r8",
	[0x21] = "LD HL, d16", [0x32] = "LD (HL-), A", [0x3E] = "LD A, d8",
	[0xAF] = "XOR A", [0xC3] = "JP a16", [0xD0] = "RET NC"
};

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

static long long int cycles;
static uint8_t zpram[127];
static uint8_t wram[8192];


// flags set/get
static void set_flag(const bool value, const int bit)
{
	if (value)
		rgs.f |= 0x01<<bit;
	else
		rgs.f &= ~(0x01<<bit);
}
static void set_z(const bool value) { set_flag(value, 7); }
static void set_n(const bool value) { set_flag(value, 6); }
static void set_h(const bool value) { set_flag(value, 5); }
static void set_c(const bool value) { set_flag(value, 4); }
static uint8_t get_z(void) { return (rgs.f&(0x01<<7)) != 0; }
static uint8_t get_n(void) { return (rgs.f&(0x01<<6)) != 0; }
static uint8_t get_h(void) { return (rgs.f&(0x01<<5)) != 0; }
static uint8_t get_c(void) { return (rgs.f&(0x01<<4)) != 0; }


// memory operations
static uint8_t memread(const uint16_t addr)
{
	uint8_t val;

	if (addr <= 0x7FFF) {
		val = rom_data[addr];
	} else if (addr >= 0xFF80 && addr <= 0xFFFE) {
		val = zpram[addr - 0xFF80];
	} else if (addr >= 0xC000 && addr <= 0xDFFF) {
		val = wram[addr - 0xC000];
	} else {
		val = 0x00;
		fprintf(stderr, "memread Unknown Address: $%X", addr);
	}

	return val;
}

static void memwrite(const uint8_t val, const uint16_t addr)
{
	if (addr >= 0xFF80 && addr <= 0xFFFE) {
		zpram[addr - 0xFF80] = val;
	} else if (addr >= 0xC000 && addr <= 0xDFFF) {
		wram[addr - 0xC000] = val;
	} else {
		fprintf(stderr, "memwrite at Unknown Address: $%.4X value $%.2X\n", addr, val);
	}
}

static uint16_t memread16(const uint16_t addr)
{
	return (memread(addr + 1)<<8)|memread(addr);
}

static void stackpush(const uint8_t val)
{
	memwrite(val, rgs.sp);
	--rgs.sp;	
}

static uint8_t stackpop(void)
{
	const uint8_t val = memread(rgs.sp);
	++rgs.sp;
	return val;
}

static uint16_t stackpop16(void)
{
	const uint16_t val = memread16(rgs.sp);
	rgs.sp += 2;
	return val;
}

static uint8_t immediate(void)
{
	return memread(rgs.pc++);
}

static uint16_t immediate16(void)
{
	rgs.pc += 2;
	return memread16(rgs.pc - 2);
}


// instructions
static void ret(const bool cond)
{
	if (cond)
		rgs.pc = stackpop16();
}

static void jp_a16(void)
{
	rgs.pc = immediate16();
}

static void jr_r8(const bool cond)
{
	if (cond) {
		rgs.pc += (int8_t)immediate();
	} else {
		++rgs.pc;
	}
}

static uint8_t xor(const uint8_t second)
{
	const uint8_t result = rgs.a ^ second;
	set_z(result == 0);
	set_n(0);
	set_h(0);
	set_c(0);
	return result;
}

static uint8_t dec(const uint8_t value)
{
	const uint8_t result = value - 1;
	set_z(result == 0);
	set_h((((int)(value&0x0F)) - 1) < 0);
	set_n(1);
	return result;
}


void resetcpu(void)
{
	cycles = 0;
	rgs.pc = 0x0100;
	rgs.sp = 0xFFFE;
	rgs.af = 0x01B0;
	rgs.bc = 0x0013;
	rgs.de = 0x00D8;
	rgs.hl = 0x014D;
}


int8_t stepcpu(void)
{
	const uint8_t opcode = memread(rgs.pc++);

	if (name_table[opcode] != NULL)
		printf("Executing Instruction: $%.2X: %s\n", opcode, name_table[opcode]);

	switch (opcode) {
	case 0x00:                            break;                   // NOP
	case 0x05: rgs.b = dec(rgs.b);        break;                   // DEC B
	case 0x06: rgs.b = immediate();       break;                   // LD B, d8
	case 0x0D: rgs.c = dec(rgs.c);        break;                   // DEC C
	case 0x0E: rgs.c = immediate();       break;                   // LD C, d8
	case 0x20: jr_r8(get_z() == 0);       break;                   // JR NZ, r8
	case 0x21: rgs.hl = immediate16();    break;                   // LD HL, d16
	case 0x32: memwrite(rgs.a, rgs.hl--); break;                   // LD (HL-), A
	case 0x3E: rgs.a = immediate();       break;                   // LD A, d8
	case 0xAF: rgs.a = xor(rgs.a);        break;                   // XOR A
	case 0xC3: jp_a16();                  break;                   // JP a16
	case 0xD0: ret(rgs.c != 0);           break;                   // RET NC
	default:
		fprintf(stderr, "Unknown Opcode: $%.2X\n", opcode);
		exit(EXIT_FAILURE);
		break;
	};

	cycles += clock_table[opcode];
	return clock_table[opcode];
}

void printcpu(void)
{
	loginfo("PC: $%.4X\n"
               "SP: $%.4X\n"
               "AF: $%.4X\n"
               "BC: $%.4X\n"
               "DE: $%.4X\n"
               "HL: $%.4X\n"
               "Z: %d\nN: %d\nH: %d\nC: %d\n"
               "Cycles: %lld\n",
               rgs.pc, rgs.sp,
               rgs.af, rgs.bc,
               rgs.de, rgs.hl,
               get_z(), get_n(), get_h(), get_c(),
               cycles);
}

