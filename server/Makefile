
include $(TOP)/make/system.mk

SOURCES = \
	channel.cpp \
	compound.cpp \
	config.cpp \
	connectionDescription.cpp \
	frame.cpp \
	frameData.cpp \
	global.cpp \
	lexer.cpp \
	loader.cpp \
	node.cpp \
	parser.cpp \
	pipe.cpp \
	projection.cpp \
	server.cpp \
	startLocalServer.cpp \
	swapBarrier.cpp \
	view.cpp \
	wall.cpp \
	window.cpp

CXXFILES = \
	eqServer.cpp
INSTALL  = 1

CLEAN_EXTRA = \
	lexer.cpp \
	parser.cpp

MODULE  = server

CXXFLAGS    += $(WINDOW_SYSTEM_INCS) -DEQUALIZERSERVERLIBRARY_EXPORTS
LDFLAGS     += -leq
SA_LDFLAGS  += -leq -leqserver

ifeq "${ARCH}" "Linux"
  SA_CXXFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(DYNAMIC_LIB) $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(DYNAMIC_LIB)
$(DYNAMIC_LIB): lexer.cpp parser.cpp

lexer.cpp: loader.l parser.cpp
	$(FLEX) -o$@ $<

parser.cpp: loader.y
	$(BISON) -l -p eqLoader_ -d $< -o $@

parser.hpp: parser.cpp
