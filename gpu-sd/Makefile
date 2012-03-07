#!gmake
.PHONY: debug tests release clean clobber package

BUILD ?= debug
CMAKE ?= cmake

all: $(BUILD)
clobber:
	rm -rf debug release
clean:
	@-$(MAKE) -C debug clean
	@-$(MAKE) -C release clean

.DEFAULT:
	@$(MAKE) -C $(BUILD) $(MAKECMDGOALS)

debug: debug/Makefile
	@$(MAKE) -C $@

release: release/Makefile
	@$(MAKE) -C $@

debug/Makefile:
	@mkdir -p debug
	@cd debug; $(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX:PATH=install

release/Makefile:
	@mkdir -p release
	@cd release; $(CMAKE) .. -DCMAKE_BUILD_TYPE=Release

package: release/Makefile
	@$(MAKE) -C release gpusd_doxygen
	@$(MAKE) -C release package

tests: debug/Makefile
	@$(MAKE) -C debug tests
