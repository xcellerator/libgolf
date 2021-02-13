/*
 * dead_bytes.c
 * License: MIT
 * Author: Harvey Phillips
 * GitHub: github.com/xcellerator/libgolf
 */

#include "../../libgolf.h"
#include "shellcode.h"

int main(int argc, char **argv)
{
    /*
     * Specify architecture - makes 'elf', 'ehdr' and 'phdr' available
     * Format: INIT_ELF(ISA, ARCH)
     * Supported:
     * - ISA: X86_64, ARM32, AARCH64 
     * - ARCH: 32, 64
     */
    INIT_ELF(X86_64,64);

    /* Copy the bytes from buf[] (defined in shellcode.h) */
    copy_text_segment(elf, buf, sizeof(buf));

    /* Populate the binary with sane values */
    ehdr = populate_ehdr(elf);
    phdr = populate_phdr(elf);
    set_entry_point(elf);

    /* Start of Customizations */

    (ehdr->e_ident)[EI_CLASS] = 0x58;
    (ehdr->e_ident)[EI_DATA] = 0x58;
    (ehdr->e_ident)[EI_VERSION] = 0x58;
    (ehdr->e_ident)[EI_OSABI] = 0x58;

    int i;
    for ( i = 0x8 ; i < 0x10 ; i++)
    {
        (ehdr->e_ident)[i] = 0x58;
    }

    ehdr->e_version = 0x58585858;
    ehdr->e_shoff = 0x5858585858585858;
    ehdr->e_flags = 0x58585858;
    ehdr->e_shentsize = 0x5858;
    ehdr->e_shnum = 0x5858;
    ehdr->e_shstrndx = 0x5858;

    phdr->p_paddr = 0x5858585858585858;
    phdr->p_align = 0x5858585858585858;
    /* End of Customizations */

    /* Write the ELF struct to a file */
    print_info(elf);
    generate_elf(elf);

    /* Clean up and finish */
    cleanup(elf);
    return 0;
}
