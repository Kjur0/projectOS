#include "e9.h"
#include <arch/i686/io.h>

void e9_putc(char c) {
	outb(0xe9, c);
}