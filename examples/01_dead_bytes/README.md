# LibGolf

## Example: Dead Bytes

This example illustrates how many fields in the ELF format the Linux loader will ignore. The ELF file produced is *very* broken -- try to run it in `gdb` or `readelf` and they'll just give up, but it runs just fine.

### Instructions

* Run `make`
* Run `./dead_bytes.bin` (it will exit immediately)
* Check the return value with `echo $?` (it exited cleanly with `0`)

### Details

In [`shellcode.h`](./shellcode.h), you'll find the raw assembly bytes for:

```nasm
mov al, 0x3c    @ SYS_EXIT
xor rdi, rdi    @ Return value '0'
syscall
```

This gives the bytes `b0 3c 48 31 ff 0f 05`, and forms the `.text` segment of the binary (see from offset `0x78` onwards below).

```xxd
00000000: 7f45 4c46 5858 5858 5858 5858 5858 5858  .ELFXXXXXXXXXXXX
00000010: 0200 3e00 5858 5858 7800 4000 0000 0000  ..>.XXXXx.@.....
00000020: 4000 0000 0000 0000 5858 5858 5858 5858  @.......XXXXXXXX
00000030: 5858 5858 4000 3800 0100 5858 5858 5858  XXXX@.8...XXXXXX
00000040: 0100 0000 0500 0000 0000 0000 0000 0000  ................
00000050: 0000 4000 0000 0000 5858 5858 5858 5858  ..@.....XXXXXXXX
00000060: 0700 0000 0000 0000 0700 0000 0000 0000  ................
00000070: 5858 5858 5858 5858 b03c 4831 ff0f 05    XXXXXXXX.<H1...
```

### Customizations

|Field|Value|Reason|
|-|-|-|
|`e_ident[EI_CLASS]`|`X`|Ignored by ELF Loader|
|`e_ident[EI_DATA]`|`X`|Ignored by ELF Loader|
|`e_ident[EI_VERSION]`|`X`|Ignored by ELF Loader|
|`e_ident[EI_OSABI]`|`X`|Ignored by ELF Loader|
|`e_ident[0x8] - e_ident[0xf]`|`X`|Ignored by Elf Loader|
|`e_version`|`XXXX`|Ignored by ELF Loader|
|`e_shoff`|`XXXXXXXX`|Ignored by ELF Loader|
|`e_flags`|`XXXX`|Ignored by ELF Loader|
|`e_shentsize`|`XX`|Ignored by ELF Loader|
|`e_shnum`|`XX`|Ignored by ELF Loader|
|`e_shstrndx`|`XX`|Ignored by ELF Loader|
|`p_paddr`|`XXXXXXXX`|Ignored by ELF Loader|
|`p_align`|`XXXXXXXX`|Ignored by ELF Loader|
