/*
 * libgolf.h
 * License: MIT
 * Author: Harvey Phillips
 * GitHub: github.com/xcellerator/libgolf
 */

#include <stdio.h>
#include <stdlib.h>
#include <linux/elf.h>
#include <limits.h>
#include <stdlib.h>

#define ANSI_COLOUR_GREEN "\x1b[32m"
#define ANSI_COLOUR_RED   "\x1b[31m"
#define ANSI_COLOUR_RESET "\x1b[0m"

#define PRINT_GOOD(s_, ...) printf(ANSI_COLOUR_GREEN "[+] " ANSI_COLOUR_RESET s_, ##__VA_ARGS__)
#define PRINT_BAD(s_, ...) printf(ANSI_COLOUR_RED "[!] " ANSI_COLOUR_RESET s_, ##__VA_ARGS__)

#define SUPPORTED "Invalid ISA. Supported: X86_64, ARM32, AARCH64.\n"

/* EI_ABIVERSION isn't used anymore and elf.h defines EI_PAD to be 0x09 */
#define EI_ABIVERSION   0x08
#define EI_PAD          0x09

/* Define the Architecture and ISA constants to match those in <linux/elf.h> */
#define X86_64  EM_X86_64
#define ARM32   EM_ARM
#define AARCH64 EM_AARCH64

/*
 * The ELF and Program headers are different sizes depending on 32- and 64-bit
 * architectures
 */
#define EHDR_T(x) Elf##x##_Ehdr
#define PHDR_T(x) Elf##x##_Phdr
#define EHDR(x) ehdr##x
#define PHDR(x) phdr##x
#define GET_EHDR(x) (&(elf_ptr->EHDR(x)));
#define GET_PHDR(x) (&(elf_ptr->PHDR(x)));
#define REF_EHDR(b,x) ((Elf##b##_Ehdr *) ehdr)->x
#define REF_PHDR(b,x) ((Elf##b##_Phdr *) phdr)->x
int ehdr_size;
int phdr_size;

/* Create the pointer to an ELF struct, and set ISA */
#define INIT_ELF(x,b) \
        RawBinary elf_obj; \
        RawBinary *elf = &elf_obj; \
        elf->isa = x; \
        EHDR_T(b) *ehdr; \
        PHDR_T(b) *phdr; \
        format_filename(elf, argv[0]);

/*
 * This struct holds the bytes that will be executed, and the size.
 */
typedef struct text_segment {
    size_t text_size;
    unsigned char *text_segment;
} TextSegment;

/*
 * This is the raw ELF file
 * - EHDR(xx) is the ELF header
 * - PHDR(xx) is the program header
 * - text is the text segment
 * - filename is the name of the golf'd binary
 * - isa is the target architecture (X86_64, ARM32, AARCH64)
 */
typedef struct rawbinary_t {
    EHDR_T(32) EHDR(32);
    PHDR_T(32) PHDR(32);
    EHDR_T(64) EHDR(64);
    PHDR_T(64) PHDR(64);
    TextSegment text;
    char *filename;
    int isa;
} RawBinary;


/*
 * Copy an E_IDENT array into the corresponding fields in the ELF header
 * Called by populate_ehdr()
 */
int populate_e_ident(RawBinary *elf_ptr, unsigned char e_ident[])
{
    int i;
    /* Depending on whether the target ISA is 32- or 64-bit, set e_ident */
    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            for ( i = 0 ; i < EI_NIDENT ; i++ )
                elf_ptr->EHDR(64).e_ident[i] = e_ident[i];
            break;
        case ARM32:
            for ( i = 0 ; i < EI_NIDENT ; i++ )
                elf_ptr->EHDR(32).e_ident[i] = e_ident[i];
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }
    return 0;
}

/*
 * Copy bytes from buf[] array into text_segment in ELF struct
 */
int copy_text_segment(RawBinary *elf_ptr, unsigned char buf[], int text_size)
{
    int i;

    /* Set size of text segment and allocate the buffer */
    elf_ptr->text.text_size = text_size;
    elf_ptr->text.text_segment = malloc(elf_ptr->text.text_size * sizeof(unsigned char));

    /* Copy the bytes into the text segment buffer */
    for ( i = 0 ; i < elf_ptr->text.text_size ; i++ )
    {
        elf_ptr->text.text_segment[i] = buf[i];
    }
}

/*
 * Populate the ELF Header with sane values
 * Returns a pointer to an EHDR struct
 */
void * populate_ehdr(RawBinary *elf_ptr)
{
    /*
     * Set ehdr_size and phdr_size. Determined by whether target ISA is 32- or
     * 64-bit.
     */
    switch(elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            ehdr_size = sizeof(EHDR_T(64));
            phdr_size = sizeof(PHDR_T(64));
            break;
        case ARM32:
            ehdr_size = sizeof(EHDR_T(32));
            phdr_size = sizeof(PHDR_T(32));
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    };

    /* Start with the E_IDENT area at the top of the file */
    unsigned char e_ident[EI_NIDENT] = {0};

    /* Magic Bytes */
    e_ident[EI_MAG0] = 0x7F;
    e_ident[EI_MAG1] = 0x45; // E
    e_ident[EI_MAG2] = 0x4C; // L
    e_ident[EI_MAG3] = 0x46; // F

    /*
     * EI_CLASS denotes the architecture:
     * ELFCLASS32: 0x01
     * ELFCLASS64: 0x02
     */
    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            e_ident[EI_CLASS] = ELFCLASS64;
            break;
        case ARM32:
            e_ident[EI_CLASS] = ELFCLASS32;
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }

    /*
     * EI_DATA denotes the endianness:
     * ELFDATA2LSB:   0x01
     * ELFDATA2MSB:   0x02
     */
    e_ident[EI_DATA] = ELFDATA2LSB;

    /* EI_VERSION is always 0x01 */
    e_ident[EI_VERSION] = EV_CURRENT;

    /*
     * EI_OSABI defines the target OS. Ignored by most modern ELF parsers.
     */
    e_ident[EI_OSABI] = ELFOSABI_NONE;

    /* EI_ABIVERSION was for sub-classification. Un-defined since Linux 2.6 */
    e_ident[EI_ABIVERSION] = 0x00;

    /* EI_PAD is currently unused */
    e_ident[EI_PAD] = 0x00;

    /* Copy the E_IDENT section to the ELF struct */
    populate_e_ident(elf_ptr, e_ident);

    /*
     * The remainder of the ELF header following E_IDENT follows.
     *
     * ehdr is a pointer to either an Elf32_Edhr, or Elf64_Ehdr struct.
     */
    void *ehdr = NULL;
    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            ehdr = (&(elf_ptr->EHDR(64)));
            break;
        case ARM32:
            ehdr = (&(elf_ptr->EHDR(32)));
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }

    /*
     * Depending on whether the ISA is 32- or 64-bit determines the size of 
     * many of the fields in the ELF Header. This switch case deals with it.
     */
    switch (elf_ptr->isa)
    {
        // 64-Bit ISAs
        case X86_64:
        case AARCH64:
            /*
             * e_type specifies what kind of ELF file this is:
             * ET_NONE:         0x00    // Unknown Type
             * ET_REL:          0x01    // Relocatable
             * ET_EXEC:         0x02    // Executable File
             * ET_DYN:          0x03    // Shared Object
             * ET_CORE:         0x04    // Core Dump
             */
            REF_EHDR(64,e_type) = ET_EXEC;        // 0x0002

            /* e_machine specifies the target ISA */
            REF_EHDR(64,e_machine) = elf_ptr->isa;

            /* e_version is always set of 0x01 for the original ELF spec */
            REF_EHDR(64,e_version) = EV_CURRENT;  // 0x00000001

            /*
             * e_entry is the memory address of the entry point
             * Set by set_entry_point() after p_vaddr is set in the phdr
             */
            REF_EHDR(64,e_entry) = 0x0;

            /*
             * e_phoff points to the start of the program header, which
             * immediately follows the ELF header
             */
            REF_EHDR(64,e_phoff) = ehdr_size;

            /* e_shoff points to the start of the section header table */
            REF_EHDR(64,e_shoff) = 0x00;

            /* e_flags is architecture dependent */
            REF_EHDR(64,e_flags) = 0x0;

            /* e_ehsize contains the size of the ELF header */
            REF_EHDR(64,e_ehsize) = ehdr_size;

            /* e_phentsize is the size of the program header */
            REF_EHDR(64,e_phentsize) = phdr_size;

            /*
             * e_phnum contains the number of entries in the program header
             * e_phnum * e_phentsize = size of program header table
             */
            REF_EHDR(64,e_phnum) = 0x1;

            /* e_shentsize contains the size of a section header entry */
            REF_EHDR(64,e_shentsize) = 0x0;

            /*
             * e_shnum contains the number of entries in the section header
             * e_shnum * e_shentsize = size of section header table
             */
            REF_EHDR(64,e_shnum) = 0x0;

            /*
             * e_shstrndx contains the index of the section header table that
             * contains the section names
             */
            REF_EHDR(64,e_shstrndx) = 0x0;

            break;
        // 32-Bit ISAs
        case ARM32:
            /*
             * e_type specifies what kind of ELF file this is:
             * ET_NONE:         0x00    // Unknown Type
             * ET_REL:          0x01    // Relocatable
             * ET_EXEC:         0x02    // Executable File
             * ET_DYN:          0x03    // Shared Object
             * ET_CORE:         0x04    // Core Dump
             */
            REF_EHDR(32,e_type) = ET_EXEC;        // 0x0002

            /* e_machine specifies the target ISA */
            REF_EHDR(32,e_machine) = elf_ptr->isa;

            /* e_version is always set of 0x01 for the original ELF spec */
            REF_EHDR(32,e_version) = EV_CURRENT;  // 0x00000001

            /*
             * e_entry is the memory address of the entry point
             * Set by set_entry_point() after p_vaddr is set in the phdr
             */
            REF_EHDR(32,e_entry) = 0x0;

            /*
             * e_phoff points to the start of the program header, which
             * immediately follows the ELF header
             */
            REF_EHDR(32,e_phoff) = ehdr_size;

            /* e_shoff points to the start of the section header table */
            REF_EHDR(32,e_shoff) = 0x0i;

            /* e_flags is architecture dependent */
            REF_EHDR(32,e_flags) = 0x0;

            /* e_ehsize contains the size of the ELF header */
            REF_EHDR(32,e_ehsize) = ehdr_size;

            /* e_phentsize is the size of the program header */
            REF_EHDR(32,e_phentsize) = phdr_size;

            /*
             * e_phnum contains the number of entries in the program header
             * e_phnum * e_phentsize = size of program header table
             */
            REF_EHDR(32,e_phnum) = 0x1;

            /* e_shentsize contains the size of a section header entry */
            REF_EHDR(32,e_shentsize) = 0x0;

            /*
             * e_shnum contains the number of entries in the section header
             * e_shnum * e_shentsize = size of section header table
             */
            REF_EHDR(32,e_shnum) = 0x0;

            /*
             * e_shstrndx contains the index of the section header table that
             * contains the section names
             */
            REF_EHDR(32,e_shnum) = 0x0;

            break;
    }
    return ehdr;
}

/*
 * Populate the program headers with sane values
 * Returns a pointer to a PHDR struct
 */
void * populate_phdr(RawBinary *elf_ptr)
{
    /*
     * All offsets are relative to the start of the program header (0x40)
     *
     * phdr is a pointer to either an Elf32_Phdr, or Elf64_Phdr struct.
     */
    void *phdr = NULL;
    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            phdr = (&(elf_ptr->PHDR(64)));
            break;
        case ARM32:
            phdr = (&(elf_ptr->PHDR(32)));
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }

    /*
     * Depending on whether the ISA is 32- or 64-bit determines the size of
     * many of the fields in the Progra Header. This switch case deals with it.
     */
    switch (elf_ptr->isa)
    {
        // 64-Bit ISAs
        case X86_64:
        case AARCH64:
            /*
             * p_type identifies what type of segment this is
             * PT_NULL:         0x0     // Unused
             * PT_LOAD:         0x1     // Loadable Segment
             * PT_DYNAMIC:      0x2     // Dynamic Linker Information
             * PT_INTERP:       0x3     // Interpreter Information
             * PT_NOTE:         0x4     // Auxiliary Information
             * PT_SHLIB:        0x5     // Reserved
             * PT_PHDR:         0x6     // Segment with Program Header
             * PT_TLS:          0x7     // Thread Local Storage
             */
            REF_PHDR(64,p_type) = PT_LOAD;      // 0x1

            /*
             * p_flags defines permissions for this section
             * PF_R:    0x4     // Read
             * PF_W:    0x2     // Write
             * PF_X:    0x1     // Execute
             */
            REF_PHDR(64,p_flags) = PF_R | PF_X; // 0x5

            /*
             * p_offset is the offset in the file image (relative to the start
             * of the program header) for this segment.
             */
            REF_PHDR(64,p_offset) = 0x0;

            /*
             * p_vaddr is the virtual address where this segment should be loaded
             * p_paddr is for the physical address (unused by System V)
             */
            REF_PHDR(64,p_vaddr) = 0x400000;
            REF_PHDR(64,p_paddr) = 0x400000;

            /*
             * p_filesz is the size of the segment in the file image
             * p_memsz is the size of the segment in memory
             *
             * Note: p_filesz doesn't have to equal p_memsz
             */
            REF_PHDR(64,p_filesz) = elf_ptr->text.text_size;
            REF_PHDR(64,p_memsz) = elf_ptr->text.text_size;

            break;
        // 32-Bit ISAs
        case ARM32:
             /*
             * p_type identifies what type of segment this is
             * PT_NULL:         0x0     // Unused
             * PT_LOAD:         0x1     // Loadable Segment
             * PT_DYNAMIC:      0x2     // Dynamic Linker Information
             * PT_INTERP:       0x3     // Interpreter Information
             * PT_NOTE:         0x4     // Auxiliary Information
             * PT_SHLIB:        0x5     // Reserved
             * PT_PHDR:         0x6     // Segment with Program Header
             * PT_TLS:          0x7     // Thread Local Storage
             */
            REF_PHDR(32,p_type) = PT_LOAD;      // 0x1

            /*
             * p_flags defines permissions for this section
             * PF_R:    0x4     // Read
             * PF_W:    0x2     // Write
             * PF_X:    0x1     // Execute
             */
            REF_PHDR(32,p_flags) = PF_R | PF_X; // 0x5

            /*
             * p_offset is the offset in the file image (relative to the start
             * of the program header) for this segment.
             */
            REF_PHDR(32,p_offset) = 0x0;

            /*
             * p_vaddr is the virtual address where this segment should be loaded
             * p_paddr is for the physical address (unused by System V)
             */
            REF_PHDR(32,p_vaddr) = 0x10000;
            REF_PHDR(32,p_paddr) = 0x10000;

            /*
             * p_filesz is the size of the segment in the file image
             * p_memsz is the size of the segment in memory
             *
             * Note: p_filesz doesn't have to equal p_memsz
             */
            REF_PHDR(32,p_filesz) = elf_ptr->text.text_size;
            REF_PHDR(32,p_memsz) = elf_ptr->text.text_size;

            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }

    /*
     * p_align is the memory alignment
     *
     * Note: p_vaddr = p_offset % p_align
     */
    switch (elf_ptr->isa)
    {
        case X86_64:
            REF_PHDR(64,p_align) = 0x400000;
            break;
        case ARM32:
            REF_PHDR(32,p_align) = 0x10000;
            break;
        case AARCH64:
            REF_PHDR(64,p_align) = 0x400000;
            break;
    }
    return phdr;
}

/*
 * e_entry depends on p_vaddr, so has to be set after populate_ehdr()
 * and populate_phdr() have been called.
 */
int set_entry_point(RawBinary *elf_ptr)
{
    /*
     * Once the whole ELF file is copied into memory, control is handed to e_entry.
     * Relative to the process's virtual memory address, the .text segment will
     * be located immediately after the ELF and program header.
     *
     * ehdr and phdr are pointers to the ELF and Program headers respectively.
     * The switch case casts and assigns them to the correct fields of the ELF
     * struct, then sets ehdr->e_entry.
     */
    void *ehdr, *phdr;

    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            ehdr = GET_EHDR(64);
            phdr = GET_PHDR(64);
            REF_EHDR(64,e_entry) = REF_PHDR(64,p_vaddr) + ehdr_size + phdr_size;
            break;
        case ARM32:
            ehdr = GET_EHDR(32);
            phdr = GET_PHDR(32);
            REF_EHDR(32,e_entry) = REF_PHDR(32,p_vaddr) + ehdr_size + phdr_size;
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }
    return 0;
}

/*
 * Print some basic info about certain fields
 * (useful for debugging)
 */
int print_info(RawBinary *elf_ptr)
{
    int filesize;
    void *ehdr, *phdr;

    /* Calculate the final file size of the binary */
    filesize = ehdr_size + phdr_size + elf_ptr->text.text_size;

    PRINT_GOOD("Filename\t: \"%s\"\n", elf_ptr->filename);

    /* Grab pointers to the ELF and Program Headers and print the ISA */
    PRINT_GOOD("ISA\t\t: ");
    switch(elf_ptr->isa)
    {
        case X86_64:
            ehdr = GET_EHDR(64);
            phdr = GET_PHDR(64);
            printf("X86_64 (0x%lx)\n", X86_64);
            break;
        case ARM32:
            ehdr = GET_EHDR(32);
            phdr = GET_PHDR(32);
            printf("ARM32 (0x%lx)\n", ARM32);
            break;
        case AARCH64:
            ehdr = GET_EHDR(64);
            phdr = GET_PHDR(64);
            printf("AARCH64 (0x%lx)\n", AARCH64);
            break;
    }

    PRINT_GOOD("Code Size\t: %d\n", (int) elf_ptr->text.text_size);
    PRINT_GOOD("Load Addr\t: 0x%x\n", (unsigned int) REF_PHDR(64,p_vaddr));
    PRINT_GOOD("Entry Point\t: 0x%x\n", (unsigned int) REF_EHDR(64,e_entry));

    return 0;
}

/*
 * Format the filename
 *
 * Note: elf_ptr->filename is free'd by cleanup().
 */
int format_filename(RawBinary *elf_ptr, char *original)
{
    /*
     * Strip the leading './', and append '.bin'.
     */
    elf_ptr->filename = malloc(NAME_MAX);
    memcpy(elf_ptr->filename, original+2, NAME_MAX);
    strncat(elf_ptr->filename, ".bin", 4);
    return 0;
}

/*
 * Free the malloc'd buffers used in building the ELF
 */
int cleanup(RawBinary *elf_ptr)
{
    free(elf_ptr->text.text_segment);
    free(elf_ptr->filename);
    return 0;
}

/*
 * Write the ELF struct to a file
 */
int generate_elf(RawBinary *elf_ptr)
{
    FILE *output;
    void *ehdr, *phdr;

    /* Open the target file */
    output = fopen(elf_ptr->filename, "wb");
    if (!output)
    {
        PRINT_BAD("Unable to write to \"%s\"\n", elf_ptr->filename);
        return 1;
    }

    /* Grab pointers to ELF and Program Headers */
    switch (elf_ptr->isa)
    {
        case X86_64:
        case AARCH64:
            ehdr = GET_EHDR(64);
            phdr = GET_PHDR(64);
            break;
        case ARM32:
            ehdr = GET_EHDR(32);
            phdr = GET_PHDR(32);
            break;
        default:
            PRINT_BAD(SUPPORTED);
            exit(1);
    }

    /*
     * Write:
     * - ELF Header
     * - Program Header
     * - Text Segment
     */
    fwrite(ehdr, ehdr_size, 1, output);
    fwrite(phdr, phdr_size, 1, output);
    fwrite(elf_ptr->text.text_segment, elf_ptr->text.text_size, 1, output);

    fclose(output);

    return 0;
}
