# DSi Studio / BlocksDS. Derived from the NTM CC0 build files.
BLOCKSDS ?= /opt/blocksds/core
WONDERFUL_TOOLCHAIN ?= /opt/wonderful
.PHONY: all arm9 arm7
all: dist/DSiStudio.nds
arm9:
	$(MAKE) -f Makefile.arm9
arm7:
	$(MAKE) -f Makefile.arm7
dist/DSiStudio.nds: arm9 arm7
	mkdir -p dist
	$(BLOCKSDS)/tools/ndstool/ndstool -c $@ -7 build/arm7.elf -9 build/arm9.elf -g "####" 00 "DSISTUDIO" -b assets/icon.bmp "DSi Studio;Your DSiWare collection;DSi Studio contributors"
	cp $@ dist/DSiStudio.dsi
