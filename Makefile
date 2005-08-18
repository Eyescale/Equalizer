#!gmake

include make/system.mk

SUBDIRS = \
	base \
	net \
	proto \
	tests

.PHONY: docs

all: subdirs # docs

docs: net
	@$(DOXYGEN) Doxyfile

net: base
proto: base
server: net
tests: net

include make/rules.mk
