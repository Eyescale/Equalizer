#!gmake

include make/system.mk

SUBDIRS = \
	base \
	client \
	examples \
	net \
	proto \
	server \
	tests

.PHONY: docs

all: subdirs # docs

docs: net
	@$(DOXYGEN) Doxyfile

net: base
proto: base
server: net
tests: net
client: net
examples: client

include make/rules.mk
