# FAT32 Filesystem Emulator

A lightweight emulator for the FAT32 filesystem that allows you to create, navigate, and manipulate a virtual FAT32 disk image.

## Features

- **Virtual Disk Management**: Create and manage disk images that emulate physical storage devices
- **FAT32 Filesystem Operations**: Format disks with FAT32 filesystem
- **Directory Navigation**: Navigate through the directory structure with standard commands
- **File and Directory Manipulation**: Create files and directories within the filesystem
- **Command-Line Interface**: Simple and intuitive command-line interface for interacting with the filesystem

## Getting Started

### Prerequisites

- C compiler (GCC, Clang, etc.)
- CMake (version 3.30 or higher)
- Make

### Building from Source

1. Clone this repository:
   ```
   git clone https://github.com/yourusername/fat32emu.git
   cd fat32emu
   ```

2. Create a build directory and compile:
   ```
   mkdir build
   cd build
   cmake ..
   make
   ```

3. Install (optional):
   ```
   make install
   ```

### Running Tests

The project includes comprehensive tests for disk operations, FAT32 functionality, and utility functions. To run the tests:

```
cd build
cmake .. -DBUILD_TESTING=ON
make
ctest
```

## Usage

### Basic Command Syntax

```
f32disk <disk_file>
```

Where `<disk_file>` is the path to the disk image file. If the file doesn't exist, a new one will be created.

### Available Commands

Once the program is running, you can use the following commands:

- `format` - Create a new FAT32 filesystem on the disk
- `ls [path]` - List directory contents (current directory if no path is specified)
- `cd <path>` - Change current directory
- `mkdir <name>` - Create a new directory
- `touch <name>` - Create an empty file
- `exit` or `quit` - Exit the program
- `help` - Display available commands

### Example Session

```
$ ./f32disk mydisk.img
/>format
Ok
/>mkdir documents
Ok
/>mkdir pictures
Ok
/>ls
documents
pictures
/>cd documents
/documents>touch report.txt
Ok
/documents>ls
report.txt
/documents>cd ..
/>
```

## System Architecture

### Core Components

- **Disk Emulation Layer**: Handles low-level sector operations on the disk image file
- **FAT32 Filesystem**: Implements the FAT32 filesystem specification
- **Command Processor**: Parses and executes user commands
- **Utility Functions**: Provides path manipulation and other helper functions

### Filesystem Structure

The emulator implements a standard FAT32 filesystem with:

- Boot sector with BIOS Parameter Block (BPB)
- File Allocation Tables (FAT)
- Root directory cluster
- Data region

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- FAT32 File System Specification by Microsoft
- [FATFS by ChaN](http://elm-chan.org/fsw/ff/00index_e.html) for inspiration