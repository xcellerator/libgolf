# LibGolf

## Example: AARCH64

This is an example for `aarch64` shellcode - tested on a PinePhone.

### Instructions

* Build the binary with `make`
* Run `aarch64.bin` on a 64-bit ARM platform (e.g. a PinePhone)
* Check the return value with `echo $?` - it will be `0`

### Details

In [`shellcode.h`](./shellcode.h) is ARM shellcode which calls `sys_exit` with return value `0`.

```nasm
mov x0, #0      @ Return value is 0
mov x8, #0x5d   @ 0x5d is sys_exit
svc #0          @ Interrupt to svc mode (syscall)
```

This gives the bytes `00 00 80 d2 a8 0b 80 d2 01 00 00 d4`, and forms the `.text` segment of the binary (see from offset `0x78` onwards below).

> Note that the bytes of each instruction are *reversed* for aarch64 platforms.

```xxd
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 b700 0100 0000 7800 4000 0000 0000  ........x.@.....
00000020: 4000 0000 0000 0000 0000 0000 0000 0000  @...............
00000030: 0000 0000 4000 3800 0100 0000 0000 0000  ....@.8.........
00000040: 0100 0000 0500 0000 0000 0000 0000 0000  ................
00000050: 0000 4000 0000 0000 0000 4000 0000 0000  ..@.......@.....
00000060: 0c00 0000 0000 0000 0c00 0000 0000 0000  ................
00000070: 0000 0100 0000 0000 0000 80d2 a80b 80d2  ................
00000080: 0100 00d4                                ....
```

### Customizations

None required.
