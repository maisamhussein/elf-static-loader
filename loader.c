#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <elf.h>

const char *phdr_type_to_string(int type)
{
    switch (type)
    {
    case PT_NULL:
        return "NULL";
    case PT_LOAD:
        return "LOAD";
    case PT_DYNAMIC:
        return "DYNAMIC";
    case PT_INTERP:
        return "INTERP";
    case PT_NOTE:
        return "NOTE";
    case PT_PHDR:
        return "PHDR";
    default:
        return "OTHER";
    }
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;

    Elf32_Phdr *phdr_table =
        (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);

    int i;
    for (i = 0; i < elf_header->e_phnum; i++)
    {
        func(&phdr_table[i], arg);
    }

    return 0;
}

void print_mmap_flags(Elf32_Phdr *phdr)
{
    if (phdr->p_type != PT_LOAD)
        return;

    printf("  mmap prot: ");

    if (phdr->p_flags & PF_R)
        printf("PROT_READ ");

    if (phdr->p_flags & PF_W)
        printf("PROT_WRITE ");

    if (phdr->p_flags & PF_X)
        printf("PROT_EXEC ");

    printf("| mmap flags: MAP_PRIVATE MAP_FIXED");
}

void print_phdr_info(Elf32_Phdr *phdr, int arg)
{
    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x ",
           phdr_type_to_string(phdr->p_type),
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz);

    printf("%c%c%c ",
           (phdr->p_flags & PF_R) ? 'R' : ' ',
           (phdr->p_flags & PF_W) ? 'W' : ' ',
           (phdr->p_flags & PF_X) ? 'E' : ' ');

    printf("0x%x", phdr->p_align);

    print_mmap_flags(phdr);

    printf("\n");
}

int get_prot_flags(Elf32_Phdr *phdr)
{
    int prot = 0;

    if (phdr->p_flags & PF_R)
        prot |= PROT_READ;

    if (phdr->p_flags & PF_W)
        prot |= PROT_WRITE;

    if (phdr->p_flags & PF_X)
        prot |= PROT_EXEC;

    return prot;
}

void load_phdr(Elf32_Phdr *phdr, int fd)
{
    if (phdr->p_type != PT_LOAD)
        return;

    unsigned int vaddr = phdr->p_vaddr & 0xfffff000;
    unsigned int offset = phdr->p_offset & 0xfffff000;
    unsigned int padding = phdr->p_vaddr & 0xfff;

    void *map = mmap((void *)vaddr,
                     phdr->p_memsz + padding,
                     get_prot_flags(phdr),
                     MAP_PRIVATE | MAP_FIXED,
                     fd,
                     offset);

    if (map == MAP_FAILED)
    {
        perror("mmap");
        _exit(1);
    }

    print_phdr_info(phdr, 0);
}
extern int startup(int argc,
                   char **argv,
                   void (*start)());

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <ELF file> [args...]\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        close(fd);
        return 1;
    }

    void *map_start = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;

    printf("Type     Offset   VirtAddr   PhysAddr   FileSiz MemSiz Flg Align\n");

    foreach_phdr(map_start, load_phdr, fd);
    fflush(stdout);

    startup(argc - 1,
            argv + 1,
            (void (*)())elf_header->e_entry);

    return 0;
}