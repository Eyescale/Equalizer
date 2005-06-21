#!gmake

include make/system.mk

SUBDIRS = base net tests

.PHONY: docs

all: subdirs docs

docs: net
	@$(DOXYGEN) Doxyfile

net: base
tests: net

include make/rules.mk
