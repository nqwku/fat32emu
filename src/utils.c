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

    char *src = path;
    char *dst = path;

    bool prev_slash = false;

    while (*src) {
        if (*src == '/') {
            if (!prev_slash) {
                *dst++ = *src;
                prev_slash = true;
            }
        } else {
            *dst++ = *src;
            prev_slash = false;
        }
        src++;
    }
    *dst = '\0';

    size_t len = strlen(path);
    if (len > 1 && path[len - 1] == '/') {
        path[len - 1] = '\0';
    }
}

void path_combine(char *dest, const char *base, const char *relative) {
    if (!dest || !base) {
        return;
    }

    if (!relative || *relative == '\0') {
        strcpy(dest, base);
        return;
    }

    if (path_is_absolute(relative)) {
        strcpy(dest, relative);
        return;
    }

    strcpy(dest, base);
    size_t len = strlen(dest);

    if (len > 0 && dest[len - 1] == '/') {
        dest[len] = '/';
        dest[len + 1] = '\0';
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

void convert_to_short_name(char *dest, const char *src) {
    if (!dest || !src) {
        return;
    }

    memset(dest, ' ', 1);
     int i = 0, j = 0;

    while (src[i] && src[i] != '.' && j < 8) {
        char c = src[i++];
        dest[j++] = c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
    }

    while (src[i] && src[i] != '.') {
        i++;
    }

    if (src[i] == '.') {
        i++;
        j = 0;

        while (src[i] && j < 11) {
            char c = src[i++];
            dest[j++] = c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
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