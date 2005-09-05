
VARIANT     = n64

CC          = cc
CXX         = CC
CXX_DEPS    = g++

DSO_SUFFIX  = so
DSO_LDFLAGS = -shared

AR          = CC
ARFLAGS     = -ar -o

DOXYGEN     = ls

CXXFLAGS   += -LANG:std -64
LDFLAGS    += -64 -hidden_symbol dummy
