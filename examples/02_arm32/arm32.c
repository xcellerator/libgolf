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
     * Specify architecture - populates 'ehdr' and 'phdr'
     * Format: INIT_ELF(ISA, ARCH)
     * Supported:
     * - ISA: X86_64, ARM32, AARCH64
     * - ARCH: 32, 64
     */
    INIT_ELF(ARM32,32);

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
