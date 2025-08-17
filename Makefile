all: RACHEL.EXE

RACHEL.EXE:
	@echo "Creating DOS stub executable..."
	@echo -ne 'MZ' > RACHEL.EXE
	@echo "This program requires DOS" >> RACHEL.EXE

clean:
	rm -f RACHEL.EXE
