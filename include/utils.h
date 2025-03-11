#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdint.h>

void path_normalize(char *path);
void path_combine(char *dest, const char *base, const char *relative);

bool path_is_absolute(const char *path);

void path_get_parent(char *dest, const char *path);
void path_get_filename(char *dest, const char *path);
void path_get_extension(char *dest, const char *path);

bool is_valid_filename(const char *name);
void convert_to_short_name(char *dest, const char *src);
void convert_from_short_name(char *dest, const char *src);

#endif //UTILS_H
