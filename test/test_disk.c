#include "../include/disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

const char* get_temp_filename() {
    static char filename[64];
    sprintf(filename, "test_disk_%d.bin", rand());
    return filename;
}

void test_disk_init() {
    printf("Testing DISK initialization...\n");
    
    const char *test_filename = get_temp_filename();
    Disk disk;
    
    assert(disk_init(&disk, test_filename));
    assert(disk.file != NULL);
    assert(disk.total_sectors == DISK_DEFAULT_SIZE / DISK_SECTOR_SIZE);
    
    disk_close(&disk);
    assert(disk_init(&disk, test_filename));
    
    disk_close(&disk);
    remove(test_filename);
    
    printf("DISK initialization test passed!\n");
}

void test_disk_sector_operations() {
    printf("Testing disk sector operations...\n");
    
    const char *test_filename = get_temp_filename();
    Disk disk;
    assert(disk_init(&disk, test_filename));
    
    uint8_t write_buffer[DISK_SECTOR_SIZE];
    uint8_t read_buffer[DISK_SECTOR_SIZE];
    
    for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
        write_buffer[i] = (i % 256);
    }
    
    assert(disk_write_sector(&disk, 0, write_buffer));
    
    assert(disk_read_sector(&disk, 0, read_buffer));
    
    assert(memcmp(write_buffer, read_buffer, DISK_SECTOR_SIZE) == 0);
    
    disk_close(&disk);
    remove(test_filename);
    
    printf("Disk sector operations test passed!\n");
}

int main() {
    srand(time(NULL));
    
    test_disk_init();
    test_disk_sector_operations();
    
    printf("All disk tests passed successfully!\n");
    return 0;
}