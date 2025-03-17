#include "../include/commands.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_DIR_ENTRIES 1024

static void parse_input(const char *input, char *command, char *arg, size_t max_len) {
    char *space = strchr(input, ' ');

    if (space) {
        size_t cmd_len = space - input;
        if (cmd_len >= max_len) {
            cmd_len = max_len - 1;
        }

        strncpy(command, input, cmd_len);
        command[cmd_len] = '\0';

        while (*space && isspace(*space)) {
            space++;
        }

        strncpy(arg, space, max_len - 1);
        arg[max_len - 1] = '\0';
    } else {
        strncpy (command, input, max_len - 1);
        command[max_len - 1] = '\0';
        arg[0] = '\0';
    }
}

static void get_readable_name(char *dest, const char *src) {
    char name[9], ext[4];
    int i;

    for (i = 0; i < 8 && src[i] != ' '; i++) {
        name[i] = src[i];
    }
    name[i] = '\0';

    for (i = 0; i < 3 && src[i + 8] != ' '; i++) {
        ext[i] = src[i + 8];
    }
    ext[i] = '\0';

    if (ext[0]) {
        sprintf(dest, "%s.%s", name, ext);
    } else strcpy(dest, name);
}

bool cmd_format(FAT32_FileSystem *fs) {
    if (!fs) {
        return false;
    }

    if (!fat32_format(fs)) {
        printf("Error: Failed to format disk\n");
        return false;
    }

    printf("Ok\n");
    return true;
}

bool cmd_ls(FAT32_FileSystem *fs, const char *path) {
    if (!fs) {
        return false;
    }

    if (!fs->is_formatted) {
        printf("Unknown disk format\n");
        return false;
    }

    FAT32_DirEntry entries[MAX_DIR_ENTRIES];
    uint32_t count = 0;

    if (!fat32_list_directory(fs, path, entries, MAX_DIR_ENTRIES, &count)) {
        printf("Error: Failed to list directory\n");
        return false;
    }

    for (uint32_t i = 0; i < count; i++) {
        char readable_name[13];
        get_readable_name(readable_name, entries[i].DIR_Name);

        if (entries[i].DIR_Attr & FAT32_ATTR_DIRECTORY) {
            printf("%s\n", readable_name);
        } else printf("%s\n", readable_name);
    }

    return true;
}

bool cmd_cd(FAT32_FileSystem *fs, const char *path) {
    if (!fs || !path) {
        return false;
    }

    if (!fs->is_formatted) {
        printf("Unknown disk format\n");
        return false;
    }
    char absolute_path[256];
    if (path[0] == '/') {
        strncpy(absolute_path, path, sizeof(absolute_path) - 1);
        absolute_path[sizeof(absolute_path) - 1] = '\0';
    } else {
        if (strcmp(path, "..") == 0) {
            char parent_path[256];
            path_get_parent(parent_path, fs->current_path);
            strncpy(absolute_path, parent_path, sizeof(absolute_path) - 1);
            absolute_path[sizeof(absolute_path) - 1] = '\0';
        } else {
            if (strcmp(fs->current_path, "/") == 0) {
                snprintf(absolute_path, sizeof(absolute_path), "/%s", path);
            } else {
                snprintf(absolute_path, sizeof(absolute_path), "%s/%s", fs->current_path, path);
            }
        }
    }

    if (!fat32_change_directory(fs, path)) {
        printf("Error: Directory not found\n");
        return false;
    }

    return true;
}

bool cmd_mkdir(FAT32_FileSystem *fs, const char *name) {
    if (!fs || !name) {
        return false;
    }

    if (!fs->is_formatted) {
        printf("Unknown disk format\n");
        return false;
    }

    if (!fat32_create_directory(fs, name)) {
        printf("Error: Failed to create directory\n");
        return false;
    }

    printf("Ok\n");
    return true;
}

bool cmd_touch(FAT32_FileSystem *fs, const char *name) {
    if (!fs || !name) {
        return false;
    }

    if (!fs->is_formatted) {
        printf("Unknown disk format\n");
        return false;
    }

    if (!fat32_create_file(fs, name)) {
        printf("Error: Failed to create file\n");
        return false;
    }

    printf("Ok\n");
    return true;
}

void cmd_help() {
    printf("Available commands:\n");
    printf("  format         - Create new FAT32 filesystem\n");
    printf("  ls [path]      - List directory contents\n");
    printf("  cd <path>      - Change current directory (absolute path)\n");
    printf("  mkdir <name>   - Create new directory\n");
    printf("  touch <name>   - Create empty file\n");
    printf("  exit/quit      - Exit the program\n");
}

void process_command(FAT32_FileSystem *fs, const char *input) {
    if (!fs || !input) {
        return;
    }

    char command[32];
    char arg[256];

    parse_input(input, command, arg, sizeof(command));

    if (strcmp(command, "format") == 0) {
        cmd_format(fs);
    } else if (strcmp(command, "ls") == 0) {
        cmd_ls(fs, arg[0] ? arg : NULL);
    } else if (strcmp(command, "cd") == 0) {
        if (arg[0]) {
            cmd_cd(fs, arg);
        } else {
            printf("Error: Path expected\n");
        }
    } else if (strcmp(command, "mkdir") == 0) {
        if (arg[0]) {
            cmd_mkdir(fs, arg);
        } else {
            printf("Error: Name expected\n");
        }
    } else if (strcmp(command, "touch") == 0) {
        if (arg[0]) {
            cmd_touch(fs, arg);
        } else {
            printf("Error: Name expected\n");
        }
    } else if (strcmp(command, "help") == 0) {
        cmd_help();
    } else if (command[0]) {
        printf("Error: Unknown command '%s'\n", command);
        printf("Type 'help' for available commands\n");
    }
}