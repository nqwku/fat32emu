#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void test_path_normalize() {
    printf("Testing path normalization...\n");
    
    char path[256];
    
    strcpy(path, "//test//path//");
    path_normalize(path);
    assert(strcmp(path, "/test/path") == 0);
    
    strcpy(path, "/test/./path/../new");
    path_normalize(path);
    assert(strcmp(path, "/test/new") == 0);
    
    strcpy(path, "/");
    path_normalize(path);
    assert(strcmp(path, "/") == 0);
    
    printf("Path normalization test passed!\n");
}

void test_path_combine() {
    printf("Testing path combination...\n");
    
    char result[256];
    
    path_combine(result, "/test", "path");
    assert(strcmp(result, "/test/path") == 0);
    
    path_combine(result, "/test", "/absolute");
    assert(strcmp(result, "/absolute") == 0);
    
    path_combine(result, "/test", "");
    assert(strcmp(result, "/test") == 0);
    
    printf("Path combination test passed!\n");
}

void test_path_is_absolute() {
    printf("Testing absolute path detection...\n");
    
    assert(path_is_absolute("/test"));
    assert(!path_is_absolute("test"));
    assert(path_is_absolute("/"));
    
    printf("Absolute path detection test passed!\n");
}

void test_path_get_components() {
    printf("Testing path component extraction...\n");
    
    char result[256];
    
    path_get_parent(result, "/test/path");
    assert(strcmp(result, "/test") == 0);
    
    path_get_parent(result, "/");
    assert(strcmp(result, "/") == 0);
    
    path_get_filename(result, "/test/path/file.txt");
    assert(strcmp(result, "file.txt") == 0);
    
    path_get_extension(result, "/test/path/file.txt");
    assert(strcmp(result, ".txt") == 0);
    
    path_get_extension(result, "/test/path/file");
    assert(strcmp(result, "") == 0);
    
    printf("Path component extraction test passed!\n");
}

void test_filename_validation() {
    printf("Testing filename validation...\n");
    
    assert(is_valid_filename("test.txt"));
    assert(is_valid_filename("test"));
    assert(is_valid_filename("a"));
    
    assert(!is_valid_filename(""));
    assert(!is_valid_filename(NULL));
    assert(!is_valid_filename("test/path"));
    assert(!is_valid_filename("test:invalid"));
    assert(!is_valid_filename("test?invalid"));
    
    char long_name[300];
    memset(long_name, 'a', 256);
    long_name[256] = '\0';
    assert(!is_valid_filename(long_name));
    
    printf("Filename validation test passed!\n");
}

void test_name_conversion() {
    printf("Testing name conversion...\n");
    
    char short_name[11];
    char result[256];
    
    convert_to_short_name(short_name, "test.txt");
    assert(strncmp(short_name, "TEST    TXT", 11) == 0);
    
    convert_from_short_name(result, short_name);
    assert(strcmp(result, "TEST.TXT") == 0);
    
    convert_to_short_name(short_name, "testfile");
    assert(strncmp(short_name, "TESTFILE   ", 11) == 0);



    convert_to_short_name(short_name, "verylongfilename.extension");

    printf("Expecting: 'VERYLONGEXT', Received: '");
    for(int i = 0; i < 11; i++) {
        printf("%c", short_name[i]);
    }

    assert(strncmp(short_name, "VERYLONGEXT", 11) == 0);


    printf("'\n");
    printf("Name conversion test passed!\n");
}

int main(void) {
    test_path_normalize();
    test_path_combine();
    test_path_is_absolute();
    test_path_get_components();
    test_filename_validation();
    test_name_conversion();
    printf("All utility tests passed successfully!\n");
    return 0;
}