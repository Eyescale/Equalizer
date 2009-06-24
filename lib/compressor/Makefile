
include $(TOP)/make/system.mk

MODULE = eqCompressorRLE

CXXFILES = \
	compressor.cpp \
	compressorRLE.cpp \
	compressorRLE3B.cpp \
	compressorRLE4B.cpp \
	compressorRLE4F.cpp \
	compressorRLE4HF.cpp \
	compressorRLEByte.cpp

CXXFLAGS += $(WINDOW_SYSTEM_INCS) -DEQUALIZER_EXPORTS
CFLAGS   += -DGLEW_BUILD -DEQ_DEFINES_H
LDFLAGS  += $(WINDOW_SYSTEM_LIBS)

TARGETS = $(DYNAMIC_LIB)

include $(TOP)/make/rules.mk
