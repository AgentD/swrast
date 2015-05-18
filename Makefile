CONFFLAGS = -D_XOPEN_SOURCE=500
OPTFLAGS = -O3 -msse2
CFLAGS = -ansi -pedantic -Wall -Wextra -Iinclude $(CONFFLAGS) $(OPTFLAGS)

a.out: obj/inputassembler.o obj/framebuffer.o obj/rasterizer.o obj/texture.o\
		obj/tl.o obj/window.o obj/test.o obj/3ds.o obj/context.o
	$(CC) $^ -lX11 -lm -o $@

obj/%.o: src/%.c include/%.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

# rasterizer
obj/context.o: src/context.c include/context.h include/predef.h
obj/inputassembler.o: src/inputassembler.c include/inputassembler.h\
					include/framebuffer.h include/rasterizer.h include/tl.h\
					include/predef.h include/context.h
obj/framebuffer.o: src/framebuffer.c include/framebuffer.h include/predef.h
obj/rasterizer.o: src/rasterizer.c include/rasterizer.h include/framebuffer.h\
				include/texture.h include/predef.h include/context.h
obj/texture.o: src/texture.c include/texture.h include/framebuffer.h\
				include/predef.h
obj/tl.o: src/tl.c include/tl.h include/rasterizer.h include/predef.h\
				include/context.h

# text program source
obj/window.o: test/window.c test/window.h include/framebuffer.h\
				include/predef.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/test.o: test/test.c test/window.h include/framebuffer.h\
			include/rasterizer.h include/predef.h include/context.h
	$(CC) $(CFLAGS) -c $< -o $@

obj/3ds.o: test/3ds.c test/3ds.h include/inputassembler.h
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	$(RM) a.out -r obj

