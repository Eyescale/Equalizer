#!gmake
.PHONY: debug tests cdash release xcode debug_glx docs clean clobber

BUILD ?= debug
CMAKE ?= cmake

ifeq ($(wildcard Makefile), Makefile)
all:
	@$(MAKE) -f Makefile $(MAKECMDGOALS)

clean:
	@$(MAKE) -f Makefile $(MAKECMDGOALS)

.DEFAULT:
	@$(MAKE) -f Makefile $(MAKECMDGOALS)

else

default: $(BUILD)
all: debug release
clobber:
	rm -rf debug release XCode debug_glx man cdash
clean:
	@-$(MAKE) -C debug clean
	@-$(MAKE) -C release clean
	@-$(MAKE) -C XCode clean
	@-$(MAKE) -C debug_glx clean
	@-$(MAKE) -C cdash clean
	@rm -rf man

.DEFAULT:
	@$(MAKE) -C $(BUILD) $(MAKECMDGOALS)

debug: debug/Makefile
	@$(MAKE) -C $@

release: release/Makefile
	@$(MAKE) -C $@

debug_glx: debug_glx/Makefile
	@$(MAKE) -C $@

cdash: cdash/Makefile
	@$(MAKE) -C cdash clean
	@$(MAKE) -C cdash Continuous

debug/Makefile:
	@mkdir -p debug
	@cd debug; $(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX:PATH=install -DEQUALIZER_RUN_GPU_TESTS=ON

release/Makefile:
	@mkdir -p release
	@cd release; $(CMAKE) .. -DCMAKE_BUILD_TYPE=Release

cdash/Makefile:
	@mkdir -p cdash
	@cd cdash; env CXXFLAGS="-fprofile-arcs -ftest-coverage" CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs -ftest-coverage" $(CMAKE) ..

debug_glx/Makefile:
	@mkdir -p debug_glx
	@cd debug_glx; $(CMAKE) .. -DEQUALIZER_PREFER_AGL=OFF

package: release/Makefile ../equalizergraphics/build/documents/Developer/API
	@$(MAKE) -C release doxygen
	@$(MAKE) -C release package

XCode/Equalizer.xcodeproj: CMakeLists.txt
	@mkdir -p XCode
	@cd XCode; $(CMAKE) -G Xcode .. -DCMAKE_INSTALL_PREFIX:PATH=install

xcode: XCode/Equalizer.xcodeproj
	open XCode/Equalizer.xcodeproj

tests: debug/Makefile
	@$(MAKE) -C debug tests

docs: ../equalizergraphics/build/documents/Developer/API
	@$(MAKE) -C $(BUILD) doxygen

.PHONY: ../equalizergraphics/build/documents/Developer/API/internal
../equalizergraphics/build/documents/Developer/API/internal:
	@mkdir -p ../equalizergraphics/build/documents/Developer/API/internal

.PHONY: ../equalizergraphics/build/documents/Developer/API
../equalizergraphics/build/documents/Developer/API: ../equalizergraphics/build/documents/Developer/API/internal $(BUILD)/Makefile
	@mkdir -p ../equalizergraphics/build/collage/documents/Developer/API

endif
