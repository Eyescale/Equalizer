#!gmake

include make/system.mk

SUBDIRS = \
	lib \
	examples \
	proto \
	server \
	tests

.PHONY: docs

TARGETS = precompile subdirs # docs

include make/rules.mk

docs: lib
	@$(DOXYGEN) Doxyfile

lib: precompile
proto: lib
tests: lib
examples: lib
server: lib
