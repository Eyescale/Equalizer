
include $(TOP)/make/system.mk

PROGRAM = eqServer

SOURCES = \
	channel.cpp \
	config.cpp \
	connectionDescription.cpp \
	compound.cpp \
	frame.cpp \
	frameData.cpp \
	global.cpp \
	loader.cpp \
	node.cpp \
	pipe.cpp \
	server.cpp \
	swapBarrier.cpp \
	window.cpp \
	lexer.cpp \
	parser.cpp \
	eqServer.cpp

CLEAN_EXTRA = \
	lexer.cpp \
	parser.cpp

CXXFLAGS    += $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk

$(PROGRAMS): lexer.cpp parser.cpp

lexer.cpp: loader.l parser.cpp
	$(FLEX) -o$@ $<

parser.cpp: loader.y
	$(BISON) -l -p eqLoader_ -d $< -o $@

parser.hpp: parser.cpp
