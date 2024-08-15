/** Disk functions
 * @author @Kjur0
 * @version 0.1.2
 * @since 0.0.1
 * Disk functions for stage 2 bootloader.
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - stdbool.h
 * - @refitem x86.h
 * - @refitem stdio.h
 * @endsecreflist
*/
#include "disk.h"
#include "x86.h"
#include "stdio.h"

/**	Initialize the disk.
 * @author @Kjur0
 * @version 0.1.2
 * @param disk			The pointer to disk for initialization.
 * @param driveNumber	The drive number to initialize.
 * @return bool			True if the disk was successfully initialized, false otherwise.
 * @since 0.0.1
 * @headerfile disk.h
 * @note This will override previous disk setup.
 * Gets the disk parameters and sets up the disk with them.
*/
bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
	uint8_t driveType;	///< Drive type
						///< @see x86_Disk_GetDriveParams()
	uint16_t cylinders, sectors, heads;	///< Disk parameters
						///< @see x86_Disk_GetDriveParams()

	if (!x86_Disk_GetDriveParams(driveNumber, &driveType, &cylinders, &sectors, &heads))
		return false;

	disk->id = driveNumber;
	disk->cylinders = cylinders;
	disk->heads = heads;
	disk->sectors = sectors;

	return true;
}

/** Convert LBA to CHS.
 * @author @Kjur0
 * @version 0.1.2
 * @param disk			The pointer to disk.
 * @param lba			The logical block address.
 * @param cylinderOut	The pointer to the cylinder output.
 * @param sectorOut		The pointer to the sector output.
 * @param headOut		The pointer to the head output.
 * @see https://wiki.osdev.org/LBA#Comparison_with_CHS
 * @since 0.0.1
 * @headerfile disk.h
 * Converts the logical block address to cylinder, head, and sector.
*/
void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut) {
	/// sector = (LBA % sectors per track + 1)
	*sectorOut = lba % disk->sectors + 1;

	/// cylinder = (LBA / sectors per track) / heads
	*cylinderOut = (lba / disk->sectors) / disk->heads;

	/// head = (LBA / sectors per track) % heads
	*headOut = (lba / disk->sectors) % disk->heads;
}

/** Read disk sectors.
 * @author @Kjur0
 * @version 0.1.2
 * @param disk		The pointer to disk.
 * @param lba		The logical block address.
 * @param sectors	The number of sectors to read.
 * @param dataOut	The pointer to the data output.
 * @return bool		True if the sectors were successfully read, false otherwise.
 * @since 0.0.1
 * @headerfile disk.h
 * Reads the specified number of sectors from the disk.
*/
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void* dataOut) {
	uint16_t cylinder, sector, head;///< CHS values
									///< @see DISK_LBA2CHS()

	DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

	for (int i = 0; i < 3; i++) {
		if (x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut))
			return true;

		x86_Disk_Reset(disk->id);
	}

	return false;
}