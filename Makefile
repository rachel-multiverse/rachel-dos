# Rachel DOS Edition Makefile
# For cross-compilation from modern systems

# DOS cross-compiler (install djgpp-cross-gcc)
DOSCC = i586-pc-msdosdjgpp-gcc
DOSFLAGS = -O2 -Wall

# Native compiler for testing
CC = gcc
CFLAGS = -O2 -Wall -DDOS_EMULATION

# Targets
all: rachel.exe

# DOS executable
rachel.exe: rachel.c rules.c
	@echo "Building DOS executable..."
	$(DOSCC) $(DOSFLAGS) rachel.c rules.c -o rachel.exe
	@echo "rachel.exe created - run in DOSBox or real DOS"

# Native test build (for development)
test: rachel.c rules.c
	@echo "Building native test version..."
	$(CC) $(CFLAGS) rachel.c rules.c -o rachel_test
	@echo "Test build created as rachel_test"

# Run in DOSBox
dosbox: rachel.exe
	@echo "Launching in DOSBox..."
	dosbox -c "mount c ." -c "c:" -c "rachel.exe"

# Clean
clean:
	rm -f rachel.exe rachel_test *.o

# Create disk image for real hardware
disk: rachel.exe
	@echo "Creating bootable DOS disk image..."
	# Would need DOS system files for this
	@echo "Copy rachel.exe to a DOS boot disk"

.PHONY: all test dosbox clean disk