
CXXFLAGS           += -DWIN32 -D_WIN32
CFLAGS             += -DWIN32 -D_WIN32
DSO_LDFLAGS        += -shared
DSO_SUFFIX          = dll
WINDOW_SYSTEM      ?= WGL
AR                  = ld
ARFLAGS             = -r
WINDOW_SYSTEM_LIBS += -lopengl32 -lgdi32
LDFLAGS            += -lrpcrt4 -lws2_32 -lmswsock
IMP_LIB             = $(@D)/$(@F:lib%=cyg%)
DSO_LDFLAGS        += -Wl,--out-implib=$(IMP_LIB) \
                      -Wl,--export-all-symbols -Wl,--enable-auto-import

SKIP_EVOLVE         = 1