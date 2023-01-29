
# vim: ft=make noexpandtab

C_FILES := src/main.c

include maug/Makefile.inc

# Target-specific options.
.PHONY: clean

all: speed.ale speed.sdl speedd.exe speedw.exe speednt.exe speed.html

# Unix (Allegro)

$(eval $(call TGTUNIXALE,speed))

# Unix (SDL)

$(eval $(call TGTUNIXSDL,speed))

# WASM

$(eval $(call TGTWASMSDL,speed))

# DOS

$(eval $(call TGTDOSALE,speed))

# WinNT

$(eval $(call TGTWINNT,speed))

# Win386

$(eval $(call TGTWIN386,speed))

# Clean

clean:
	rm -rf $(CLEAN_TARGETS)

