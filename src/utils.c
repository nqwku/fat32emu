#include "../include/utils.h"
#include <string.h>
#include <ctype.h>

void path_normalize(char *path){
    if (!path || *path == '\0') {
        return;
    }

    for (char *p = path; *p; p++) {
        if (*p == '\\') {
            *p = '/';
        }
    }

    char result[256];
    char *result_ptr = result;
    char *token;
    char *saveptr;
    char path_copy[256];

    strcpy(path_copy, path);

    if (path[0] == '/') {
        *result_ptr++ = '/';
    }

    token = strtok_r(path_copy, "/", &saveptr);
    while (token) {
        if (strcmp(token, ".") == 0) {
        } else if (strcmp(token, "..") == 0) {
            if (result_ptr > result + 1) {
                result_ptr--;
                while (result_ptr > result && *result_ptr != '/') {
                    result_ptr--;
                }
                if (*result_ptr == '/' && result_ptr != result) {
                    result_ptr++;
                }
            }
        } else {
            if (*(result_ptr-1) != '/') {
                *result_ptr++ = '/';
            }
            strcpy(result_ptr, token);
            result_ptr += strlen(token);
        }

        token = strtok_r(NULL, "/", &saveptr);
    }

    if (result_ptr > result + 1 && *(result_ptr-1) == '/') {
        result_ptr--;
    }

    *result_ptr = '\0';
    strcpy(path, result);
}

void path_combine(char *dest, const char *base, const char *relative) {
    if (!dest || !base) {
        return;
    }

    if (!relative || *relative == '\0') {
        strcpy(dest, base);
        return;
    }

    if (relative[0] == '/') {
        strcpy(dest, relative);
        return;
    }

    strcpy(dest, base);
    size_t base_len = strlen(dest);

    if (base_len > 0 && dest[base_len - 1] != '/') {
        dest[base_len] = '/';
        dest[base_len + 1] = '\0';
        base_len++;
    }

    strcat(dest, relative);

    path_normalize(dest);
}



bool path_is_absolute(const char *path) {
    return path && path[0] == '/';
}

void path_get_parent(char *dest, const char *path) {
    if (!dest || !path) {
        return;
    }

    strcpy(dest, path);
    path_normalize(dest);

    char *last_slash = strrchr(dest, '/');

    if (!last_slash) {
        strcpy(dest, ".");
        return;
    }

    if (last_slash == dest) {
        dest[1] = '\0';
    } else {
        *last_slash = '\0';
    }
}

void path_get_filename(char *dest, const char *path) {
    if (!dest || !path) {
        return;
    }

    const char *last_slash = strrchr(path, '/');

    if (!last_slash) {
        strcpy(dest, path);
    } else {
        strcpy(dest, last_slash + 1);
    }
}

void path_get_extension(char *dest, const char *path) {
    if (!dest || !path) {
        return;
    }

    char filename[256];
    path_get_filename(filename, path);

    char *last_dot = strrchr(filename, '.');

    if (!last_dot || last_dot == filename) {
        dest[0] = '\0';
    } else {
        strcpy(dest, last_dot);
    }
}

bool is_valid_filename(const char *name) {
    if (!name || *name == '\0') {
        return false;
    }

    if (strlen(name) > 255) {
        return false;
    }

    const char *invalid_chars = "\"\\/:*?<>|";

    for (const char *p = name; *p; p++) {
        if (strchr(invalid_chars, *p) || *p < 32) {
            return false;
        }
    }

    return true;
}


void convert_to_short_name(char *short_name, const char *name) {
    for (int i = 0; i < 11; i++) {
        short_name[i] = ' ';
    }

    int i = 0;
    while (i < 8 && name[i] && name[i] != '.') {
        char c = name[i];
        if (c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        short_name[i] = c;
        i++;
    }

    const char *ext = NULL;
    const char *p = name;
    while (*p) {
        if (*p == '.') {
            ext = p + 1;
        }
        p++;
    }

    if (ext) {
        for (i = 0; i < 3 && ext[i]; i++) {
            char c = ext[i];
            if (c >= 'a' && c <= 'z') {
                c = c - 'a' + 'A';
            }
            short_name[8 + i] = c;
        }
    }
}

void convert_from_short_name(char *dest, const char *src) {
    if (!dest || !src) {
        return;
    }

    int j = 0;

    for (int i = 0; i < 8 && src[i] != ' '; i++) {
        dest[j++] = src[i];
    }

    if (src[8] != ' ') {
        dest[j++] = '.';

        for (int i = 8; i < 11 && src[i] != ' '; i++) {
            dest[j++] = src[i];
        }
    }

    dest[j] = '\0';
}