#pragma once
#include <stdint.h>

typedef enum {
	IDT_FLAG_GATE_TASK = 0x05,
	IDT_FLAG_GATE_16BIT_INT = 0x06,
	IDT_FLAG_GATE_16BIT_TRAP = 0x07,
	IDT_FLAG_GATE_32BIT_INT = 0x0E,
	IDT_FLAG_GATE_32BIT_TRAP = 0x0F,

	IDT_FLAG_RING0 = (0 << 5),
	IDT_FLAG_RING1 = (1 << 5),
	IDT_FLAG_RING2 = (2 << 5),
	IDT_FLAG_RING3 = (3 << 5),

	IDT_FLAG_PRESENT = 0x80,

} IDT_FLAGS;
void i686_IDT_Initialize();
void i686_IDT_DisableGate(int interrupt);
void i686_IDT_EnableGate(int interrupt);
void i686_IDT_SetGate(int interrupt, void* base, uint16_t segmentSelector, uint8_t flags);