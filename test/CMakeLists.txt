enable_testing()

set(TEST_COMMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/disk.c
    ${CMAKE_SOURCE_DIR}/src/fat32.c
    ${CMAKE_SOURCE_DIR}/src/utils.c
)

add_executable(test_disk test_disk.c ${TEST_COMMON_SOURCES})
target_include_directories(test_disk PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME DiskTest COMMAND test_disk)

add_executable(test_fat32 test_fat32.c ${TEST_COMMON_SOURCES})
target_include_directories(test_fat32 PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME FAT32Test COMMAND test_fat32)

add_executable(test_utils test_utils.c ${TEST_COMMON_SOURCES})
target_include_directories(test_utils PRIVATE ${CMAKE_SOURCE_DIR}/include)
add_test(NAME UtilsTest COMMAND test_utils)