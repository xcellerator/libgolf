/*
 * arm32.c
 * License: MIT
 * Author: Harvey Phillips
 * GitHub: github.com/xcellerator/libgolf
 */

#include "../../libgolf.h"
#include "shellcode.h"

int main(int argc, char **argv)
{
    /*
     * Specify architecture - makes 'elf', 'ehdr' and 'phdr'  available
     * Format: INIT_ELF(ISA, ARCH)
     * Supported:
     * - ISA: X86_64, ARM32, AARCH64
     * - ARCH: 32, 64
     */
    INIT_ELF(ARM32,32);

    /* Copy the bytes from buf[] (defined in shellcode.h) */
    copy_text_segment(elf, buf, sizeof(buf));

    /* Populate the binary with sane values */
    ehdr = populate_ehdr(elf);
    phdr = populate_phdr(elf);
    set_entry_point(elf);

    /* Start of Customizations */

    // ARM32
    ehdr->e_flags = 0x5000200;  // Version5 EABI, soft-float ABI

    /* End of Customizations */

    /* Write the ELF struct to a file */
    print_info(elf);
    generate_elf(elf);

    /* Clean up and finish */
    cleanup(elf);
    return 0;
}
