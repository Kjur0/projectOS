/** Bootloader stage 2
 * @author @Kjur0
 * @version 0.2.1
 * @see https://wiki.osdev.org/Bootloader#Two-Stage_Bootloader
 * @since 0.0.1
 * Bootloader stage 2 for loading the kernel.
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - @refitem stdio.h
 * - @refitem x86.h
 * - @refitem disk.h
 * - @refitem fat.h
 * - @refitem memdefs.h
 * - @refitem memory.h
 * - @refitem mbr.h
 * @endsecreflist
*/
#include <stdint.h>
#include "stdio.h"
#include "x86.h"
#include "disk.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "mbr.h"

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL; ///< Kernel load buffer.
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR; ///< Kernel address.

typedef void (*KernelStart)();

/** Stage 2 bootloader entry point.
 * @author @Kjur0
 * @version 0.2.2
 * @param bootDrive		Boot drive.
 * @param partition		Partition.
 * @since 0.0.1
 * Stage 2 bootloader for loading the kernel.
*/
void __attribute__((cdecl)) start(uint16_t bootDrive, void* partition) {
	clrscr();

	DISK disk; ///< Disk
	if (!DISK_Initialize(&disk, bootDrive)) {
		printf("Disk init error\r\n");
		goto end;
	}

	Partition part; ///< Partition
	MBR_DetectPartition(&part, &disk, partition);

	if (!FAT_Initialize(&part)) {
		printf("FAT init error\r\n");
		goto end;
	}

	// load kernel
	FAT_File* fd = FAT_Open(&part, "/boot/kernel.bin"); ///< Kernel file
	uint32_t read;
	uint8_t* kernelBuffer = Kernel; ///< Kernel buffer
	while ((read = FAT_Read(&part, fd, MEMORY_LOAD_SIZE, KernelLoadBuffer))) {
		memcpy(kernelBuffer, KernelLoadBuffer, read);
		kernelBuffer += read;
	}
	FAT_Close(fd);

	// execute kernel
	KernelStart kernelStart = (KernelStart)Kernel; ///< Kernel entry point
	kernelStart();

end:
	for (;;); // infinite loop so the computer won't shutdown
}