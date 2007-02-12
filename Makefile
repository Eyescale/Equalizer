#!gmake

include make/system.mk

SUBDIRS = \
	lib \
	examples \
	proto \
	server \
	tests

.PHONY: docs

TARGETS = precompile subdirs postcompile # docs

include make/rules.mk

docs: lib
	@$(DOXYGEN) Doxyfile

lib: precompile
proto: lib
tests: lib
examples: lib
server: lib

postcompile: subdirs
	@echo "----- Compilation successful -----"
ifeq (Darwin,$(ARCH))
	@echo "Set DYLD_LIBRARY_PATH to $(PWD)/$(LIBRARY_DIR)"
else
	@echo "Set LD_LIBRARY_PATH to $(PWD)/$(BUILD_DIR)/$(word 1, $(VARIANTS))/lib"
endif
