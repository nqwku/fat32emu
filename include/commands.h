/**
 * @file commands.h
 * @brief Command processing functions for a FAT32 filesystem interface
 *
 * This header provides function declarations for command-line operations
 * that can be performed on a FAT32 filesystem, including formatting
 * directory navidation
 */


#ifndef COMMANDS_H
#define COMMANDS_H

#include "fat32.h"
/**
 * @brief Format the filesystem to FAT32
 *
 * Creates a new FAT32 filesystem on the device represented by the filesystem object.
 *
 * @param fs Pointer to the filesystem object
 * @return true if formatting was successful, false otherwise
 */
bool cmd_format(FAT32_FileSystem *fs);

/**
 * @brief List directory contents
 *
 * Displays all files and directories in the specified path. If no path is provided,
 * the current directory is listed
 *
 * @param fs Pointer to the filesystem object
 * @param path Path to the directory to list, or NULL for current directory
 * @return true if the directory listing was successful, false otherwise
 */
bool cmd_ls(FAT32_FileSystem *fs, const char *path);

/**
 * @brief Change current directory
 *
 * Navigates to the specified directory path
 *
 * @param fs Pointer to the filesystem object
 * @param path Path to the target directory (can be absolute or relative)
 * @return true if the directory change was successful, false otherwise
 */
bool cmd_cd(FAT32_FileSystem *fs, const char *path);

/**
 * @brief Create an empty file
 *
 * Creates a new empty file with the specified name in the current directory.
 *
 * @param fs Pointer to the filesystem object
 * @param name Name of the file to create
 * @return true if the file creation was successful, false otherwise
 */
bool cmd_mkdir(FAT32_FileSystem *fs, const char *name);

/**
 * @brief Create an empty file
 *
 * Creates a new empty file with the specified name in the current directory.
 *
 * @param fs Pointer to the filesystem object
 * @param name Name of the file to create
 * @return true if the file creation was successful, false otherwise
 */
bool cmd_touch(FAT32_FileSystem *fs, const char *name);

/**
 * @brief Display help information
 *
 * Prints a list of available commands and their descriptions to the console.
 */
void cmd_help();

/**
 * @brief Process a command string
 *
 * Parses an input command string and executes the corresponding function.
 * Supports commands like format, ls, cd, mkdir, touch, and help.
 *
 * @param fs Pointer to the filesystem object
 * @param input The command string to process
 */
void process_command(FAT32_FileSystem *fs, const char *input);
#endif //COMMANDS_H
