CONFFLAGS = -D_BSD_SOURCE
OPTFLAGS = -O3 -Ofast -msse2 -ffast-math
CFLAGS = -ansi -pedantic -Wall -Wextra $(CONFFLAGS) $(OPTFLAGS)

a.out: inputassembler.o framebuffer.o rasterizer.o texture.o window.o test.o
	$(CC) $^ -lX11 -lm -o $@

inputassembler.o: inputassembler.c inputassembler.h framebuffer.h rasterizer.h
framebuffer.o: framebuffer.c framebuffer.h
rasterizer.o: rasterizer.c rasterizer.h framebuffer.h texture.h
texture.o: texture.c texture.h
window.o: window.c window.h framebuffer.h
test.o: test.c window.h framebuffer.h rasterizer.h



.PHONY: clean
clean:
	$(RM) *.o a.out

