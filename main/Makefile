CONFFLAGS = -D_XOPEN_SOURCE=500
OPTFLAGS = -O3 -msse2
CFLAGS = -ansi -pedantic -Wall -Wextra -Iinclude $(CONFFLAGS) $(OPTFLAGS)

.PHONY: all
all: libraster.a

.PHONY: clean
clean:
	$(RM) libraster.a -r obj

libraster.a: obj/inputassembler.o obj/framebuffer.o obj/rasterizer.o \
		obj/texture.o obj/tl.o obj/context.o
	$(AR) rcs $@ $^
	ranlib $@

obj/%.o: src/%.c include/%.h
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/context.o: src/context.c include/context.h include/predef.h\
				include/config.h
obj/inputassembler.o: src/inputassembler.c include/inputassembler.h\
					include/rasterizer.h include/tl.h include/predef.h\
					include/context.h include/config.h
obj/framebuffer.o: src/framebuffer.c include/framebuffer.h include/predef.h\
					include/config.h
obj/rasterizer.o: src/rasterizer.c include/rasterizer.h include/framebuffer.h\
				include/texture.h include/predef.h include/context.h\
				include/config.h
obj/texture.o: src/texture.c include/texture.h include/predef.h\
				include/config.h
obj/tl.o: src/tl.c include/tl.h include/rasterizer.h include/predef.h\
				include/context.h include/config.h

