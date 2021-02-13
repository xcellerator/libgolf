.PHONY: golf clean

CC=gcc
CFLAGS=-I.
PROG=aarch64

golf:
	@$(CC) -w -o aarch64 aarch64.c
	@./$(PROG)
	@chmod +x $(PROG).bin
	@if [ -f /usr/bin/hexyl ]; then hexyl $(PROG).bin; else xxd $(PROG).bin; fi

clean:
	@rm $(PROG) $(PROG).bin
