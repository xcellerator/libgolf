.PHONY: golf clean

CC=gcc
CFLAGS=-I.
PROG=arm32

golf:
	@$(CC) -w -o $(PROG) $(PROG).c
	@./$(PROG)
	@chmod +x $(PROG).bin
	@if [ -f /usr/bin/hexyl ]; then hexyl $(PROG).bin; else xxd $(PROG).bin; fi

clean:
	@rm $(PROG) $(PROG).bin
