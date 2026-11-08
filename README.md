# Mini File System — C++ (UFS Simulation)

A command-line file system simulator developed in C++11 as part of a collaborative Computer Science project at Université Laval.

The application simulates basic file system operations using a virtual disk, i-nodes, data blocks, and directory structures.

## Features

- **Virtual File System:** Simulates file and directory management using a virtual disk.
- **File Creation:** Create files within the simulated file system.
- **Directory Management:** Create and navigate directory structures.
- **File and Directory Removal:** Remove entries and release allocated resources.
- **Directory Listing:** Display files, directories, sizes, i-node numbers, and link counts.
- **Disk Formatting:** Initialize the virtual file system.
- **Command-Line Interface:** Interactive menu supporting individual commands and batch execution from text files.

## Technologies

- **Language:** C++11
- **Compiler:** GCC / G++
- **Build System:** Makefile
- **Concepts:** Object-Oriented Programming, Data Structures, File Systems, I-nodes, Virtual Disks, Memory and Resource Management

## Project Structure

```text
Mini-File-System-Portfolio/
├── block.cpp
├── block.h
├── disqueVirtuel.cpp
├── disqueVirtuel.h
├── main.cpp
├── Makefile
└── README.md
```

## Getting Started

### Requirements

- A C++11-compatible compiler (G++ recommended)
- A terminal or command prompt
- Make (optional)

### Compile

Using G++:

```bash
g++ -std=c++11 -Wall -Wextra main.cpp disqueVirtuel.cpp block.cpp -o ufs.exe
```

Alternatively, on a Unix-like environment with Make installed:

```bash
make
```

### Run

On Windows:

```cmd
ufs.exe
```

On Linux or macOS, after compiling to an executable named `ufs`:

```bash
./ufs
```

## Available Commands

| Command | Description |
|---|---|
| `format` | Initialize the virtual file system |
| `mkdir /documents` | Create a directory |
| `create /documents/test.txt` | Create a file |
| `ls /documents` | List directory contents |
| `rm /documents/test.txt` | Remove a file |

## Example Usage

The following commands were tested successfully:

```text
format
mkdir /documents
create /documents/test.txt
ls /documents
```

Example output:

```text
/documents
d    .           Size: 84   inode: 1   nlink: 3
d    ..          Size: 84   inode: 2   nlink: 2
-    test.txt    Size: 0    inode: 7   nlink: 1
```

The application displays information about file types, sizes, i-nodes, and link counts.

## Academic Context

This project was independently designed and implemented as part of the GLO-2001 course at Université Laval.

It demonstrates practical experience with:

- C++ programming and object-oriented design
- File system fundamentals and virtual disk simulation
- Data structures and resource allocation
- I-node and block management
- Command parsing and interactive terminal applications
- Debugging, compilation, and testing with G++

## Author

**Alou Coulibaly** — Computer Science Graduate, Université Laval

[GitHub Profile](https://github.com/AlouCoul)

*This project was developed individually as part of an academic assignment.*