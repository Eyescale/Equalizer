#!gmake
.PHONY: debug tests cdash release xcode debug_glx docs clean clobber

BUILD ?= debug
PYTHON ?= python
CMAKE ?= cmake

all: $(BUILD) RELNOTES.txt README.rst
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

package: release/Makefile ../equalizergraphics.com/build/documents/Developer/API
	@$(MAKE) -C release doxygen
	@$(MAKE) -C release package

xcode:
	@mkdir -p XCode
	@cd XCode; $(CMAKE) -G Xcode .. -DCMAKE_INSTALL_PREFIX:PATH=install
	open XCode/Equalizer.xcodeproj

tests: debug/Makefile
	@$(MAKE) -C debug tests

docs: ../equalizergraphics.com/build/documents/Developer/API
	@$(MAKE) -C $(BUILD) doxygen

.PHONY: ../equalizergraphics.com/build/documents/Developer/API/internal
../equalizergraphics.com/build/documents/Developer/API/internal:
	@mkdir -p ../equalizergraphics.com/build/documents/Developer/API/internal

.PHONY: ../equalizergraphics.com/build/documents/Developer/API
../equalizergraphics.com/build/documents/Developer/API: ../equalizergraphics.com/build/documents/Developer/API/internal $(BUILD)/Makefile
	@mkdir -p ../equalizergraphics.com/build/collage/documents/Developer/API

RELNOTES.txt: libs/RelNotes.dox
	-links -dump -width 65 $< > $@.tmp && mv $@.tmp $@

README.rst: libs/RelNotes.dox
	-$(PYTHON) CMake/html2rst.py $< > $@
