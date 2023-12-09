#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>
#include <debug.h>

#define MODULE "HAL"

void HAL_Initialize() {
	log_debug(MODULE, "Initializing HAL...");
	VGA_clrscr();

	i686_GDT_Initialize();
	log_info(MODULE, "GDT initialized!");

	i686_IDT_Initialize();
	log_info(MODULE, "IDT initialized!");

	i686_ISR_Initialize();
	log_info(MODULE, "ISR initialized!");
	
	i686_IRQ_Initialize();
	log_info(MODULE, "IRQ initialized!");
}