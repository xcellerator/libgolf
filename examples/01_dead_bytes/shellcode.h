/*
 * mov al, 0x3c @ b0 3c
 * xor edi, edi @ 31 ff
 * syscall      @ 0f 05
 */

unsigned char buf[] = { 0xb0, 0x3c, 0x31, 0xff, 0x0f, 0x05 };
