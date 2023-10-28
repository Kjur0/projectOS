#include "isr.h"
#include "idt.h"
#include "gdt.h"
#include "io.h"
#include <stdio.h>
#include <stddef.h>

ISRHandler g_ISRHandlers[256];

static const char* const g_Exceptions[] = {
	"Divide by zero error",
	"Debug",
	"Non-maskable Interrupt",
	"Breakpoint",
	"Overflow",
	"Bound Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack-Segment Fault",
	"General Protection Fault",
	"Page Fault",
	"",
	"x87 Floating-Point Exception",
	"Alignment Check",
	"Machine Check",
	"SIMD Floating-Point Exception",
	"Virtualization Exception",
	"Control Protection Exception ",
	"",
	"",
	"",
	"",
	"",
	"",
	"Hypervisor Injection Exception",
	"VMM Communication Exception",
	"Security Exception",
	""
};

void i686_ISR_InitializeGates();

void i686_ISR_Initialize() {
	i686_ISR_InitializeGates();
	for (int i = 0; i < 256; i++)
		i686_IDT_EnableGate(i);

	i686_IDT_DisableGate(0x80);
}

void __attribute__((cdecl)) i686_ISR_Handler(Registers* regs) {
	if (g_ISRHandlers[regs->interrupt] != NULL)
		g_ISRHandlers[regs->interrupt](regs);

	else if (regs->interrupt >= 32)
		printf("Unhandled interrupt %d!\n", regs->interrupt);

	else {
		printf("Unhandled exception %d %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);

		printf("EAX: %x\nEBX: %x\nECX: %x\nEDX: %x\nESI: %x\nEDI: %x\nESP: %x\nEBP: %x\nEIP: %x\nEFLAGS: %x\nCS: %x\nDS: %x\nSS: %x\ninterrupt: %x\nerror: %x\n",
			regs->eax, regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi, regs->esp, regs->ebp, regs->eip, regs->eflags, regs->cs, regs->ds, regs->ss, regs->interrupt, regs->error);

		printf("KERNEL PANIC!\n");
		i686_Panic();
	}
}

void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler) {
	g_ISRHandlers[interrupt] = handler;
	i686_IDT_EnableGate(interrupt);
}