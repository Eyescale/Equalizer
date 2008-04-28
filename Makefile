#!gmake

include make/system.mk

SUBDIRS = \
	externals \
	lib \
	server \
	tools \
	examples \
	contrib \
	proto \
	tests

.PHONY: docs

TARGETS     = precompile subdirs postcompile # docs
CLEAN_EXTRA = $(INSTALL_FILES)

include make/rules.mk

docs:
	@$(DOXYGEN) Doxyfile

lib: precompile
lib: externals
proto: lib
tests: lib server
examples: lib
contrib: lib
server: lib
tools: lib

postcompile: subdirs
	@echo "----- Compilation successful -----"
ifeq (Darwin,$(ARCH))
	@echo "Set DYLD_LIBRARY_PATH to $(PWD)/$(LIBRARY_DIR)"
else
	@echo "Set LD_LIBRARY_PATH to $(PWD)/$(LIBRARY_DIR)"
endif

RELNOTES: ../website/build/documents/RelNotes/RelNotes_0.5.0.html
	links -dump $< > $@
