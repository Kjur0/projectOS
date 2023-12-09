[bits 32]

extern i686_ISR_Handler

; cpu pushes to the stack: ss, esp, eflags, cs, eip

%macro ISR_NOERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
	push 0			; push dummy error code
	push %1			; push interrupt number
	jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1

global i686_ISR%1:
i686_ISR%1:
					; cpu pushes an error code to the stack
	push %1			; push interrupt number
	jmp isr_common

%endmacro

%include "arch/i686/isrs_gen.inc"

isr_common:
	pusha					; pushes edi, esi, ebp, esp, ebx, edx, ecx, eax

	xor eax, eax			; push ds
	mov ax, ds
	push eax

	mov ax, 0x10			; use kernel data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	push esp				; push pointer to stack to C
	call i686_ISR_Handler
	add esp, 4

	pop eax					; restore old segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	popa					; pops edi, esi, ebp, esp, ebx, edx, ecx, eax
	add esp, 8				; remove error code and interrupt number
	iret					; will pop: cs, eip,eflags, ss, esp