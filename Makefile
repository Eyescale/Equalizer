
TOP = .
include $(TOP)/make/system.mk

SUBDIRS = base net tests

.PHONY: docs subdirs $(SUBDIRS)

all: subdirs docs

docs:
	doxygen Doxyfile

net: base

include $(TOP)/make/rules.mk
