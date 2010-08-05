
include $(TOP)/make/system.mk

SUBDIRS = \
	configs \
	eqHello \
	eqPly \
	eVolve \
	eqPixelBench \
	osgScaleViewer

ifdef CUDA
SUBDIRS += eqNBody
endif

TARGETS = subdirs

include $(TOP)/make/rules.mk
