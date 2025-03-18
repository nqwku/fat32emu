/**
 * @file utils.h
 * @brief Utility functions for path manipulation and FAT32 name conversion
 *
 * This header provides various helper functions for path operations and
 * filename conversions between standard filenames and FAT32 8.3 format.
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Normalize a file path
 *
 * Normalizes a path by converting backslashes to forward slashes,
 * resolving "." and ".." components, and handling other path-related issues.
 *
 * @param path Path string to normalize (modified in-place)
 */
void path_normalize(char *path);

/**
 * @brief Combine base path with a relative path
 *
 * Creates a new path by combining a base path with a relative path.
 * If the relative path is absolute, it replaces the base path entirely.
 *
 * @param dest Buffer to store the resulting combined path
 * @param base Base path to start with
 * @param relative Relative path to append to the base
 */
void path_combine(char *dest, const char *base, const char *relative);

/**
 * @brief Check if a path is absolute
 *
 * Determines whether a path is absolute (starts with '/') or relative.
 *
 * @param path Path to check
 * @return true if the path is absolute, false otherwise
 */
bool path_is_absolute(const char *path);

/**
 * @brief Get the parent directory of a path
 *
 * Extracts the parent directory path from a given path.
 *
 * @param dest Buffer to store the parent path
 * @param path Original path to get the parent from
 */
void path_get_parent(char *dest, const char *path);

/**
 * @brief Get the filename component of a path
 *
 * Extracts the filename (without directory) from a path.
 *
 * @param dest Buffer to store the filename
 * @param path Original path to extract the filename from
 */
void path_get_filename(char *dest, const char *path);

/**
 * @brief Get the file extension from a path
 *
 * Extracts the file extension (including the dot) from a path.
 *
 * @param dest Buffer to store the extension
 * @param path Original path to extract the extension from
 */
void path_get_extension(char *dest, const char *path);

/**
 * @brief Check if a filename is valid
 *
 * Validates a filename according to common filesystem rules.
 * Checks for invalid characters and length constraints.
 *
 * @param name Filename to validate
 * @return true if the filename is valid, false otherwise
 */
bool is_valid_filename(const char *name);

/**
 * @brief Convert a normal filename to FAT32 8.3 short name format
 *
 * Transforms a standard filename into the 8.3 format used by FAT32.
 * The result is an 11-character string with 8 chars for name and 3 for extension.
 *
 * @param dest Buffer to store the 8.3 format name (must be at least 11 bytes)
 * @param src Original filename to convert
 */
void convert_to_short_name(char *dest, const char *src);

/**
 * @brief Convert a FAT32 8.3 short name to a normal filename
 *
 * Transforms an 8.3 format FAT32 filename back to a standard filename.
 *
 * @param dest Buffer to store the normal filename
 * @param src FAT32 8.3 format name (11 characters)
 */
void convert_from_short_name(char *dest, const char *src);

#endif /* UTILS_H */