#!gmake

include make/system.mk

SUBDIRS = \
	lib \
	examples \
	proto \
	server \
	tests

.PHONY: docs

all: precompile subdirs # docs

docs: net
	@$(DOXYGEN) Doxyfile

proto: lib
tests: lib
examples: lib
server: lib

include make/rules.mk
