
CC          = cc
CXX         = CC
DEP_CXX     = g++

DSO_SUFFIX  = so
DSO_LDFLAGS = -shared

AR          = CC
ARFLAGS     = -ar -o

DOXYGEN     = ls

CXXFLAGS   += -LANG:std -64
LDFLAGS    += -64 -hidden_symbol dummy
