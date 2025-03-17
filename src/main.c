#include "../include/fat32.h">"
#include "../include/commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 512

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FAT32_FileSystem fs;
    if (!fat32_init(&fs, argv[1])) {
        fprintf(stderr, "Failed to initialize disk: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    char command[MAX_COMMAND_LENGTH];
    while (1) {
        printf("%s>", fs.current_path);

        if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
            break;
        }

        size_t len = strlen(command);
        if (len > 0 && command[len - 1] == '\n') {
            command[len - 1] = '\0';
        }

        if (strcmp(command, "exit") == 0 || strcmp(command, "quit") == 0) {
            break;
        }

        process_command(&fs, command);
    }

    fat32_close(&fs);

    return EXIT_SUCCESS;

}
