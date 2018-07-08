CONFFLAGS = -D_XOPEN_SOURCE=500
OPTFLAGS = -O3 -Ofast -msse3 -mfpmath=sse
CFLAGS = -ansi -pedantic -Wall -Wextra -Iinclude $(CONFFLAGS) $(OPTFLAGS)\
		-Wno-unused-function

.PHONY: all
all: libraster.a

.PHONY: clean
clean:
	$(RM) libraster.a -r obj

libraster.a: obj/inputassembler.o obj/framebuffer.o obj/rspixel.o \
		obj/texture.o obj/shader.o obj/context.o obj/rsflat.o \
		obj/rasterizer.o obj/rsvertex.o obj/window.o
	$(AR) rcs $@ $^
	ranlib $@

obj/%.o: src/%.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/rsvertex.o: src/rsvertex.c include/shader.h include/rasterizer.h\
				include/predef.h include/context.h\
				include/config.h include/vector.h\
				include/color.h
obj/rasterizer.o: src/rasterizer.c include/shader.h include/rasterizer.h\
				include/predef.h include/context.h\
				include/config.h include/vector.h\
				include/color.h
obj/context.o: src/context.c include/context.h include/predef.h\
				include/config.h include/framebuffer.h\
				include/rasterizer.h include/vector.h\
				include/shader.h
obj/inputassembler.o: src/inputassembler.c include/inputassembler.h\
			include/rasterizer.h include/shader.h include/predef.h\
			include/context.h include/config.h include/vector.h
obj/framebuffer.o: src/framebuffer.c include/framebuffer.h include/predef.h\
			include/config.h include/color.h include/vector.h
obj/rspixel.o: src/rspixel.c include/rasterizer.h include/framebuffer.h\
			include/shader.h include/predef.h include/context.h\
			include/config.h include/vector.h include/color.h
obj/texture.o: src/texture.c include/texture.h include/predef.h\
			include/config.h include/vector.h include/color.h
obj/shader.o: src/shader.c include/shader.h include/rasterizer.h\
			include/predef.h include/context.h include/config.h\
			include/vector.h include/color.h
obj/rsflat.o: src/rsflat.c include/rasterizer.h include/framebuffer.h\
			include/shader.h include/predef.h include/context.h\
			include/config.h include/vector.h include/color.h

obj/window.o: src/window.c include/window.h include/framebuffer.h
