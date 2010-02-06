
CXXFLAGS           += -DWIN32 -DWIN32_API -I$(TOP)/Windows/pthreads/include
CFLAGS             += -DWIN32 -D_WIN32
DSO_LDFLAGS        += -shared
DSO_SUFFIX          = dll
WINDOW_SYSTEM      ?= WGL

SKIP_EVOLVE         = 1

AR                  = ld
ARFLAGS             = -r
FLEX                = $(TOP)/Windows/bin/flex.exe
BISON               = env BISON_SIMPLE=$(TOP)/Windows/bin/bison.simple $(TOP)/Windows/bin/bison.exe

WINDOW_SYSTEM_LIBS += -lopengl32 -lgdi32
LDFLAGS            += -lrpcrt4 -lws2_32 -lmswsock -L$(LIBRARY_DIR) -lpthread
IMP_LIB             = $(@D)/$(@F:lib%=cyg%)
DSO_LDFLAGS        += -Wl,--out-implib=$(IMP_LIB) \
                      -Wl,--export-all-symbols -Wl,--enable-auto-import
PTHREAD_LIBS = $(BUILD_DIR)/lib/pthread.dll) $(BUILD_DIR)/lib/pthread.def
