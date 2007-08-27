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
CLEAN_EXTRA = $(INSTALL_CMD) $(INSTALL_FILES)

include make/rules.mk

docs: lib
	@$(DOXYGEN) Doxyfile

subdirs: precompile
lib: externals
proto: lib
tests: lib
examples: lib
server: lib

postcompile: $(INSTALL_CMD) subdirs
	@echo "----- Compilation successful -----"
ifeq (Darwin,$(ARCH))
	@echo "Set DYLD_LIBRARY_PATH to $(PWD)/$(LIBRARY_DIR)"
else
	@echo "Set LD_LIBRARY_PATH to $(PWD)/$(BUILD_DIR)/$(word 1, $(VARIANTS))/lib"
endif

$(INSTALL_CMD): subdirs
	@sort $@ | sort -u > .$@.tmp && mv .$@.tmp $@
