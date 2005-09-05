
VARIANTS    = ppc ppc64

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif


DSO_LDFLAGS += -dynamiclib
DSO_SUFFIX   = dylib

AR           = libtool
ARFLAGS      = -static
