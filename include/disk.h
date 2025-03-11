#ifndef DISK_H
#define DISK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define DISK_DEFAULT_SIZE (20 * 1024 * 1024)
#define DISK_SECTOR_SIZE 512

typedef struct {
    FILE *file;
    char *filename;
    uint32_t total_sectors;
} Disk;

bool disk_init(Disk *disk, const char *filename);

bool disk_read_sector(Disk *disk, uint32_t sector, void *buffer);
bool disk_write_sector(Disk *disk, uint32_t sector, const void *buffer);

bool disk_read_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, void *buffer);
bool disk_write_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, const void *buffer);

uint32_t disk_get_total_sectors(Disk *disk);

void disk_close(Disk *disk);

#endif //DISK_H
