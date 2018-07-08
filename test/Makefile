CONFFLAGS = -D_XOPEN_SOURCE=500
OPTFLAGS = -O3 -Ofast -msse3 -mfpmath=sse
CFLAGS = -ansi -pedantic -Wall -Wextra -I../main/include $(CONFFLAGS)\
		$(OPTFLAGS) -Wno-unused-function

.PHONY: all
all: a.out subpixel benchmark

.PHONY: clean
clean:
	$(RM) a.out subpixel benchmark *.o

a.out: test.o 3ds.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

subpixel: subpixel.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

benchmark: benchmark.o 3ds.o ../main/libraster.a
	$(CC) $^ -lX11 -lm -o $@

../main/libraster.a:
	$(MAKE) -C ../main

# test program source
test.o: test.c 3ds.h ../main/include/inputassembler.h \
		../main/include/framebuffer.h ../main/include/rasterizer.h \
		../main/include/texture.h ../main/include/context.h \
		../main/include/shader.h ../main/include/vector.h
benchmark.o: test.c 3ds.h ../main/include/inputassembler.h \
		../main/include/framebuffer.h ../main/include/rasterizer.h \
		../main/include/texture.h ../main/include/context.h \
		../main/include/shader.h ../main/include/vector.h
3ds.o: 3ds.c 3ds.h ../main/include/inputassembler.h ../main/include/context.h
subpixel.o: subpixel.c ../main/include/context.h \
		../main/include/framebuffer.h \
		../main/include/inputassembler.h ../main/include/vector.h
