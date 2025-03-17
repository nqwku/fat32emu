#ifndef COMMANDS_H
#define COMMANDS_H

#include "fat32.h"

bool cmd_format(FAT32_FileSystem *fs);
bool cmd_ls(FAT32_FileSystem *fs, const char *path);
bool cmd_cd(FAT32_FileSystem *fs, const char *path);
bool cmd_mkdir(FAT32_FileSystem *fs, const char *name);
bool cmd_touch(FAT32_FileSystem *fs, const char *name);

void cmd_help();

void process_command(FAT32_FileSystem *fs, const char *input);
#endif //COMMANDS_H
