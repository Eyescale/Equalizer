
include $(TOP)/make/system.mk

PROGRAM = eqVol

SOURCES = \
	localInitData.cpp \
	rawConverter.cpp \
	channel.cpp \
	config.cpp \
	eqVol.cpp \
	initData.cpp \
	main.cpp \
	node.cpp \
	pipe.cpp \
	shader.cpp\
	tracker.cpp \
	rawVolModel.cpp \
	window.cpp 

CXXFLAGS    += -I../include $(WINDOW_SYSTEM_INCS)
SA_LDFLAGS  += -leq -lpthread $(WINDOW_SYSTEM_LIBS)

ifeq "${ARCH}" "Darwin"
  LDFLAGS += -framework Cg
endif


ifeq "${ARCH}" "Linux"
  SA_LDFLAGS += -lrt
ifeq "${SUBARCH}" "ia64"
    SA_LDFLAGS += -luuid
endif
endif

TARGETS = $(PROGRAMS)

include $(TOP)/make/rules.mk

README: ../../../website/build/documents/user/eqVol.html
	links -dump $< > $@
