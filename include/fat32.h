#ifndef FAT32_H
#define FAT32_H

#include "disk.h"
#include <stdint.h>
#include <stdbool.h>

//FAT32 const
#define FAT32_SIGNATURE         0xAA55
#define FAT32_CLUSTER_FREE      0x00000000
#define FAT32_CLUSTER_RESERVED  0x00000001
#define FAT32_CLUSTER_BAD       0x0FFFFFF7
#define FAT32_CLUSTER_END       0x0FFFFFFF
#define FAT32_ROOTDIR_CLUSTER   2

// Attributes
#define FAT32_ATTR_READ_ONLY    0x01
#define FAT32_ATTR_HIDDEN       0x02
#define FAT32_ATTR_SYSTEM       0x04
#define FAT32_ATTR_VOLUME_ID    0x08
#define FAT32_ATTR_DIRECTORY    0x10
#define FAT32_ATTR_ARCHIVE      0x20
#define FAT32_ATTR_LFN          (FAT32_ATTR_READ_ONLY | FAT32_ATTR_HIDDEN | FAT32_ATTR_SYSTEM | FAT32_ATTR_VOLUME_ID)

typedef struct {
    uint8_t BS_jmpBoot[3];
    char BS_OEMName[8];
    uint16_t BPB_BytesPerSec;
    uint8_t BPB_SecPerClus;
    uint16_t BPB_RsvdSecCnt;
    uint8_t BPB_NumFATs;
    uint16_t BPB_RootEntCnt;
    uint16_t BPB_TotSec16;
    uint8_t BPB_Media;
    uint16_t BPB_FATSz16;
    uint16_t BPB_SecPerTrk;
    uint16_t BPB_NumHeads;
    uint32_t BPB_HiddSec;
    uint32_t BPB_TotSec32;

    uint32_t BPB_FATSz32;
    uint16_t BPB_ExtFlags;
    uint16_t BPB_FSVer;
    uint32_t BPB_RootClus;
    uint16_t BPB_FSInfo;
    uint16_t BPB_BkBootSec;
    uint8_t  BPB_Reserved[12];
    uint8_t  BS_DrvNum;
    uint8_t  BS_Reserved1;
    uint8_t  BS_BootSig;
    uint32_t BS_VolID;
    char BS_VolLab[11];
    char BS_FilSysType[8];
    uint8_t BootCode[420];
    uint16_t BootSignature;
} __attribute__((packed)) FAT32_BootSector;

typedef struct {
    char DIR_Name[11];
    uint8_t DIR_Attr;
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTenth;
    uint16_t DIR_CrtTime;
    uint16_t DIR_CrtDate;
    uint16_t DIR_LstAccDate;
    uint16_t DIR_FstClusHI;
    uint16_t DIR_WrtTime;
    uint16_t DIR_WrtDate;
    uint16_t DIR_FstClusLO;
    uint32_t DIR_FileSize;
} __attribute__((packed)) FAT32_DirEntry;

typedef struct {
    uint8_t LDIR_0rd;
    uint16_t LDIR_Name1[5];
    uint8_t LDIR_Attr;
    uint8_t LDIR_Type;
    uint8_t LDIR_Chksum;
    uint8_t LDIR_Name2[6];
    uint16_t LDIR_FstClusL0;
    uint16_t LDIR_Name3[2];
} __attribute__((packed)) FAT32_LFNEntry;

typedef struct {
    Disk disk;
    FAT32_BootSector bootSector;
    uint32_t *fat;
    uint32_t fat_size;
    uint32_t sectors_per_cluster;
    uint32_t first_data_sector;
    uint32_t data_cluster_count;
    uint32_t bytes_per_cluster;
    uint32_t current_dir_cluster;
    char current_path[256];
    bool is_formatted;
} FAT32_FileSystem;

bool fat32_init(FAT32_FileSystem *fs, const char *filename);
bool fat32_format(FAT32_FileSystem *fs);
bool fat32_check_fs(FAT32_FileSystem *fs);

bool fat32_list_directory(FAT32_FileSystem *fs, const char *path,
                          FAT32_DirEntry *entries,
                          uint32_t max_entries, uint32_t *count);

bool fat32_change_directory(FAT32_FileSystem *fs, const char *path);
bool fat32_create_directory(FAT32_FileSystem *fs, const char *name);

bool fat32_create_file(FAT32_FileSystem *fs, const char *name);

void fat32_close(FAT32_FileSystem *fs);

bool fat32_read_boot_sector(FAT32_FileSystem *fs);
bool fat32_write_boot_sector(FAT32_FileSystem *fs);
bool fat32_read_fat(FAT32_FileSystem *fs);
bool fat32_write_fat(FAT32_FileSystem *fs);
uint32_t fat32_get_next_cluster(FAT32_FileSystem *fs, uint32_t cluster);
uint32_t fat32_allocate_cluster(FAT32_FileSystem *fs);
bool fat32_set_cluster_value(FAT32_FileSystem *fs, uint32_t cluster, uint32_t value);
bool fat32_read_cluster(FAT32_FileSystem *fs, uint32_t cluster, void *buffer);
bool fat32_write_cluster(FAT32_FileSystem *fs, uint32_t cluster, const void *buffer);
uint32_t fat32_sector_for_cluster(FAT32_FileSystem *fs, uint32_t cluster);


#endif //FAT32_H
