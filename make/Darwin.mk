
VARIANTS    = ppc ppc64

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif


DSO_LDFLAGS += -dynamiclib -flat_namespace
DSO_SUFFIX   = dylib

AR           = libtool
ARFLAGS      = -static
