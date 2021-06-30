# You will need SDL2 (http://www.libsdl.org):
# MSYS2:
#   pacman -S mingw-w64-x86_64-SDL2

run: text-editor.exe
	./text-editor.exe

text-editor.exe: main.c
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

# pkg-config -h
# --cflags                          print required CFLAGS to stdout
# --libs                            print required linker flags to stdout
CFLAGS = `pkg-config --cflags sdl2`
LFLAGS = `pkg-config --libs sdl2`

.PHONY: tags
tags: main.c
	ctags --c-kinds=+l --exclude=Makefile -R .

.PHONY: lib-tags
lib-tags: main.c
	gcc $(CFLAGS) $< -M > headers-windows.txt
	python.exe parse-lib-tags.py
	rm -f headers-windows.txt
	ctags -f lib-tags --c-kinds=+p -L headers-posix.txt
	rm -f headers-posix.txt

.PHONY: clean
clean:
	rm -f text-editor.exe

what-CFLAGS:
	@echo $(CFLAGS)
what-LFLAGS:
	@echo $(LFLAGS)
