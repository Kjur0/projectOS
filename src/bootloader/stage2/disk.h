#pragma once

/** Disk functions header.
 * @author @Kjur0
 * @version 0.1.2
 * @since 0.0.1
 * Header for disk functions and structs.
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - stdbool.h
 * @endsecreflist
*/
#include <stdint.h>
#include <stdbool.h>

/** Disk structure.
 * @author @Kjur0
 * @version 0.1.2
 * @since 0.0.1
 * Structure for disk parameters.
*/
typedef struct {
	uint8_t id;
	uint16_t cylinders;
	uint16_t sectors;
	uint16_t heads;
} DISK;

bool DISK_Initialize(DISK* disk, uint8_t driveNumber);
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* lowerDataOut);