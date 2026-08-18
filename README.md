# ELF Static Loader

A custom **32-bit ELF executable loader** developed in C as an academic project for an Extended Systems Programming Laboratory.

The loader reads an ELF executable, examines its program headers, maps loadable segments into memory with the appropriate permissions, and transfers execution to the program's entry point.

## Technologies

* C
* x86 Assembly
* Linux
* ELF32
* `mmap`
* Linux system interfaces
* GCC
* NASM
* Make

## Key Features

* Parses 32-bit ELF headers
* Iterates over ELF program headers
* Identifies loadable `PT_LOAD` segments
* Maps executable segments into virtual memory
* Converts ELF segment permissions to Linux memory protection flags
* Uses `MAP_PRIVATE` and `MAP_FIXED`
* Handles read, write, and execute permissions
* Transfers execution to the ELF entry point
* Uses custom assembly startup code

## How It Works

The loader first opens the target ELF executable and maps the file into memory.

It then reads the ELF header and locates the program header table.

For every program header, the loader checks whether the segment is loadable. Loadable segments are mapped to their required virtual addresses using `mmap`.

ELF permissions are translated into the corresponding Linux memory protection flags:

* `PF_R` → `PROT_READ`
* `PF_W` → `PROT_WRITE`
* `PF_X` → `PROT_EXEC`

After the required segments have been loaded, execution is transferred to the ELF executable's entry point.

## Project Structure

```text
elf-static-loader/
├── loader.c
├── start.s
├── startup.s
├── linking_script
├── Makefile
└── .gitignore
```

## Build

The project targets **32-bit Linux** and requires GCC with 32-bit development support and NASM.

Build the loader:

```bash
make
```

This creates the `loader` executable.

## Run

Run the loader with a 32-bit ELF executable:

```bash
./loader <ELF-file> [arguments...]
```

For example:

```bash
./loader program
```

The loader prints information about the ELF program headers and the memory mapping permissions before transferring execution to the loaded program.

## Clean

Remove generated object files and the loader executable:

```bash
make clean
```

## Academic Project

Developed as an academic project as part of an Extended Systems Programming Laboratory at Ben-Gurion University.
