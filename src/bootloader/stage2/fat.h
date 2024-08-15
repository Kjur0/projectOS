#pragma once

/** FAT12/16/32 filesystem driver header
 * @author @Kjur0
 * @version 0.1.1
 * @see {@link https://wiki.osdev.org/FAT FAT}
 * @since 0.0.1
 * FAT12/16/32 filesystem driver for stage 2 bootloader.
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - @refitem mbr.h
 * @endsecreflist
*/
#include <stdint.h>
#include "mbr.h"

/** FAT directory entry structure.
 * @author @Kjur0
 * @version 0.1.1
 * @see {@link https://wiki.osdev.org/FAT#Directories_on_FAT12.2F16.2F32 FAT Directory Entry}
 * @since 0.0.1
 * FAT directory entry structure for stage 2 bootloader.
*/
typedef struct {
	uint8_t Name[11];			///< File name.
	uint8_t Attributes;			///< File attributes.
								///< @see #FAT_Attributes
	uint8_t _Reserved;			///< Reserved.
								///< @warning Do not modify.
	uint8_t CreatedTimeTenths;	///< Created time (tenths of second.)
	uint16_t CreatedTime;		///< Created time.
	uint16_t CreatedDate;		///< Created date.
	uint16_t AccessedDate;		///< Accessed date.
	uint16_t FirstClusterHigh;	///< First cluster (high bytes).
	uint16_t ModifiedTime;		///< Modified time.
	uint16_t ModifiedDate;		///< Modified date.
	uint16_t FirstClusterLow;	///< First cluster (low bytes).
	uint32_t Size;				///< File size.
} __attribute__((packed)) FAT_DirectoryEntry;

/** File handling structure.
 * @author @Kjur0
 * @version 0.1.1
 * @since 0.0.1
 * Structure for handling files in FAT.
*/
typedef struct {
	int Handle;			///< File handle.
	bool IsDirectory;	///< Is directory.
	uint32_t Position;	///< File position (lba).
	uint32_t Size;		///< File size.
} FAT_File;

/** FAT attributes enumeration.
 * @author @Kjur0
 * @version 0.1.1
 * @since 0.0.1
 * FAT attributes enumeration for stage 2 bootloader.
*/
enum FAT_Attributes {
	FAT_ATTRIBUTE_READ_ONLY = 0x01,	///< Read only.
	FAT_ATTRIBUTE_HIDDEN = 0x02,	///< Hidden file.
	FAT_ATTRIBUTE_SYSTEM = 0x04,	///< System file.
	FAT_ATTRIBUTE_VOLUME_ID = 0x08,	///< Volume ID.
	FAT_ATTRIBUTE_DIRECTORY = 0x10,	///< Directory.
	FAT_ATTRIBUTE_ARCHIVE = 0x20,	///< Archive.
	FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID ///< Lonf File Name.
};

bool FAT_Initialize(Partition* disk);
FAT_File* FAT_Open(Partition* disk, const char* path);
uint32_t FAT_Read(Partition* disk, FAT_File* file, uint32_t byteCount, void* dataOut);
bool FAT_ReadEntry(Partition* disk, FAT_File* file, FAT_DirectoryEntry* dirEntry);
void FAT_Close(FAT_File* file);