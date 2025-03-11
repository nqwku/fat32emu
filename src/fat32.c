#include "../include/fat32.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint16_t get_fat_date() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    uint16_t year = tm->tm_year - 80;
    uint16_t month = tm->tm_mon + 1;
    uint16_t day = tm->tm_mday;

    return(year << 9) | (month << 5) | day;
}

static uint16_t get_fat_time() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    uint16_t hour = tm->tm_hour;
    uint16_t minute = tm->tm_min;
    uint16_t second = tm->tm_sec / 2;

    return (hour << 11) | (minute << 5) | second;
}

static void convert_to_short_name(char *short_name, const char *name) {
    memset(short_name, ' ', 11);

    int i = 0, j = 0;

    while (name[i] && name[i] != '.' && j < 8) {
        char c = name[i++];
        short_name[j++] = c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
    }

    while (name[i] && name[i] != '.') {
        i++;
    }

    if (name[i] == '.') {
        i++;
        j = 8;

        while (name[i] && j < 11) {
            char c = name[i++];
            short_name[j++] = c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
        }
    }
}

static int find_entry_by_name(FAT32_FileSystem *fs,
    uint32_t dir_cluster, const char *name) {
    uint8_t *cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
    if (!cluster_data) {
        return -1;
    }

    char short_name[11];
    convert_to_short_name(short_name, name);

    uint32_t current_cluster = dir_cluster;
    int entry_index = -1;
    int current_offset = 0;

    while (current_cluster >= 2 && current_cluster < FAT32_CLUSTER_END) {
        if (!fat32_read_cluster(fs, current_cluster, cluster_data)) {
            free(cluster_data);
            return -1;
        }

        FAT32_DirEntry * entries = (FAT32_DirEntry*)cluster_data;
        uint32_t entries_per_cluster = fs->bytes_per_cluster / sizeof(FAT32_DirEntry);
        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].DIR_Name[0] == 0x00 || entries[i].DIR_Name[0] == 0xE5) {
                continue;
            }

            if (memcmp(entries[i].DIR_Name, short_name, 11) == 0) {
                entry_index = current_offset + i;
                break;
            }
        }
        if (entry_index >= 0) {
            break;
        }

        current_cluster = fat32_get_next_cluster(fs, current_cluster);
        current_offset += entries_per_cluster;
    }

    free(cluster_data);
    return entry_index;
}

static int find_free_entry(FAT32_FileSystem *fs,
    uint32_t dir_cluster, uint32_t *out_cluster) {
    uint8_t *cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
    if (!cluster_data) {
        return -1;
    }

    uint32_t current_cluster = dir_cluster;
    int entry_index = -1;

    while (current_cluster >= 2 && current_cluster < FAT32_CLUSTER_END) {
        if (!fat32_read_cluster(fs, current_cluster, cluster_data)) {
            free(cluster_data);
            return -1;
        }
        FAT32_DirEntry *entries = (FAT32_DirEntry*)cluster_data;
        uint32_t entries_per_cluster = fs->bytes_per_cluster / sizeof(FAT32_DirEntry);

        for (uint32_t i = 0; i < entries_per_cluster; i++) {
            if (entries[i].DIR_Name[0] == 0x00 || entries[i].DIR_Name[0] == 0xE5) {
                entry_index = i;
                *out_cluster = current_cluster;
                break;
            }
        }

        if (entry_index >= 0) {
            break;
        }

        uint32_t next_cluster = fat32_get_next_cluster(fs, current_cluster);

        if (next_cluster >= FAT32_CLUSTER_END) {
            uint32_t new_cluster = fat32_allocate_cluster(fs);
            if (new_cluster == 0) {
                free(cluster_data);
                return -1;
            }

            memset(cluster_data, 0, fs->bytes_per_cluster);
            if (!fat32_write_cluster(fs, new_cluster, cluster_data)) {
                free(cluster_data);
                return -1;
            }

            fat32_set_cluster_value(fs, current_cluster, new_cluster);

            fat32_set_cluster_value(fs, new_cluster, FAT32_CLUSTER_END);

            entry_index = 0;
            *out_cluster = new_cluster;
            break;
        }
        current_cluster = next_cluster;
    }
    free(cluster_data);
    return entry_index;
}

static bool parse_path(const char *path, char components[][13], int *component_count) {
    if (!path || path[0] != '/')
        return false;

    *component_count = 0;

    path++;

    char temp[13];
    int temp_index = 0;

    while (*path) {
        if (*path == '/') {
            if (temp_index > 0) {
                temp[temp_index] = '\0';
                strcpy(components[*component_count], temp);
                (*component_count)++;
                temp_index = 0;
            }
        } else if (temp_index < 12) {
            temp[temp_index++] = *path;
        } else {
            return false;
        }

        path++;
    }

    if (temp_index > 0) {
        temp[temp_index] = '\0';
        strcpy(components[*component_count], temp);
        (*component_count)++;
    }

    return true;
}

bool fat32_init(FAT32_FileSystem *fs, const char *filename) {
    if (!fs || !filename) {
        return false;
    }

    if (!disk_init(&fs->disk, filename)) {
        return false;
    }

    strcpy(fs->current_path, "/");

    if (!fat32_read_boot_sector(fs)) {
        fs->is_formatted = false;
        return true;
    }

    if (!fat32_check_fs(fs)) {
        fs->is_formatted = false;
        return true;
    }

    fs->sectors_per_cluster = fs->bootSector.BPB_SecPerClus;
    fs->fat_size = fs->bootSector.BPB_FATSz32;
    fs->bytes_per_cluster = fs->bootSector.BPB_BytesPerSec * fs->sectors_per_cluster;

    fs->first_data_sector = fs->bootSector.BPB_RsvdSecCnt +
                            (fs->bootSector.BPB_NumFATs * fs->fat_size);

    uint32_t total_sectors = fs->bootSector.BPB_TotSec32;
    uint32_t data_sectors = total_sectors - fs->first_data_sector;
    fs->data_cluster_count = data_sectors / fs->sectors_per_cluster;

    if (!fat32_read_fat(fs)) {
        fs->is_formatted = false;
        return true;
    }

    fs->current_dir_cluster = fs->bootSector.BPB_RootClus;
    fs->is_formatted = true;

    return true;
}

bool fat32_read_boot_sector(FAT32_FileSystem *fs) {
    if (!fs) {
        return false;
    }

    return disk_read_sector(&fs->disk, 0, &fs->bootSector);
}

bool fat32_write_boot_sector(FAT32_FileSystem *fs) {
    if (!fs) {
        return false;
    }

    return disk_write_sector(&fs->disk, 0, &fs->bootSector);
}

bool fat32_check_fs(FAT32_FileSystem *fs) {
    if (!fs) {
        return false;
    }

    if (fs->bootSector.BootSignature != FAT32_SIGNATURE) {
        return false;
    }

    if (strncmp(fs->bootSector.BS_FilSysType, "FAT32   ", 8) != 0) {
        return false;
    }

    if (fs->bootSector.BPB_BytesPerSec != 512 ||
        fs->bootSector.BPB_SecPerClus == 0 ||
        fs->bootSector.BPB_NumFATs == 0 ||
        fs->bootSector.BPB_FATSz32 == 0) {
        return false;
    }

    return true;
}

bool fat32_read_fat(FAT32_FileSystem *fs) {
    if (!fs || !fs->is_formatted) {
        return false;
    }

    uint32_t fat_size_bytes = fs->fat_size * fs->bootSector.BPB_BytesPerSec;

    fs->fat = (uint32_t*)malloc(fat_size_bytes);
    if (!fs->fat) {
        return false;
    }

    uint32_t fat_start_sector = fs->bootSector.BPB_RsvdSecCnt;
    return disk_read_sectors(&fs->disk, fat_start_sector, fs->fat_size, fs->fat);
}

bool fat32_write_fat(FAT32_FileSystem *fs) {
    if (!fs || !fs->is_formatted) {
        return false;
    }

    uint32_t fat_start_sector = fs->bootSector.BPB_RsvdSecCnt;
    bool success = disk_write_sectors(&fs->disk, fat_start_sector, fs->fat_size, fs->fat);

    if (success && fs->bootSector.BPB_NumFATs > 1) {
        for (uint8_t i = 1; i < fs->bootSector.BPB_NumFATs; i++) {
            uint32_t fat_copy_start = fat_start_sector + (i * fs->fat_size);
            success = disk_write_sectors(&fs->disk, fat_copy_start, fs->fat_size, fs->fat);
            if (!success) {
                break;
            }
        }
    }
    return success;
}

uint32_t fat32_get_next_cluster(FAT32_FileSystem *fs, uint32_t cluster) {
    if (!fs || !fs->fat || !fs->is_formatted || cluster < 2 || cluster >= fs->data_cluster_count + 2) {
        return FAT32_CLUSTER_END;
    }

    return fs->fat[cluster] & 0x0FFFFFFF;
}

uint32_t fat32_allocate_cluster(FAT32_FileSystem *fs) {
    if (!fs || !fs->fat || !fs->is_formatted) {
        return 0;
    }

    for (uint32_t i = 2; i < fs->data_cluster_count + 2; i++) {
        if (fs->fat[i] == FAT32_CLUSTER_FREE) {
            fs->fat[i] = FAT32_CLUSTER_END;

            fat32_write_fat(fs);

            return i;
        }
    }

    return 0;
}

bool fat32_set_cluster_value(FAT32_FileSystem *fs, uint32_t cluster, uint32_t value) {
    if (!fs || !fs->fat || !fs->is_formatted || cluster < 2 || cluster >= fs->data_cluster_count + 2) {
        return false;
    }

    fs->fat[cluster] = value & 0x0FFFFFFF;
    return fat32_write_fat(fs);
}

uint32_t fat32_sector_for_cluster(FAT32_FileSystem *fs, uint32_t cluster) {
    if (!fs || !fs->is_formatted || cluster < 2 ) {
        return 0;
    }

    return fs->first_data_sector + (cluster + 2) * fs->sectors_per_cluster;
}

bool fat32_read_cluster(FAT32_FileSystem *fs, uint32_t cluster, void *buffer) {
    if (!fs || !buffer || !fs->is_formatted || cluster < 2 ) {
        return false;
    }

    uint32_t first_sector = fat32_sector_for_cluster(fs, cluster);
    return disk_read_sectors(&fs->disk, first_sector, fs->sectors_per_cluster, buffer);
}

bool fat32_write_cluster(FAT32_FileSystem *fs, uint32_t cluster, const void *buffer) {
    if (!fs || !buffer || !fs->is_formatted || cluster < 2) {
        return false;
    }

    uint32_t first_sector = fat32_sector_for_cluster(fs, cluster);
    return disk_write_sectors(&fs->disk, first_sector, fs->sectors_per_cluster, buffer);
}


bool fat32_format(FAT32_FileSystem *fs) {
    if (!fs) {
        return false;
    }

    memset(&fs->bootSector, 0, sizeof(FAT32_BootSector));

    fs->bootSector.BS_jmpBoot[0] = 0xEB;
    fs->bootSector.BS_jmpBoot[1] = 0x58;
    fs->bootSector.BS_jmpBoot[2] = 0x90;

    memcpy(fs->bootSector.BS_OEMName, "MSWIN4.1", 8);
    fs->bootSector.BPB_BytesPerSec = 512;
    fs->bootSector.BPB_SecPerClus = 4;
    fs->bootSector.BPB_RsvdSecCnt = 32;
    fs->bootSector.BPB_NumFATs = 2;
    fs->bootSector.BPB_RootEntCnt = 0;
    fs->bootSector.BPB_TotSec16 = 0;
    fs->bootSector.BPB_Media = 0xF8;
    fs->bootSector.BPB_FATSz16 = 0;
    fs->bootSector.BPB_SecPerTrk = 63;
    fs->bootSector.BPB_NumHeads = 255;
    fs->bootSector.BPB_HiddSec = 0;

    uint32_t total_sectors = disk_get_total_sectors(&fs->disk);
    fs->bootSector.BPB_TotSec32 = total_sectors;

    uint32_t data_sectors = total_sectors - fs->bootSector.BPB_RsvdSecCnt;
    uint32_t clusters = data_sectors / fs->bootSector.BPB_SecPerClus;

    uint32_t fat_size = ((clusters * 4) + 512 - 1) / 512;

    data_sectors = total_sectors - fs->bootSector.BPB_RsvdSecCnt - (fat_size * fs->bootSector.BPB_NumFATs);
    clusters = data_sectors / fs->bootSector.BPB_SecPerClus;
    fat_size = ((clusters * 4) + 512 - 1) / 512;

    fs->bootSector.BPB_FATSz32 = fat_size;
    fs->bootSector.BPB_ExtFlags = 0;
    fs->bootSector.BPB_FSVer = 0;
    fs->bootSector.BPB_RootClus = FAT32_ROOTDIR_CLUSTER;
    fs->bootSector.BPB_FSInfo = 1;
    fs->bootSector.BPB_BkBootSec = 6;
    memset(fs->bootSector.BPB_Reserved, 0, 12);

    fs->bootSector.BS_DrvNum = 0x80;
    fs->bootSector.BS_Reserved1 = 0;
    fs->bootSector.BS_BootSig = 0x29;
    fs->bootSector.BS_VolID = (uint32_t)time(NULL);
    memcpy(fs->bootSector.BS_VolLab, "NO NAME    ", 11);
    memcpy(fs->bootSector.BS_FilSysType, "FAT32   ", 8);

    memset(fs->bootSector.BootCode, 0, 420);
    fs->bootSector.BootSignature = FAT32_SIGNATURE;

    if (!fat32_write_boot_sector(fs)) {
        return false;
    }

    fs->sectors_per_cluster = fs->bootSector.BPB_SecPerClus;
    fs->fat_size = fs->bootSector.BPB_FATSz32;
    fs->bytes_per_cluster = fs->bootSector.BPB_BytesPerSec * fs->sectors_per_cluster;
    fs->first_data_sector = fs->bootSector.BPB_RsvdSecCnt +
                            (fs->bootSector.BPB_NumFATs * fs->fat_size);

    uint32_t data_sector_count = total_sectors - fs->first_data_sector;
    fs->data_cluster_count = data_sector_count / fs->sectors_per_cluster;

    if (fs->fat) {
        free(fs->fat);
    }

    uint32_t fat_size_bytes = fs->fat_size * fs->bootSector.BPB_BytesPerSec;
    fs->fat = (uint32_t*)calloc(1, fat_size_bytes);
    if (!fs->fat) {
        return false;
    }

    fs->fat[0] = 0x0FFFFF00 | fs->bootSector.BPB_Media;
    fs->fat[1] = 0x0FFFFFFF;

    fs->fat[FAT32_ROOTDIR_CLUSTER] = FAT32_CLUSTER_END;

    if (!fat32_write_fat(fs)) {
        free(fs->fat);
        fs->fat = NULL;
        return false;
    }

    uint8_t *root_dir = (uint8_t*)calloc(1, fs->bytes_per_cluster);
    if (!root_dir) {
        free(fs->fat);
        fs->fat = NULL;
        return false;
    }

    FAT32_DirEntry *dir_entries = (FAT32_DirEntry*)root_dir;

    memset(dir_entries[0].DIR_Name, ' ', 11);
    dir_entries[0].DIR_Name[0] = '.';
    dir_entries[0].DIR_Attr = FAT32_ATTR_DIRECTORY;
    dir_entries[0].DIR_CrtTime = get_fat_time();
    dir_entries[0].DIR_CrtDate = get_fat_date();
    dir_entries[0].DIR_LstAccDate = get_fat_date();
    dir_entries[0].DIR_WrtTime = get_fat_time();
    dir_entries[0].DIR_WrtDate = get_fat_date();
    dir_entries[0].DIR_FstClusHI = (FAT32_ROOTDIR_CLUSTER >> 16) & 0xFFFF;
    dir_entries[0].DIR_FstClusLO = FAT32_ROOTDIR_CLUSTER & 0xFFFF;
    dir_entries[0].DIR_FileSize = 0;

    memset(dir_entries[1].DIR_Name, ' ', 11);
    dir_entries[1].DIR_Name[0] = '.';
    dir_entries[1].DIR_Name[1] = '.';
    dir_entries[1].DIR_Attr = FAT32_ATTR_DIRECTORY;
    dir_entries[1].DIR_CrtTime = get_fat_time();
    dir_entries[1].DIR_CrtDate = get_fat_date();
    dir_entries[1].DIR_LstAccDate = get_fat_date();
    dir_entries[1].DIR_WrtTime = get_fat_time();
    dir_entries[1].DIR_WrtDate = get_fat_date();
    dir_entries[1].DIR_FstClusHI = (FAT32_ROOTDIR_CLUSTER >> 16) & 0xFFFF;
    dir_entries[1].DIR_FstClusLO = FAT32_ROOTDIR_CLUSTER & 0xFFFF;
    dir_entries[1].DIR_FileSize = 0;

    if (!fat32_write_cluster(fs, FAT32_ROOTDIR_CLUSTER, root_dir)){
        free(root_dir);
        free(fs->fat);
        fs->fat = NULL;
        return false;
    }

    free(root_dir);

    fs->current_dir_cluster = FAT32_ROOTDIR_CLUSTER;
    strcpy(fs->current_path, "/");
    fs->is_formatted = true;

    return true;
}

bool fat32_change_directory(FAT32_FileSystem *fs, const char *path) {
    if (!fs || !fs->is_formatted || !path) {
        return false;
    }

    if (path[0] != '/') {
        return false;
    }

    if (strcmp(path[0], "/") == 0) {
        fs->current_dir_cluster = fs->bootSector.BPB_RootClus;
        strcpy(fs->current_path, "/");
        return true;
    }

    char path_components[256][13];
    int path_components_count = 0;

    if (!parse_path(path, path_components, &path_components_count)) {
        return false;
    }

    uint32_t dir_cluster = fs->bootSector.BPB_RootClus;

    for (int i = 0; i < path_components_count; i++) {
        int entry_index = find_entry_by_name(fs, dir_cluster, path_components[i]);
        if (entry_index < 0) {
            return false;
        }

        uint8_t *cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
        if (!cluster_data) {
            return false;
        }

        uint32_t current_cluster = dir_cluster;
        while (entry_index >= fs->bytes_per_cluster / sizeof(FAT32_DirEntry)) {
            entry_index -= fs->bytes_per_cluster / sizeof(FAT32_DirEntry);
            current_cluster = fat32_get_next_cluster(fs, current_cluster);
        }

        if (!fat32_read_cluster(fs, current_cluster, cluster_data)) {
            free(cluster_data);
            return false;
        }

        FAT32_DirEntry *dir_entries = (FAT32_DirEntry*)cluster_data;

        if (!(dir_entries[entry_index].DIR_Attr & FAT32_ATTR_DIRECTORY)) {
            free(cluster_data);
            return false;
        }

        dir_cluster = ((uint32_t)dir_entries[entry_index].DIR_FstClusHI << 16)
                               | dir_entries[entry_index].DIR_FstClusLO;

        free(cluster_data);
    }

    fs->current_dir_cluster = dir_cluster;
    strcpy(fs->current_path, path);

    return true;
}

bool fat32_create_directory(FAT32_FileSystem *fs, const char *name) {
    if (!fs || !fs->is_formatted || !name || name[0] == '\0') {
        return false;
    }

    if (find_entry_by_name(fs, fs->current_dir_cluster, name) >= 0) {
        return false;
    }

    uint32_t new_dir_cluster = fat32_allocate_cluster(fs);
    if (new_dir_cluster == 0) {
        return false;
    }

    uint32_t free_entry_cluster;
    int free_entry_index = find_free_entry(fs, fs->current_dir_cluster, &free_entry_cluster);
    if (free_entry_index < 0) {
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    uint8_t *parent_cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
    if (!parent_cluster_data) {
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    if (!fat32_read_cluster(fs, free_entry_cluster, parent_cluster_data)) {
        free(parent_cluster_data);
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    FAT32_DirEntry *parent_entries = (FAT32_DirEntry*)parent_cluster_data;

    char short_name[11];
    convert_to_short_name(short_name, name);
    memcpy(parent_entries[free_entry_index].DIR_Name, short_name, 11);

    parent_entries[free_entry_index].DIR_Attr = FAT32_ATTR_DIRECTORY;
    parent_entries[free_entry_index].DIR_NTRes = 0;
    parent_entries[free_entry_index].DIR_CrtTimeTenth = 0;
    parent_entries[free_entry_index].DIR_CrtTime = get_fat_time();
    parent_entries[free_entry_index].DIR_CrtDate = get_fat_date();
    parent_entries[free_entry_index].DIR_LstAccDate = get_fat_date();
    parent_entries[free_entry_index].DIR_FstClusHI = (new_dir_cluster >> 16) & 0xFFFF;
    parent_entries[free_entry_index].DIR_FstClusLO = new_dir_cluster & 0xFFFF;
    parent_entries[free_entry_index].DIR_WrtTime = get_fat_time();
    parent_entries[free_entry_index].DIR_WrtDate = get_fat_date();
    parent_entries[free_entry_index].DIR_FileSize = 0;

    if (!fat32_write_cluster(fs, free_entry_cluster, parent_cluster_data)) {
        free(parent_cluster_data);
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    free(parent_cluster_data);

    uint8_t *new_dir_data = (uint8_t*)calloc(1, fs->bytes_per_cluster);
    if (!new_dir_data) {
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    FAT32_DirEntry *new_dir_entries = (FAT32_DirEntry*)new_dir_data;

    memset(new_dir_entries[0].DIR_Name, ' ', 11);
    new_dir_entries[0].DIR_Name[0] = '.';
    new_dir_entries[0].DIR_Attr = FAT32_ATTR_DIRECTORY;
    new_dir_entries[0].DIR_CrtTime = get_fat_time();
    new_dir_entries[0].DIR_CrtDate = get_fat_date();
    new_dir_entries[0].DIR_LstAccDate = get_fat_date();
    new_dir_entries[0].DIR_WrtTime = get_fat_time();
    new_dir_entries[0].DIR_WrtDate = get_fat_date();
    new_dir_entries[0].DIR_FstClusHI = (new_dir_cluster >> 16) & 0xFFFF;
    new_dir_entries[0].DIR_FstClusLO = new_dir_cluster & 0xFFFF;
    new_dir_entries[0].DIR_FileSize = 0;

    memset(new_dir_entries[1].DIR_Name, ' ', 11);
    new_dir_entries[1].DIR_Name[0] = '.';
    new_dir_entries[1].DIR_Name[1] = '.';
    new_dir_entries[1].DIR_Attr = FAT32_ATTR_DIRECTORY;
    new_dir_entries[1].DIR_CrtTime = get_fat_time();
    new_dir_entries[1].DIR_CrtDate = get_fat_date();
    new_dir_entries[1].DIR_LstAccDate = get_fat_date();
    new_dir_entries[1].DIR_WrtTime = get_fat_time();
    new_dir_entries[1].DIR_WrtDate = get_fat_date();
    new_dir_entries[1].DIR_FstClusHI = (fs->current_dir_cluster >> 16) & 0xFFFF;
    new_dir_entries[1].DIR_FstClusLO = fs->current_dir_cluster & 0xFFFF;
    new_dir_entries[1].DIR_FileSize = 0;

    if (!fat32_write_cluster(fs, new_dir_cluster, new_dir_data)) {
        free(new_dir_data);
        fat32_set_cluster_value(fs, new_dir_cluster, FAT32_CLUSTER_FREE);
        return false;
    }

    free(new_dir_data);

    return true;
}

bool fat32_create_file(FAT32_FileSystem *fs, const char *name) {
    if (!fs || !fs->is_formatted || !name || name[0] == '\0') {
        return false;
    }

    if (find_entry_by_name(fs, fs->current_dir_cluster, name) >= 0) {
        return false;
    }

    uint32_t free_entry_cluster;
    int free_entry_index = find_free_entry(fs, fs->current_dir_cluster, &free_entry_cluster);
    if (free_entry_index < 0) {
        return false;
    }

    uint8_t *parent_cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
    if (!parent_cluster_data) {
        return false;
    }

    if (!fat32_read_cluster(fs, free_entry_cluster, parent_cluster_data)) {
        free(parent_cluster_data);
        return false;
    }

    FAT32_DirEntry *parent_entries = (FAT32_DirEntry*)parent_cluster_data;

    char short_name[11];
    convert_to_short_name(short_name, name);
    memcpy(parent_entries[free_entry_index].DIR_Name, short_name, 11);

    parent_entries[free_entry_index].DIR_Attr = FAT32_ATTR_ARCHIVE;
    parent_entries[free_entry_index].DIR_NTRes = 0;
    parent_entries[free_entry_index].DIR_CrtTimeTenth = 0;
    parent_entries[free_entry_index].DIR_CrtTime = get_fat_time();
    parent_entries[free_entry_index].DIR_CrtDate = get_fat_date();
    parent_entries[free_entry_index].DIR_LstAccDate = get_fat_date();
    parent_entries[free_entry_index].DIR_FstClusHI = 0;
    parent_entries[free_entry_index].DIR_FstClusLO = 0;
    parent_entries[free_entry_index].DIR_WrtTime = get_fat_time();
    parent_entries[free_entry_index].DIR_WrtDate = get_fat_date();
    parent_entries[free_entry_index].DIR_FileSize = 0;

    if (!fat32_write_cluster(fs, free_entry_cluster, parent_cluster_data)) {
        free(parent_cluster_data);
        return false;
    }

    free(parent_cluster_data);

    return true;
}

void fat32_close(FAT32_FileSystem *fs) {
    if (!fs) {
        return;
    }

    if (fs->fat) {
        free(fs->fat);
        fs->fat = NULL;
    }

    disk_close(&fs->disk);

    fs->is_formatted = false;
}

bool fat32_list_directory(FAT32_FileSystem *fs, const char *path, FAT32_DirEntry *entries,
                          uint32_t max_entries, uint32_t *count) {
    if (!fs || !fs->is_formatted || !entries || !count) {
        return false;
    }

    *count = 0;

    uint32_t dir_cluster;

    if (path == NULL) {
        dir_cluster = fs->current_dir_cluster;
    } else {
        char path_components[256][13];
        int path_component_count = 0;

        if (!parse_path(path, path_components, &path_component_count)) {
            return false;
        }

        dir_cluster = fs->bootSector.BPB_RootClus;

        for (int i = 0; i < path_component_count; i++) {
            int entry_index = find_entry_by_name(fs, dir_cluster, path_components[i]);
            if (entry_index < 0) {
                return false;
            }

            uint8_t *cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
            if (!cluster_data) {
                return false;
            }

            uint32_t current_cluster = dir_cluster;
            while (entry_index >= fs->bytes_per_cluster / sizeof(FAT32_DirEntry)) {
                entry_index -= fs->bytes_per_cluster / sizeof(FAT32_DirEntry);
                current_cluster = fat32_get_next_cluster(fs, current_cluster);
            }

            if (!fat32_read_cluster(fs, current_cluster, cluster_data)) {
                free(cluster_data);
                return false;
            }

            FAT32_DirEntry *dir_entries = (FAT32_DirEntry*)cluster_data;

            if (!(dir_entries[entry_index].DIR_Attr & FAT32_ATTR_DIRECTORY)) {
                free(cluster_data);
                return false;
            }

            dir_cluster = ((uint32_t)dir_entries[entry_index].DIR_FstClusHI << 16) |
                dir_entries[entry_index].DIR_FstClusLO;

            free(cluster_data);
        }
    }

    uint8_t *cluster_data = (uint8_t*)malloc(fs->bytes_per_cluster);
    if (!cluster_data) {
        return false;
    }

    uint32_t current_cluster = dir_cluster;

    while (current_cluster >= 2 && current_cluster < FAT32_CLUSTER_END) {
        if (!fat32_read_cluster(fs, current_cluster, cluster_data)) {
            free(cluster_data);
            return false;
        }

        FAT32_DirEntry *dir_entries = (FAT32_DirEntry*)cluster_data;
        uint32_t entries_per_cluster = fs->bytes_per_cluster / sizeof(FAT32_DirEntry);

        for (uint32_t i = 0; i < entries_per_cluster && *count < max_entries; i++) {
            if (dir_entries[i].DIR_Name[0] == 0x00) {
                break;
            }

            if (dir_entries[i].DIR_Name[0] == 0xE5) {
                continue;
            }

            memcpy(&entries[*count], &dir_entries[i], sizeof(FAT32_DirEntry));
            (*count)++;
        }

        current_cluster = fat32_get_next_cluster(fs, current_cluster);
    }

    free(cluster_data);
    return true;
}