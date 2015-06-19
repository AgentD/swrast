CONFFLAGS = -D_XOPEN_SOURCE=500
OPTFLAGS = -O3 -Ofast -msse3 -mfpmath=sse
CFLAGS = -ansi -pedantic -Wall -Wextra -I../main/include $(CONFFLAGS)\
		$(OPTFLAGS) -Wno-unused-function

.PHONY: all
all: a.out subpixel benchmark

.PHONY: clean
clean:
	$(RM) a.out subpixel benchmark *.o

a.out: window.o test.o 3ds.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

subpixel: subpixel.o window.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

benchmark: benchmark.o window.o 3ds.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

../main/libraster.a:
	$(MAKE) -C ../main

# test program source
window.o: window.c window.h ../main/include/framebuffer.h
test.o: test.c window.h window.h 3ds.h ../main/include/inputassembler.h \
		../main/include/framebuffer.h ../main/include/rasterizer.h \
		../main/include/texture.h ../main/include/context.h \
		../main/include/shader.h ../main/include/vector.h
benchmark.o: test.c window.h window.h 3ds.h ../main/include/inputassembler.h \
		../main/include/framebuffer.h ../main/include/rasterizer.h \
		../main/include/texture.h ../main/include/context.h \
		../main/include/shader.h ../main/include/vector.h
3ds.o: 3ds.c 3ds.h ../main/include/inputassembler.h ../main/include/context.h
subpixel.o: subpixel.c window.h ../main/include/context.h \
		../main/include/framebuffer.h \
		../main/include/inputassembler.h ../main/include/vector.h
