
VARIANTS    = ppc ppc64 i386

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

DSO_SUFFIX  = dylib
DSO_LDFLAGS = -dynamiclib 

AR          = libtool
ARFLAGS     = -static
