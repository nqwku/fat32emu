#include "../include/disk.h"
#include <stdlib.h>
#include <string.h>

bool disk_init(Disk *disk, const char *filename) {

    if (!disk || !filename) {
        return false;
    }

    disk->filename = strdup(filename);
    if (!disk->filename) {
        return false;
    }

    disk->file = fopen(disk->filename, "r+b");
    if (!disk->file) {
        disk->file = fopen(disk->filename, "w+b");
        if (!disk->file) {
            free(disk->filename);
            disk->filename = NULL;
            return false;
        }

        uint8_t *empty_sector = calloc(1, DISK_SECTOR_SIZE);
        if (!empty_sector) {
            fclose(disk->file);
            free(disk->filename);
            disk->filename = NULL;
            return false;
        }

        uint32_t sectors = DISK_DEFAULT_SIZE / DISK_SECTOR_SIZE;

        for (uint32_t i = 0; i < sectors; i++) {
            if (fwrite(empty_sector, DISK_SECTOR_SIZE, 1, disk->file) != 1) {
                free(empty_sector);
                fclose(disk->file);
                free(disk->filename);
                disk->filename = NULL;
                return false;
            }
        }
        free(empty_sector);

        fflush(disk->file);
        disk->total_sectors = sectors;
    } else {
        fseek(disk->file, 0, SEEK_END);
        long file_size = ftell(disk->file);
        fseek(disk->file, 0, SEEK_SET);

        disk->total_sectors = file_size / DISK_SECTOR_SIZE;
    }

    return true;
}

bool disk_read_sector(Disk *disk, uint32_t sector_num, void *buffer) {
    if (!disk || !disk->file || !buffer
              || sector_num >= disk->total_sectors) {
        return false;
    }

    if (fseek(disk->file, sector_num * DISK_SECTOR_SIZE, SEEK_SET) != 0) {
        return false;
    }

    size_t read_count = fread(buffer, DISK_SECTOR_SIZE, 1, disk->file);
    return read_count == 1;
}

bool disk_write_sector(Disk *disk, uint32_t sector_num, const void *buffer) {

    if (!disk || !disk->file || !buffer
              || sector_num >= disk->total_sectors) {
        return false;
    }

    if (fseek(disk->file, sector_num * DISK_SECTOR_SIZE, SEEK_SET) != 0) {
        return false;
    }

    size_t write_count = fwrite(buffer, DISK_SECTOR_SIZE, 1, disk->file);

    if (write_count == 1) {
        fflush(disk->file);
        return true;
    }
    return false;
}

bool disk_read_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, void *buffer) {
    if (!disk || !disk->file || !buffer
              || start_sector >= disk->total_sectors
              || start_sector + sector_count > disk->total_sectors) {
        return false;
    }

    if (fseek(disk->file, start_sector * DISK_SECTOR_SIZE, SEEK_SET) != 0) {
        return false;
    }

    size_t read_count = fread(buffer, DISK_SECTOR_SIZE, sector_count, disk->file);
    return read_count == sector_count;
}

bool disk_write_sectors(Disk *disk, uint32_t start_sector, uint32_t sector_count, const void *buffer) {
    if (!disk || !disk->file || !buffer
        || start_sector >= disk->total_sectors
        || start_sector + sector_count > disk->total_sectors) {
        return false;
    }

    if (fseek(disk->file, start_sector * DISK_SECTOR_SIZE, SEEK_SET) != 0) {
        return false;
    }

    size_t write_count = fwrite(buffer, DISK_SECTOR_SIZE, sector_count, disk->file);

    if (write_count == sector_count) {
        fflush(disk->file);
        return true;
    }

    return false;
}

uint32_t disk_get_total_sectors(Disk *disk) {
    if (!disk) {
        return 0;
    }
    return disk->total_sectors;
}

void disk_close(Disk *disk) {
    if (!disk) {
        return;
    }

    if (disk->file) {
        fclose(disk->file);
        disk->file = NULL;
    }

    if (disk->filename) {
        free(disk->filename);
        disk->filename = NULL;
    }

    disk->total_sectors = 0;
}