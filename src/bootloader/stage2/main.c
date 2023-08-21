#include "stdint.h"
#include "stdio.h"

void _cdecl cstart_(uint16_t bootDrive)
{
	puts("Stage 2 completed\r\nC language loaded successfully\r\n");
	for (;;)
		;
}
