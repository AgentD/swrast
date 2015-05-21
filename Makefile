.PHONY: all
all: main test

.PHONY: main
main:
	$(MAKE) -C main

.PHONY: test
test: main
	$(MAKE) -C test

.PHONY: clean
clean:
	$(MAKE) -C main clean
	$(MAKE) -C test clean

