#!gmake

include make/system.mk

SUBDIRS = \
	lib \
	examples \
	externals \
	proto \
	server \
	tests \
	tools

.PHONY: docs

TARGETS     = precompile subdirs postcompile # docs
CLEAN_EXTRA = $(INSTALL_FILES)

include make/rules.mk

docs:
	@$(DOXYGEN) Doxyfile

subdirs: precompile
lib: externals
proto: lib
tests: lib server
examples: lib
server: lib
tools: lib

postcompile: subdirs
	@echo "----- Compilation successful -----"
ifeq (Darwin,$(ARCH))
	@echo "Set DYLD_LIBRARY_PATH to $(PWD)/$(LIBRARY_DIR)"
else
	@echo "Set LD_LIBRARY_PATH to $(PWD)/$(BUILD_DIR)/$(word 1, $(VARIANTS))/lib"
endif

RELNOTES: ../website/build/documents/RelNotes/RelNotes_0.4.0.html
	links -dump $< > $@
