CONFFLAGS = -D_BSD_SOURCE
OPTFLAGS = -O3 -msse2
CFLAGS = -ansi -pedantic -Wall -Wextra $(CONFFLAGS) $(OPTFLAGS)

a.out: inputassembler.o framebuffer.o rasterizer.o texture.o window.o test.o\
		3ds.o tl.o compare.o pixel.o
	$(CC) $^ -lX11 -lm -o $@

inputassembler.o: inputassembler.c inputassembler.h framebuffer.h\
					rasterizer.h tl.h
framebuffer.o: framebuffer.c framebuffer.h
rasterizer.o: rasterizer.c rasterizer.h framebuffer.h texture.h compare.h\
				pixel.h
texture.o: texture.c texture.h framebuffer.h
compare.o: compare.c compare.h
window.o: window.c window.h framebuffer.h
pixel.o: pixel.c pixel.h rasterizer.h
test.o: test.c window.h framebuffer.h rasterizer.h compare.h pixel.h
3ds.o: 3ds.c 3ds.h inputassembler.h
tl.o: tl.c tl.h rasterizer.h



.PHONY: clean
clean:
	$(RM) *.o a.out

