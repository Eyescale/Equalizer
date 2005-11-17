
VARIANTS    = ppc ppc64

ifdef VARIANT
  CXXFLAGS    += -arch $(VARIANT)
  DSO_LDFLAGS += -arch $(VARIANT)
endif

# export MACOSX_DEPLOYMENT_TARGET = 10.3

DSO_LDFLAGS += -dynamiclib -flat_namespace -undefined warning #dynamic_lookup
DSO_SUFFIX   = dylib

AR           = libtool
ARFLAGS      = -static
