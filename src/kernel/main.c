#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include <hal/hal.h>

extern uint8_t __bss_start;
extern uint8_t __end;

void crash_me();

void __attribute__((section(".entry"))) start(uint16_t bootDrive) {
	memset(&__bss_start, 0, (&__end) - (&__bss_start));

	HAL_Initialize();

	clrscr();

	printf("Kernel loaded!\n");

	crash_me();

end:
	for (;;)
		;
}