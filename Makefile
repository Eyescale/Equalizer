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
tests: net
client: net
server: client
examples: client

include make/rules.mk
