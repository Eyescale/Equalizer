#!gmake

include make/system.mk

OPTIONAL = \
	$(wildcard auxilary)

SUBDIRS = \
	externals \
	lib \
	server \
	admin \
	tools \
	examples \
	tests \
	$(OPTIONAL)

.PHONY: docs

TARGETS     = precompile subdirs RELNOTES README.rst postcompile # docs
CLEAN_EXTRA = obj build $(INSTALL_FILES)

include make/rules.mk

docs:
	@$(DOXYGEN) Doxyfile

lib: precompile externals
tests: lib server
examples: lib admin
contrib: lib
server: lib admin
tools: server
auxilary: lib
admin: lib
dev: examples server
	@echo "Development essentials compiled successfully"

postcompile: subdirs RELNOTES README.rst
	@echo
	@echo "----- Compilation successful -----"
	@echo $(if $(findstring NDEBUG,$(CXXFLAGS)),Release,Debug) \
		build of Equalizer with support for:

ifeq ($(findstring AGL, $(WINDOW_SYSTEM)),AGL)
	@echo "    AGL/Carbon windowing"
endif
ifeq ($(findstring GLX, $(WINDOW_SYSTEM)),GLX)
	@echo "    glX/X11 windowing"
endif
ifeq ($(findstring WGL, $(WINDOW_SYSTEM)),WGL)
	@echo "    WGL/Win32 windowing"
endif
ifeq ($(findstring EQ_USE_OPENMP, $(DEFFLAGS)),EQ_USE_OPENMP)
	@echo "    OpenMP (http://www.openmp.org/)"
endif
ifeq ($(findstring EQ_USE_PARACOMP, $(DEFFLAGS)),EQ_USE_PARACOMP)
	@echo "    Paracomp (http://paracomp.sourceforge.net/)"
endif
ifeq ($(findstring EQ_USE_CUDA, $(DEFFLAGS)),EQ_USE_CUDA)
	@echo "    CUDA (http://www.nvidia.com/object/cuda_home.html)"
endif
ifeq ($(findstring EQ_USE_BOOST, $(DEFFLAGS)),EQ_USE_BOOST)
	@echo "    RSP (reliable UDP-based multicast)"
endif
	@echo
ifeq (Darwin,$(ARCH))
	@echo "Set DYLD_LIBRARY_PATH to $(LD_PATH)"
	@echo "  bash: 'export DYLD_LIBRARY_PATH=$(LD_PATH)'"
	@echo "  *csh: 'setenv DYLD_LIBRARY_PATH $(LD_PATH)'"
else
	@echo "Set LD_LIBRARY_PATH to $(LD_PATH)"
	@echo "  bash: 'export LD_LIBRARY_PATH=$(LD_PATH)'"
	@echo "  *csh: 'setenv LD_LIBRARY_PATH $(LD_PATH)'"
endif
	@echo

RELNOTES: lib/RelNotes.dox
	-links -dump $< > $@.tmp && mv $@.tmp $@

README.rst: lib/RelNotes.dox
	-$(PYTHON) make/html2rst.py $< > $@
