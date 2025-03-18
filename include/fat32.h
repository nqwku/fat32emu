/**
 * @file fat32.h
 * @brief FAT32 filesystem implementation
 *
 * This header defines structures and functions for working with FAT32 filesystems,
 * including filesystem creation, navigation, file and directory operations.
 */

#ifndef FAT32_H
#define FAT32_H

#include "disk.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @defgroup FAT32_Constants FAT32 Constants
 * @{
 */
/** @brief FAT32 boot sector signature (0x55AA in little-endian) */
#define FAT32_SIGNATURE         0xAA55
/** @brief Cluster value indicating a free cluster */
#define FAT32_CLUSTER_FREE      0x00000000
/** @brief Cluster value indicating a reserved cluster */
#define FAT32_CLUSTER_RESERVED  0x00000001
/** @brief Cluster value indicating a bad cluster */
#define FAT32_CLUSTER_BAD       0x0FFFFFF7
/** @brief Cluster value indicating the end of a cluster chain */
#define FAT32_CLUSTER_END       0x0FFFFFFF
/** @brief Cluster number assigned to the root directory */
#define FAT32_ROOTDIR_CLUSTER   2

/**
 * @defgroup FAT32_Attributes FAT32 File/Directory Attributes
 * @{
 */
/** @brief Read-only attribute */
#define FAT32_ATTR_READ_ONLY    0x01
/** @brief Hidden attribute */
#define FAT32_ATTR_HIDDEN       0x02
/** @brief System attribute */
#define FAT32_ATTR_SYSTEM       0x04
/** @brief Volume ID attribute */
#define FAT32_ATTR_VOLUME_ID    0x08
/** @brief Directory attribute */
#define FAT32_ATTR_DIRECTORY    0x10
/** @brief Archive attribute */
#define FAT32_ATTR_ARCHIVE      0x20
/** @brief Long filename entry attribute combination */
#define FAT32_ATTR_LFN          (FAT32_ATTR_READ_ONLY | FAT32_ATTR_HIDDEN | FAT32_ATTR_SYSTEM | FAT32_ATTR_VOLUME_ID)
/** @} */

/**
 * @brief FAT32 Boot Sector Structure
 *
 * Contains the BIOS Parameter Block (BPB) and Extended BIOS Parameter Block (EBPB)
 * fields that define the filesystem structure and parameters.
 */
typedef struct {
    uint8_t BS_jmpBoot[3];      /**< Jump instruction to boot code */
    char BS_OEMName[8];         /**< OEM identifier */
    uint16_t BPB_BytesPerSec;   /**< Bytes per sector */
    uint8_t BPB_SecPerClus;     /**< Sectors per cluster */
    uint16_t BPB_RsvdSecCnt;    /**< Reserved sector count */
    uint8_t BPB_NumFATs;        /**< Number of FAT copies */
    uint16_t BPB_RootEntCnt;    /**< Root entry count (0 for FAT32) */
    uint16_t BPB_TotSec16;      /**< Total sectors (16-bit, 0 for FAT32) */
    uint8_t BPB_Media;          /**< Media type*/
    uint16_t BPB_FATSz16;       /**< FAT size in sectors (16-bit, 0 for FAT32) */
    uint16_t BPB_SecPerTrk;     /**< Sectors per track */
    uint16_t BPB_NumHeads;      /**< Number of heads */
    uint32_t BPB_HiddSec;       /**< Hidden sectors */
    uint32_t BPB_TotSec32;      /**< Total sectors (32-bit)*/

    /* FAT32 specific fields*/
    uint32_t BPB_FATSz32;       /**< FAT size in sectors (32-bit) */
    uint16_t BPB_ExtFlags;      /**< Extended flags */
    uint16_t BPB_FSVer;         /**< Filesystem version */
    uint32_t BPB_RootClus;      /**< Root directory cluster */
    uint16_t BPB_FSInfo;        /**< FSInfo structure sector */
    uint16_t BPB_BkBootSec;     /**< Backup boot sector */
    uint8_t  BPB_Reserved[12];  /**< Reserved */
    uint8_t  BS_DrvNum;         /**< Drive number */
    uint8_t  BS_Reserved1;      /**< Reserved */
    uint8_t  BS_BootSig;        /**< Boot signature (0x29) */
    uint32_t BS_VolID;          /**< Volume ID */
    char BS_VolLab[11];         /**< Volume label */
    char BS_FilSysType[8];      /**< Filesystem type label */
    uint8_t BootCode[420];      /**< Boot code */
    uint16_t BootSignature;     /**< Boot sector signature (0xAA55) */
} __attribute__((packed)) FAT32_BootSector;

/**
 * @brief FAT32 Directory Entry Structure
 *
 * Represents a single directory entry in a FAT32 filesystem.
 */
typedef struct {
    char DIR_Name[11];          /**< Short filename (8.3 format) */
    uint8_t DIR_Attr;           /**< File attributes */
    uint8_t DIR_NTRes;          /**< Reserved for Windows NT */
    uint8_t DIR_CrtTimeTenth;   /**< Creation time (tenths of second) */
    uint16_t DIR_CrtTime;       /**< Creation time (hours, minutes, seconds) */
    uint16_t DIR_CrtDate;       /**< Creation date */
    uint16_t DIR_LstAccDate;    /**< Last access date */
    uint16_t DIR_FstClusHI;     /**< High 16 bits of first cluster */
    uint16_t DIR_WrtTime;       /**< Last write time */
    uint16_t DIR_WrtDate;       /**< Last write date */
    uint16_t DIR_FstClusLO;     /**< Low 16 bits of first cluster */
    uint32_t DIR_FileSize;      /**< File size in bytes */
} __attribute__((packed)) FAT32_DirEntry;

/**
 * @brief FAT32 Long Filename Entry Structure
 *
 * Represents a long filename entry in the FAT32 filesystem.
 */
typedef struct {
    uint8_t LDIR_0rd;           /**< Sequence number */
    uint16_t LDIR_Name1[5];     /**< First 5 Unicode characters */
    uint8_t LDIR_Attr;          /**< Attributes (always 0x0F) */
    uint8_t LDIR_Type;          /**< Entry type (0 for LFN) */
    uint8_t LDIR_Chksum;        /**< Checksum of short name */
    uint8_t LDIR_Name2[6];      /**< Next 6 Unicode characters */
    uint16_t LDIR_FstClusL0;    /**< First cluster (always 0 for LFN) */
    uint16_t LDIR_Name3[2];     /**< Last 2 Unicode characters */
} __attribute__((packed)) FAT32_LFNEntry;

/**
 * @brief FAT32 Filesystem Structure
 *
 * Contains all the information needed to work with a FAT32 filesystem.
 */
typedef struct {
    Disk disk;                  /**< Underlying disk interface */
    FAT32_BootSector bootSector; /**< Boot sector data */
    uint32_t *fat;              /**< File Allocation Table */
    uint32_t fat_size;          /**< Size of FAT in sectors */
    uint32_t sectors_per_cluster; /**< Number of sectors per cluster */
    uint32_t first_data_sector; /**< First sector of the data region */
    uint32_t data_cluster_count; /**< Number of data clusters */
    uint32_t bytes_per_cluster; /**< Number of bytes per cluster */
    uint32_t current_dir_cluster; /**< Current directory cluster */
    char current_path[256];     /**< Current directory path */
    bool is_formatted;          /**< Whether the filesystem is formatted */
} FAT32_FileSystem;

/**
 * @brief Initialize a FAT32 filesystem
 *
 * Opens a disk image file and initializes the FAT32 filesystem structure.
 * Detects if the disk is already formatted as FAT32.
 *
 * @param fs Pointer to the filesystem structure to initialize
 * @param filename Path to the disk image file
 * @return true if initialization was successful, false otherwise
 */
bool fat32_init(FAT32_FileSystem *fs, const char *filename);

/**
 * @brief Format a disk as FAT32
 *
 * Creates a new FAT32 filesystem on the disk.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if formatting was successful, false otherwise
 */
bool fat32_format(FAT32_FileSystem *fs);

/**
 * @brief Check if a filesystem is a valid FAT32 filesystem
 *
 * Verifies various filesystem parameters to ensure it's a valid FAT32 filesystem.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if the filesystem is valid, false otherwise
 */
bool fat32_check_fs(FAT32_FileSystem *fs);

/**
 * @brief List the contents of a directory
 *
 * Retrieves directory entries from the specified path or current directory.
 *
 * @param fs Pointer to the filesystem structure
 * @param path Path to the directory to list, or NULL for current directory
 * @param entries Array to store the directory entries
 * @param max_entries Maximum number of entries to retrieve
 * @param count Pointer to store the number of entries retrieved
 * @return true if the operation was successful, false otherwise
 */
bool fat32_list_directory(FAT32_FileSystem *fs, const char *path,
                          FAT32_DirEntry *entries,
                          uint32_t max_entries, uint32_t *count);

/**
 * @brief Change the current directory
 *
 * Navigates to the specified directory path.
 *
 * @param fs Pointer to the filesystem structure
 * @param path Path to the target directory
 * @return true if the operation was successful, false otherwise
 */
bool fat32_change_directory(FAT32_FileSystem *fs, const char *path);

/**
 * @brief Create a new directory
 *
 * Creates a new directory in the current directory.
 *
 * @param fs Pointer to the filesystem structure
 * @param name Name of the directory to create
 * @return true if the operation was successful, false otherwise
 */
bool fat32_create_directory(FAT32_FileSystem *fs, const char *name);

/**
 * @brief Create a new empty file
 *
 * Creates a new empty file in the current directory.
 *
 * @param fs Pointer to the filesystem structure
 * @param name Name of the file to create
 * @return true if the operation was successful, false otherwise
 */
bool fat32_create_file(FAT32_FileSystem *fs, const char *name);

/**
 * @brief Close a FAT32 filesystem
 *
 * Frees all allocated resources and closes the disk.
 *
 * @param fs Pointer to the filesystem structure
 */
void fat32_close(FAT32_FileSystem *fs);

/**
 * @brief Read the boot sector from disk
 *
 * Loads the boot sector data from the disk into memory.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if the operation was successful, false otherwise
 */
bool fat32_read_boot_sector(FAT32_FileSystem *fs);

/**
 * @brief Write the boot sector to disk
 *
 * Writes the boot sector data from memory to the disk.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if the operation was successful, false otherwise
 */
bool fat32_write_boot_sector(FAT32_FileSystem *fs);

/**
 * @brief Read the FAT from disk
 *
 * Loads the File Allocation Table from the disk into memory.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if the operation was successful, false otherwise
 */
bool fat32_read_fat(FAT32_FileSystem *fs);

/**
 * @brief Write the FAT to disk
 *
 * Writes the File Allocation Table from memory to the disk.
 *
 * @param fs Pointer to the filesystem structure
 * @return true if the operation was successful, false otherwise
 */
bool fat32_write_fat(FAT32_FileSystem *fs);

/**
 * @brief Get the next cluster in a cluster chain
 *
 * Retrieves the next cluster number from the FAT for a given cluster.
 *
 * @param fs Pointer to the filesystem structure
 * @param cluster Current cluster number
 * @return Next cluster number, or FAT32_CLUSTER_END if end of chain
 */
uint32_t fat32_get_next_cluster(FAT32_FileSystem *fs, uint32_t cluster);

/**
 * @brief Allocate a new cluster
 *
 * Finds a free cluster in the FAT and marks it as the end of a chain.
 *
 * @param fs Pointer to the filesystem structure
 * @return The allocated cluster number, or 0 if allocation failed
 */
uint32_t fat32_allocate_cluster(FAT32_FileSystem *fs);

/**
 * @brief Set a value in the FAT for a given cluster
 *
 * Updates the FAT entry for a cluster with a new value.
 *
 * @param fs Pointer to the filesystem structure
 * @param cluster Cluster number to update
 * @param value New value for the cluster
 * @return true if the operation was successful, false otherwise
 */
bool fat32_set_cluster_value(FAT32_FileSystem *fs, uint32_t cluster, uint32_t value);

/**
 * @brief Read a cluster from disk
 *
 * Reads the contents of a cluster from the disk into a buffer.
 *
 * @param fs Pointer to the filesystem structure
 * @param cluster Cluster number to read
 * @param buffer Buffer to store the cluster data
 * @return true if the operation was successful, false otherwise
 */
bool fat32_read_cluster(FAT32_FileSystem *fs, uint32_t cluster, void *buffer);

/**
 * @brief Write a cluster to disk
 *
 * Writes data from a buffer to a cluster on the disk.
 *
 * @param fs Pointer to the filesystem structure
 * @param cluster Cluster number to write
 * @param buffer Buffer containing the data to write
 * @return true if the operation was successful, false otherwise
 */
bool fat32_write_cluster(FAT32_FileSystem *fs, uint32_t cluster, const void *buffer);

/**
 * @brief Calculate the first sector of a cluster
 *
 * Converts a cluster number to the corresponding sector number on disk.
 *
 * @param fs Pointer to the filesystem structure
 * @param cluster Cluster number
 * @return First sector number of the cluster
 */
uint32_t fat32_sector_for_cluster(FAT32_FileSystem *fs, uint32_t cluster);

#endif //FAT32_H
