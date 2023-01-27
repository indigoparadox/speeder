
# vim: ft=make noexpandtab

MDEMO_C_FILES := src/main.c

CC_GCC := gcc
CC_WATCOM := wcc386
LD_WATCOM := wcl386
MD := mkdir -p

GLOBAL_DEFINES :=

CFLAGS_GCC := $(GLOBAL_DEFINES) -Imaug/src
CFLAGS_WATCOM := $(GLOBAL_DEFINES) -imaug/src
LDFLAGS_GCC := -lm
LDFLAGS_WATCOM :=

# Optional builds.
ifneq ("$(RELEASE)","RELEASE")
	CFLAGS_WATCOM += -we -d3
	CFLAGS_GCC += -DDEBUG -Wall -g -fsanitize=address -fsanitize=leak -fsanitize=undefined -DDEBUG_THRESHOLD=1 -DDEBUG_LOG
	LDFLAGS_GCC += -g -fsanitize=address -fsanitize=leak -fsanitize=undefined
endif

ifeq ("$(SDL_VER)","1")
	CFLAGS_SDL_GCC := -DRETROFLAT_API_SDL1 $(shell pkg-config sdl --cflags)
	LDFLAGS_SDL_GCC := $(shell pkg-config sdl --libs) -lSDL_ttf
else
	CFLAGS_SDL_GCC := -DRETROFLAT_API_SDL2 $(shell pkg-config sdl2 --cflags)
	LDFLAGS_SDL_GCC := $(shell pkg-config sdl2 --libs) -lSDL_ttf
endif

# Target-specific options.
.PHONY: clean

all: speed.ale speed.sdl spedd.exe spedw.exe spednt.exe speed.html

# Unix (Allegro)

speed.ale: $(addprefix obj/$(shell uname -s)-allegro/,$(subst .c,.o,$(MDEMO_C_FILES)))
	$(CC_GCC) -o $@ $^ $(LDFLAGS_GCC) $(shell pkg-config allegro --libs)

obj/$(shell uname -s)-allegro/%.o: %.c
	$(MD) $(dir $@)
	$(CC_GCC) -c -o $@ $< -DRETROFLAT_OS_UNIX $(CFLAGS_GCC) -DRETROFLAT_API_ALLEGRO $(shell pkg-config allegro --cflags)

# Unix (SDL)

speed.sdl: $(addprefix obj/$(shell uname -s)-sdl/,$(subst .c,.o,$(MDEMO_C_FILES)))
	$(CC_GCC) -o $@ $^ $(LDFLAGS_GCC) $(LDFLAGS_SDL_GCC)

obj/$(shell uname -s)-sdl/%.o: %.c
	$(MD) $(dir $@)
	$(CC_GCC) -c -o $@ $< -DRETROFLAT_OS_UNIX $(CFLAGS_GCC) $(CFLAGS_SDL_GCC)

# WASM

speed.html: $(addprefix obj/wasm/,$(subst .c,.o,$(MDEMO_C_FILES)))
	emcc -o $@ $^ -s USE_SDL=2 -s USE_SDL_TTF=2

obj/wasm/%.o: %.c
	$(MD) $(dir $@)
	emcc -c -o $@ $< -DRETROFLAT_OS_WASM -DRETROFLAT_API_SDL2 -s USE_SDL=2 -Imaug/src -s USE_SDL_TTF=2

# DOS

spedd.exe: $(addprefix obj/dos/,$(subst .c,.o,$(MDEMO_C_FILES)))
	#i586-pc-msdosdjgpp-gcc -o $@ $^ -L$(ALLEGRO_DJGPP_ROOT)/lib -lalleg
	wcl386 -l=dos32a -s -3s -k128k dos/clib3s.lib alleg.lib $(LDFLAGS_WATCOM) -fe=$@ $^

obj/dos/%.o: %.c
	$(MD) $(dir $@)
	#i586-pc-msdosdjgpp-gcc -fgnu89-inline -I$(ALLEGRO_DJGPP_ROOT)/include -DRETROFLAT_OS_DOS -DRETROFLAT_API_ALLEGRO -c -o $@ $<
	wcc386 -bt=dos32a -s -3s -DRETROFLAT_OS_DOS -DRETROFLAT_API_ALLEGRO $(CFLAGS_WATCOM) -fo=$@ $(<:%.c=%)

# WinNT

spednt.exe: $(addprefix obj/nt/,$(subst .c,.o,$(MDEMO_C_FILES)))
	wlink name $@ system nt_win fil {$^}

obj/nt/%.o: %.c
	$(MD) $(dir $@)
	wcc386 -bt=nt -i$(WATCOM)/h/nt -DRETROFLAT_API_WIN32 -DRETROFLAT_OS_WIN $(CFLAGS_WATCOM) -fo=$@ $(<:%.c=%) -DRETROFLAT_SCREENSAVER

# Win386

spedw.rex: $(addprefix obj/win16/,$(subst .c,.o,$(MDEMO_C_FILES)))
	wcl386 -l=win386 $(LDFLAGS_WATCOM) -fe=$@ $^

spedw.exe: spedw.rex
	wbind $< -s $(WATCOM)/binw/win386.ext -R $@

obj/win16/%.o: %.c
	$(MD) $(dir $@)
	wcc386 -bt=windows -i$(WATCOM)/h/win -DRETROFLAT_API_WIN16 -DRETROFLAT_OS_WIN $(CFLAGS_WATCOM) -fo=$@ $(<:%.c=%) -DRETROFLAT_SCREENSAVER

# Clean

clean:
	rm -rf obj speed.ale speed.sdl spedw32.exe *.err spedd.exe spedw.exe *.rex spednt.exe

