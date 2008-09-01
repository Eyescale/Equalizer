
include $(TOP)/make/system.mk

CXXFILES = \
	channel.cpp \
	channelUpdateVisitor.cpp \
	compound.cpp \
	compoundExitVisitor.cpp \
	compoundInitVisitor.cpp \
	compoundUpdateDataVisitor.cpp \
	compoundUpdateInputVisitor.cpp \
	compoundUpdateOutputVisitor.cpp \
	config.cpp \
	configUpdateDataVisitor.cpp \
	connectionDescription.cpp \
	frame.cpp \
	frameData.cpp \
	global.cpp \
	loadBalancer.cpp \
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

SIMPLE_CXXFILES = \
	eqServer.cpp

CLEAN_EXTRA = \
	lexer.cpp \
	parser.cpp \
	parser.hpp


ifeq ($(findstring CYGWIN_NT_5, $(ARCH)),CYGWIN_NT_5)
  CXXFILES += lexer_cygwin.cpp
else
  CXXFILES += lexer.cpp
endif

MODULE  = eqserver

CXXFLAGS    += $(WINDOW_SYSTEM_INCS) -DEQUALIZERSERVERLIBRARY_EXPORTS
LDFLAGS     += -leq
SA_LDFLAGS  += -leqserver

ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(DYNAMIC_LIB) $(SIMPLE_PROGRAMS)

include $(TOP)/make/rules.mk

$(SIMPLE_PROGRAMS): $(DYNAMIC_LIB)
$(DYNAMIC_LIB): lexer.cpp parser.hpp

lexer_cygwin.cpp: lexer.cpp

lexer.cpp: loader.l parser.cpp
	$(FLEX) -o$@ $<

parser.cpp: loader.y
	$(BISON) -l -p eqLoader_ -d $< -o $@

parser.hpp: parser.cpp
	@-mv parser.cpp.h parser.hpp
