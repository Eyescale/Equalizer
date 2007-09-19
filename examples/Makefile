
include $(TOP)/make/system.mk

OPTIONAL = \
	$(wildcard WorldSkin) \
	$(wildcard god)

SUBDIRS = \
	configs \
	eqPly \
	eqPlyNew \
	libmesh \
	eqPixelBench \
	$(OPTIONAL)

ifdef CG_INSTALLED
SUBDIRS += eVolve
endif

TARGETS = subdirs

include $(TOP)/make/rules.mk
