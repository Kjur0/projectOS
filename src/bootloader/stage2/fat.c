/** FAT12/16/32 filesystem driver
 * @author @Kjur0
 * @version 0.1.1
 * @see https://wiki.osdev.org/FAT
 * @since 0.0.1
 * FAT12/16/32 filesystem driver for stage 2 bootloader.
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - @refitem mbr.h
 * - @refitem stdio.h
 * - @refitem memdefs.h
 * - @refitem string.h
 * - @refitem memory.h
 * - @refitem ctype.h
 * - stddef
 * - @refitem minmax.h
 * @endsecreflist
*/
#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include <stddef.h>
#include "minmax.h"

/** FAT constants
 * @since 0.0.1
 * Constants for FAT filesystem
 * @defgroup FAT constants
 * @{
*/
#define SECTOR_SIZE				512	///< Sector size
#define MAX_PATH_SIZE			256	///< Maximum path size
#define MAX_FILE_HANDLES		10	///< Maximum file handles
#define ROOT_DIRECTORY_HANDLE	-1	///< Root directory handle
#define FAT_CACHE_SIZE			5	///< FAT cache size
/// @}

/** EBR for FAT12/16
 * @author @Kjur0
 * @version 0.1.1
 * @see {@link https://wiki.osdev.org/FAT#Extended_Boot_Record EBR}
 * @since 0.0.1
 * Extended boot record for FAT12/16
*/
typedef struct {
	uint8_t DriveNumber;		///< Drive number
								///< @note should be BIOS I13 DL output
	uint8_t _Reserved;			///< Reserved byte
								///< @warning do not modify
	uint8_t Signature;			///< Signature
								///< @attention must be 0x28 or 0x29
	uint32_t VolumeId;			///< serial number
								///< @note value doesn't matter
	uint8_t VolumeLabel[11];	///< Volume label
								///< @note 11 bytes padded with spaces
	uint8_t SystemId[8];		///< FileSystem ID
								///< @note 8 bytes padded with spaces
} __attribute__((packed)) FAT_ExtendedBootRecord;

/** EBR for FAT32
 * @author @Kjur0
 * @version 0.1.0
 * @see {@link https://wiki.osdev.org/FAT#FAT_32_2 EBR}
 * @since 0.0.1
 * Extended boot record for FAT32
*/
typedef struct {
	uint32_t SectorsPerFat;			///< Sectors per FAT
	uint16_t Flags;					///< Flags
	uint16_t FatVersion;			///< FAT version
	uint32_t RootDirectoryCluster;	///< Root directory cluster
	uint16_t FSInfoSector;			///< FSInfo sector
									///< @see {@link https://wiki.osdev.org/FAT#FSInfo_Structure_.28FAT32_only.29 FSInfo}
	uint16_t BackupBootSector;		///< Backup boot sector
	uint8_t _Reserved[12];			///< Reserved 12 bytes
									///< @warning do not modify
	FAT_ExtendedBootRecord EBR;		///< EBR
									///< @note standard EBR from FAT12/16
									///< @see FAT_ExtendedBootRecord
} __attribute((packed)) FAT32_ExtendedBootRecord;

/** Boot record
 * @author @Kjur0
 * @version 0.1.2
 * @see {@link https://wiki.osdev.org/FAT#Boot_Record Boot Record}
 * @since 0.0.1
*/
typedef struct {
/** BPB
 * @author @Kjur0
 * @version 0.1.2
 * @see {@link https://wiki.osdev.org/FAT#BPB_.28BIOS_Parameter_Block.29 BPB}
 * @since 0.0.1
 * BIOS Parameter Block
 * @defgroup BPB
 * @{
*/
	uint8_t BootJumpInstruction[3];	///< Boot jump instruction
									///< @note 3 bytes
									///< @attention must be 0xEB [address] 0x90 (JMP short [address] NOP)
	uint8_t OemIdentifier[8];		///< OEM identifier
									///< @note 8 bytes padded with spaces
	uint16_t BytesPerSector;		///< Number of bytes per sector
	uint8_t SectorsPerCluster;		///< Number of sectors per cluster
	uint16_t ReservedSectors;		///< Number of reserved sectors
	uint8_t FatCount;				///< Number of FATs
	uint16_t DirEntryCount;			///< Number of directory entries
	uint16_t TotalSectors;			///< Number of total sectors
	uint8_t MediaDescriptorType;	///< Media descriptor type
									///< @see {@link https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB20_OFS_0Ah Media descriptor type}
	uint16_t SectorsPerFat;			///< Number of sectors per FAT
	uint16_t SectorsPerTrack;		///< Number of sectors per track
	uint16_t Heads;					///< Number of heads
	uint32_t HiddenSectors;			///< Number of hidden sectors
	uint32_t LargeSectorCount;		///< Large sector count
									///< @note used if TotalSectors is 0
/// @}
/** EBR union
 * @note The actual memory size of the union will be determined by the larger of the two data types.
 * @see FAT_ExtendedBootRecord
 * @see FAT32_ExtendedBootRecord
 * This union is used to conveniently store either a FAT_ExtendedBootRecord or a FAT32_ExtendedBootRecord.
 * It allows accessing the same memory location using different data types, depending on the context.
 */
	union {
		FAT_ExtendedBootRecord EBR1216;
		FAT32_ExtendedBootRecord EBR32;
	};
} __attribute__((packed)) FAT_BootSector;

/** File data structure.
 * @author @Kjur0
 * @version 0.1.1
 * @since 0.0.1
 * A structure for file data in FAT.
*/
typedef struct {
	uint8_t Buffer[SECTOR_SIZE]; 		///< Buffer
	FAT_File Public;					///< Public file data
										///< @see #FAT_File
	bool Opened;						///< Opened
	uint32_t FirstCluster;				///< Start of file (cluster)
	uint32_t CurrentCluster;			///< Current position (cluster)
	uint32_t CurrentSectorInCluster; 	///< Current position (sector)

} FAT_FileData;

/** FAT data
 * @author @Kjur0
 * @version 0.1.1
 * @since 0.0.1
 * File Allocation Table data
*/
typedef struct {
	/** Boot sector
	 * @author @Kjur0
	 * @version 0.1.1
	 * @see @FAT_BootSector
	 * @since 0.0.1
	 * Boot sector data
	*/
	union {
		FAT_BootSector BootSector;
		uint8_t BootSectorBytes[SECTOR_SIZE];
	} BS;

	FAT_FileData RootDirectory;						///< Root directory
													///< @see #FAT_FileData

	FAT_FileData OpenedFiles[MAX_FILE_HANDLES];		///< Opened files
													///< @see #FAT_FileData

	uint8_t FatCache[FAT_CACHE_SIZE * SECTOR_SIZE];	///< FAT cache
	uint32_t FatCachePosition;						///< FAT cache position

} FAT_Data;

static FAT_Data* g_Data; 			///< current FAT data
static uint32_t g_DataSectionLba;	///< data section LBA
static uint8_t g_FatType;			///< FAT type
static uint32_t g_TotalSectors;		///< total sectors
static uint32_t g_SectorsPerFat;	///< sectors per FAT

/** Convert cluster to LBA
 * @author @Kjur0
 * @version 0.1.1
 * @param cluster		Cluster number
 * @return uint32_t		Linear Block Address
 * @since 0.0.1
 * Convert cluster to Linear Block Address
*/
uint32_t FAT_ClusterToLba(uint32_t cluster) {
	return g_DataSectionLba + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
}

/** Read Boot Sector
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @return bool			True if the boot sector was successfully read, false otherwise
 * @see Partition_ReadSectors()
 * @since 0.0.1
 * Read boot sector from disk
*/
bool FAT_ReadBootSector(Partition* disk) {
	return Partition_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

/** Read File Allocation Table (FAT) from disk
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @param lbaIndex		Linear Block Address index
 * @return bool			True if the FAT was successfully read, false otherwise
 * @see Partition_ReadSectors()
 * @since 0.0.1
 * Read File Allocation Table (FAT) from disk
*/
bool FAT_ReadFat(Partition* disk, size_t lbaIndex) {
	return Partition_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors + lbaIndex, FAT_CACHE_SIZE, g_Data->FatCache);
}

/** Detect FAT type from disk data
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to check
 * @since 0.0.1
 * @note This function sets the global variable g_FatType.
 * Detect FAT type from disk data and set g_FatType
*/
void FAT_Detect(Partition* disk) {
	uint32_t dataClusters = (g_TotalSectors - g_DataSectionLba) / g_Data->BS.BootSector.SectorsPerCluster;
	if (dataClusters < 0xFF5)
		g_FatType = 12;
	else if (dataClusters < 0xFFF5)
		g_FatType = 16;
	else
		g_FatType = 32;
}

/** Initialize FAT filesystem driver
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @return bool			True if the filesystem was successfully initialized, false otherwise
 * @since 0.0.1
 * @headerfile fat.h
 * Initialize FAT filesystem driver and setup neccecary variables
*/
bool FAT_Initialize(Partition* disk) {
	g_Data = (FAT_Data*)MEMORY_FAT_ADDR;

	// read boot sector
	if (!FAT_ReadBootSector(disk)) {
		printf("FAT: Failed to read boot sector!\r\n");
		return false;
	}

	// read FAT
	g_Data->FatCachePosition = 0xFFFFFFFF;
	g_TotalSectors = g_Data->BS.BootSector.TotalSectors;
	if (g_TotalSectors == 0) {			// fat32
		g_TotalSectors = g_Data->BS.BootSector.LargeSectorCount;
	}

	bool isFat32 = false;	///< Is FAT32
	g_SectorsPerFat = g_Data->BS.BootSector.SectorsPerFat;
	if (g_SectorsPerFat == 0) {			// fat32
		isFat32 = true;
		g_SectorsPerFat = g_Data->BS.BootSector.EBR32.SectorsPerFat;
	}

	// open root directory file
	uint32_t rootDirLba;	///< LBA of root directory
	uint32_t rootDirSize;	///< Size of root directory
	if (isFat32) {
		g_DataSectionLba = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
		rootDirLba = FAT_ClusterToLba(g_Data->BS.BootSector.EBR32.RootDirectoryCluster);
		rootDirSize = 0;
	} else {
		rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_SectorsPerFat * g_Data->BS.BootSector.FatCount;
		rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
		uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
		g_DataSectionLba = rootDirLba + rootDirSectors;
	}

	g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
	g_Data->RootDirectory.Public.IsDirectory = true;
	g_Data->RootDirectory.Public.Position = 0;
	g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
	g_Data->RootDirectory.Opened = true;
	g_Data->RootDirectory.FirstCluster = rootDirLba;
	g_Data->RootDirectory.CurrentCluster = rootDirLba;
	g_Data->RootDirectory.CurrentSectorInCluster = 0;

	if (!Partition_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
		printf("FAT: Failed to read root directory!\r\n");
		return false;
	}

	// calculate data section
	FAT_Detect(disk);

	// reset opened files
	for (int i = 0; i < MAX_FILE_HANDLES; i++)
		g_Data->OpenedFiles[i].Opened = false;

	return true;
}

/** Open directory entry as file handle
 * @author @Kjur0
 * @version 0.1.1
 * @param disk				Pointer to Partition to read from
 * @param entry				Pointer to FAT_DirectoryEntry to open
 * @return FAT_File*		Pointer to opened file
 * @since 0.0.1
 * Open directory entry as file handle
*/
FAT_File* FAT_OpenEntry(Partition* disk, FAT_DirectoryEntry* entry) {
	// find empty handle
	int handle = -1;
	for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {
		if (!g_Data->OpenedFiles[i].Opened)
			handle = i;
	}

	// out of handles
	if (handle < 0) {
		printf("FAT: Out of file handles!\r\n");
		return false;
	}

	// setup vars
	FAT_FileData* fd = &g_Data->OpenedFiles[handle];	///< File data
	fd->Public.Handle = handle;
	fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
	fd->Public.Position = 0;
	fd->Public.Size = entry->Size;
	fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
	fd->CurrentCluster = fd->FirstCluster;
	fd->CurrentSectorInCluster = 0;

	if (!Partition_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)) {
		printf("FAT: Failed to read entry! CLUSTER:%u LBA:%u\n", fd->CurrentCluster, FAT_ClusterToLba(fd->CurrentCluster));
		for (int i = 0; i < 11; i++)
			printf("%c", entry->Name[i]);
		printf("\n");
		return false;
	}

	fd->Opened = true;
	return &fd->Public;
}

/** Get next cluster in chain from FAT table
 * @author @Kjur0
 * @version 0.1.1
 * @param disk				Pointer to Partition to read from
 * @param currentCluster	Current cluster
 * @return uint32_t		Next cluster in chain
 * @since 0.0.1
 * Get next cluster in chain from FAT table
*/
uint32_t FAT_NextCluster(Partition* disk, uint32_t currentCluster) {
	// Determine the byte offset of the entry we need to read
	uint32_t fatIndex;		///< FAT index
	if (g_FatType == 12) {
		fatIndex = currentCluster * 3 / 2;
	} else if (g_FatType == 16) {
		fatIndex = currentCluster * 2;
	} else /*if (g_FatType == 32)*/ {
		fatIndex = currentCluster * 4;
	}

	// Make sure cache has the right number
	uint32_t fatIndexSector = fatIndex / SECTOR_SIZE;	///< FAT index sector size
	if (fatIndexSector < g_Data->FatCachePosition || fatIndexSector >= g_Data->FatCachePosition + FAT_CACHE_SIZE) {
		FAT_ReadFat(disk, fatIndexSector);
		g_Data->FatCachePosition = fatIndexSector;
	}

	fatIndex -= (g_Data->FatCachePosition * SECTOR_SIZE);

	uint32_t nextCluster;	///< Next cluster
	if (g_FatType == 12) {
		if (currentCluster % 2 == 0)
			nextCluster = (*(uint16_t*)(g_Data->FatCache + fatIndex)) & 0x0FFF;
		else
			nextCluster = (*(uint16_t*)(g_Data->FatCache + fatIndex)) >> 4;

		if (nextCluster >= 0xFF8) {
			nextCluster |= 0xFFFFF000;
		}
	} else if (g_FatType == 16) {
		nextCluster = *(uint16_t*)(g_Data->FatCache + fatIndex);
		if (nextCluster >= 0xFFF8) {
			nextCluster |= 0xFFFF0000;
		}
	} else /*if (g_FatType == 32)*/ {
		nextCluster = *(uint32_t*)(g_Data->FatCache + fatIndex);
	}

	return nextCluster;
}

/** Find file in directory by name
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @param file			Pointer to FAT_File to read from
 * @param name			Name of the file to find
 * @param entryOut		Pointer to FAT_DirectoryEntry to write to
 * @return bool			True if the file was found, false otherwise
 * Find file in directory by name and return directory entry data if found
*/
bool FAT_FindFile(Partition* disk, FAT_File* file, const char* name, FAT_DirectoryEntry* entryOut) {
	char fatName[12];	///< file name in FAT
	FAT_DirectoryEntry entry;

	// convert from name to fat name
	memset(fatName, ' ', sizeof(fatName));
	fatName[11] = '\0';

	const char* ext = strchr(name, '.'); ///< file extension
	if (ext == NULL)
		ext = name + 11;

	for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
		fatName[i] = toupper(name[i]);

	if (ext != name + 11) {
		for (int i = 0; i < 3 && ext[i + 1]; i++)
			fatName[i + 8] = toupper(ext[i + 1]);
	}

	while (FAT_ReadEntry(disk, file, &entry)) {
		if (memcmp(fatName, entry.Name, 11) == 0) {
			*entryOut = entry;
			return true;
		}
	}

	return false;
}

/** Open file by path
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @param path			Path to file to open
 * @return FAT_File*	Pointer to opened file
 * @since 0.0.1
 * @headerfile fat.h
 * Open file by path in FAT filesystem driver
*/
FAT_File* FAT_Open(Partition* disk, const char* path) {
	char name[MAX_PATH_SIZE];	///< file path

	// ignore leading slash
	if (path[0] == '/')
		path++;

	FAT_File* current = &g_Data->RootDirectory.Public;	///< current file

	while (*path) {
		// extract next file name from path
		bool isLast = false;	///< is last name in path (/name/name/last name)
		const char* delim = strchr(path, '/');	///< delimiter
		if (delim != NULL) {
			memcpy(name, path, delim - path);
			name[delim - path] = '\0';
			path = delim + 1;
		} else {
			unsigned len = strlen(path);	///< length of path
			memcpy(name, path, len);
			name[len + 1] = '\0';
			path += len;
			isLast = true;
		}

		// find directory entry in current directory
		FAT_DirectoryEntry entry;	///< directory entry
		if (FAT_FindFile(disk, current, name, &entry)) {
			FAT_Close(current);

			// check if directory
			if (!isLast && entry.Attributes & FAT_ATTRIBUTE_DIRECTORY == 0) {
				printf("FAT: %s is not a directory!\r\n", name);
				return NULL;
			}

			// open new directory entry
			current = FAT_OpenEntry(disk, &entry);
		} else {
			FAT_Close(current);

			printf("FAT: %s not found!\r\n", name);
			return NULL;
		}
	}

	return current;
}

/** Read file data
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @param file			Pointer to FAT_File to read from
 * @param byteCount		Number of bytes to read
 * @param dataOut		Pointer to data output
 * @return uint32_t		Number of bytes read
 * @since 0.0.1
 * @headerfile fat.h
 * Read file data from file handle
*/
uint32_t FAT_Read(Partition* disk, FAT_File* file, uint32_t byteCount, void* dataOut) {
	// get file data
	FAT_FileData* fd = (file->Handle == ROOT_DIRECTORY_HANDLE)	///< file data
		? &g_Data->RootDirectory
		: &g_Data->OpenedFiles[file->Handle];

	uint8_t* u8DataOut = (uint8_t*)dataOut;	///< standarized data output

	// don't read past the end of the file
	if (!fd->Public.IsDirectory || (fd->Public.IsDirectory && fd->Public.Size != 0))
		byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

	while (byteCount > 0) {
		uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
		uint32_t take = min(byteCount, leftInBuffer);

		memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
		u8DataOut += take;
		fd->Public.Position += take;
		byteCount -= take;

		// printf("leftInBuffer=%lu take=%lu\r\n", leftInBuffer, take);
		// See if there's need to read more data
		if (leftInBuffer == take) {
			// Special handling for root directory
			if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
				++fd->CurrentCluster;

				// read next sector
				if (!Partition_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
					printf("FAT: Failed to read root directory!\r\n");
					break;
				}
			} else {
				// calculate next cluster & sector to read
				if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster) {
					fd->CurrentSectorInCluster = 0;
					fd->CurrentCluster = FAT_NextCluster(disk, fd->CurrentCluster);
				}

				if (fd->CurrentCluster >= 0xFFFFFFF8) {
					// Mark end of file
					fd->Public.Size = fd->Public.Position;
					break;
				}

				// read next sector
				if (!Partition_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)) {
					printf("FAT: Failed to read file!\r\n");
					break;
				}
			}
		}
	}

	return u8DataOut - (uint8_t*)dataOut;
}

/** Read directory entry
 * @author @Kjur0
 * @version 0.1.1
 * @param disk			Pointer to Partition to read from
 * @param file			Pointer to FAT_File to read from
 * @param dirEntry		Pointer to FAT_DirectoryEntry to write to
 * @return bool			True if the directory entry was successfully read, false otherwise
 * @see FAT_Read()
 * @since 0.0.1
 * @headerfile fat.h
 * Read directory entry from file handle
*/
bool FAT_ReadEntry(Partition* disk, FAT_File* file, FAT_DirectoryEntry* dirEntry) {
	return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}

/** Close file handle
 * @author @Kjur0
 * @version 0.1.1
 * @param file			Pointer to FAT_File to close
 * @since 0.0.1
 * @headerfile fat.h
 * Close file handle by file pointer
*/
void FAT_Close(FAT_File* file) {
	if (file->Handle == ROOT_DIRECTORY_HANDLE) {
		file->Position = 0;
		g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
	} else {
		g_Data->OpenedFiles[file->Handle].Opened = false;
	}
}