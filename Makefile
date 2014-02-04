CONFFLAGS = -D_BSD_SOURCE
OPTFLAGS = -O3 -Ofast -msse2 -ffast-math
CFLAGS = -ansi -pedantic -Wall -Wextra $(CONFFLAGS) $(OPTFLAGS)

a.out: framebuffer.o rasterizer.o transform.o texture.o window.o test.o
	$(CC) $^ -lX11 -lm -o $@

framebuffer.o: framebuffer.c framebuffer.h
rasterizer.o: rasterizer.c rasterizer.h framebuffer.h texture.h
transform.o: transform.c transform.h rasterizer.h
texture.o: texture.c texture.h
window.o: window.c window.h framebuffer.h
test.o: test.c window.h framebuffer.h rasterizer.h



.PHONY: clean
clean:
	$(RM) *.o a.out

