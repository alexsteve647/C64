#pragma once

//#ifndef LIBRARY_GUARD
//#define LIBRARY_GUARD

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdint.h>

#include "debug.h"

using namespace std;

#define KERNAL_BASIC_ROM "roms/251913-01.bin"
#define CHARSET_ROM "roms/901225-01.bin"

#define fourK 4096
#define eightK 8192
#define sixteenK  16384
#define sixtyfourK 65536

#define KERNAL_START 0xE000
#define KERNAL_END 0xFFFF

#define BASIC_START 0xA000
#define BASIC_END 0xBFFF

#define ZERO_START 0x0000
#define ZERO_END 0x00FF

#define STACK_START 0x0100
#define STACK_END 0x01FF

#define IO_START 0xD000
#define IO_END 0x0DFFF

#define VIC_START 0xD000
#define VIC_END 0xD3FF

#define CIA2_START 0xDD00
#define CIA2_END 0xDDFF

#define RESET_routine 0xFCE2

#define NMI_vector 0xFF43
#define RESET_vector 0xFFFC
#define IRQ_vector 0xFFFE


#define null 0


void hexDump(void*, int);

void loadKernalAndBasic(uint8_t *,const char*);
void loadCharset(uint8_t *,const char*);

enum register_name
{
	regA,regX,regY
	
};

struct registers{
	uint8_t reg[3];

	uint16_t SP;
	uint16_t PC;

	bool sign_flag;
	bool overflow_flag;
	bool zero_flag;
	bool carry_flag;
	bool interrupt_flag;
	bool decimal_mode_flag;

	uint8_t flags;
};

//#endif