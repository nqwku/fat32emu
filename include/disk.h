/**
 * @file disk.h
 * @brief Disk emulation layer for filesystem operations
 *
 * This header provides an abstraction layer for disk operations,
 * allowing the filesystem to interact with a file as if it were a physical disk.
 * It handles sector-based read and write operations for both single and multiple sectors.
 */

#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/** @brief Default size for a new disk (20 MB) */
#define DISK_DEFAULT_SIZE (20 * 1024 * 1024)
/** @brief Size of each disk sector in bytes */
#define DISK_SECTOR_SIZE 512

/**
 * @brief Disk structure representing a virtual disk
 *
 * Contains the information needed to access and manage a disk image file.
 */
typedef struct {
    FILE *file;            /**< File handle for the disk image */
    char *filename;        /**< Path to the disk image file */
    uint32_t total_sectors; /**< Total number of sectors on the disk */
} Disk;

/**
 * @brief Initialize a disk
 *
 * Opens an existing disk image file or creates a new one if it doesn't exist.
 * If creating a new disk, it will be initialized with zeros to the default size.
 *
 * @param disk Pointer to the disk structure to initialize
 * @param filename Path to the disk image file
 * @return true if initialization was successful, false otherwise
 */
bool disk_init(Disk *disk, const char *filename);

/**
 * @brief Read a single sector from the disk
 *
 * Reads data from the specified sector into the provided buffer.
 *
 * @param disk Pointer to the disk structure
 * @param sector Sector number to read from
 * @param buffer Buffer to store the read data (must be at least DISK_SECTOR_SIZE bytes)
 * @return true if the read operation was successful, false otherwise
 */
bool disk_read_sector(Disk *disk, uint32_t sector, void *buffer);

/**
 * @brief Write a single sector to the disk
 *
 * Writes data from the provided buffer to the specified sector.
 *
 * @param disk Pointer to the disk structure
 * @param sector Sector number to write to
 * @param buffer Buffer containing the data to write (must be at least DISK_SECTOR_SIZE bytes)
 * @return true if the write operation was successful, false otherwise
 */
bool disk_write_sector(Disk *disk, uint32_t sector, const void *buffer);

/**
 * @brief Read multiple contiguous sectors from the disk
 *
 * Reads data from the specified range of sectors into the provided buffer.
 *
 * @param disk Pointer to the disk structure
 * @param start_sector First sector number to read from
 * @param sector_count Number of sectors to read
 * @param buffer Buffer to store the read data (must be at least sector_count * DISK_SECTOR_SIZE bytes)
 * @return true if the read operation was successful, false otherwise
 */
bool disk_read_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, void *buffer);

/**
 * @brief Write multiple contiguous sectors to the disk
 *
 * Writes data from the provided buffer to the specified range of sectors.
 *
 * @param disk Pointer to the disk structure
 * @param start_sector First sector number to write to
 * @param sector_count Number of sectors to write
 * @param buffer Buffer containing the data to write (must be at least sector_count * DISK_SECTOR_SIZE bytes)
 * @return true if the write operation was successful, false otherwise
 */
bool disk_write_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, const void *buffer);

/**
 * @brief Get the total number of sectors on the disk
 *
 * @param disk Pointer to the disk structure
 * @return Number of sectors on the disk, or 0 if the disk is invalid
 */
uint32_t disk_get_total_sectors(Disk *disk);

/**
 * @brief Close a disk and free associated resources
 *
 * Closes the disk image file and frees any allocated memory.
 *
 * @param disk Pointer to the disk structure to close
 */
void disk_close(Disk *disk);

#endif /* DISK_H */