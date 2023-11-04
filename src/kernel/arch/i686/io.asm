
global i686_outb
i686_outb:
	[bits 32]
	mov dx, [esp + 4]
	mov al, [esp + 8]
	out dx, al
	ret

global i686_inb
i686_inb:
	[bits 32]
	mov dx, [esp + 4]
	xor eax, eax
	in al, dx
	ret

global i686_Panic
i686_Panic:
	cli
	hlt

global crash_me
crash_me:
	; mov ecx, 0x1337
	; div by 0
	; mov eax, 0
	; div eax
	; int 0x80
	ret