
include $(TOP)/make/system.mk

OPTIONAL = \
	$(wildcard WorldSkin) \
	$(wildcard god)

SUBDIRS = \
	configs \
	eqHello \
	eqPly \
	eqPixelBench \
	$(OPTIONAL)

TARGETS = subdirs

ifndef SKIP_EVOLVE
  SUBDIRS += eVolve
endif

include $(TOP)/make/rules.mk
