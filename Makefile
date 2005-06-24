#!gmake

include make/system.mk

SUBDIRS = base net server tests

.PHONY: docs

all: subdirs docs

docs: net
	@$(DOXYGEN) Doxyfile

net: base
tests: net
server: net

include make/rules.mk
