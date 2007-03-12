
include $(TOP)/make/system.mk

PROGRAM = test

SOURCES = \
	test.cpp \
	vertexData.cpp

TESTS = $(PROGRAMS:%=testRun.%)

CXXFLAGS    += -I../../tests $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS += -lpthread -leq

ifeq "${ARCH}" "Linux"
SA_CXXFLAGS += -lrt -luuid
endif

TARGETS = $(PROGRAMS) $(TESTS)

include $(TOP)/make/rules.mk
