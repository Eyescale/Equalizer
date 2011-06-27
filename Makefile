#!gmake
.PHONY: debug tests cdash release xcode debug_glx docs docs/install clean clobber

all: debug RELNOTES.txt README.rst
clobber:
	rm -rf debug release docs XCode debug_glx man cdash
clean:
	@-$(MAKE) -C debug clean
	@-$(MAKE) -C release clean
	@-$(MAKE) -C docs clean
	@-$(MAKE) -C XCode clean
	@-$(MAKE) -C debug_glx clean
	@-$(MAKE) -C cdash clean
	@rm -rf man

DOXYGEN ?= doxygen
PYTHON ?= python
CTEST ?= ctest

debug: debug/Makefile
	@$(MAKE) -C debug

tests: debug/Makefile
	@$(MAKE) -C debug check

debug/Makefile:
	@mkdir -p debug
	@cd debug; cmake ..

cdash: cdash/Makefile
	@$(MAKE) -C cdash clean
	@cd cdash; $(CTEST) -D Continuous

cdash/Makefile:
	@mkdir -p cdash
	@cd cdash; env CXXFLAGS="-fprofile-arcs -ftest-coverage" CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-fprofile-arcs -ftest-coverage" cmake ..

release: release/Makefile
	@$(MAKE) -C release

release/Makefile:
	@mkdir -p release
	@cd release; cmake .. -DCMAKE_BUILD_TYPE=Release

package: release/Makefile
	@$(MAKE) -C release clean
	@$(MAKE) -C release package

xcode:
	@mkdir -p XCode
	@cd XCode; cmake -G Xcode ..
	open XCode/Equalizer.xcodeproj

debug_glx: debug_glx/Makefile
	@$(MAKE) -C debug_glx

debug_glx/Makefile:
	@mkdir -p debug_glx
	@cd debug_glx; cmake .. -DEQUALIZER_PREFER_AGL=0


docs: ../website/build/documents/Developer/API

.PHONY: ../website/build/documents/Developer/API/internal
../website/build/documents/Developer/API/internal:
	@mkdir -p ../website/build/documents/Developer/API/internal
	$(DOXYGEN) doc/Doxyfile.int

../website/build/documents/Developer/API: ../website/build/documents/Developer/API/internal docs/install doc/Doxyfile.ext doc/Doxyfile.co doc/Doxyfile.seq
	@mkdir -p ../website/build/collage/documents/Developer/API
	$(DOXYGEN) doc/Doxyfile.ext
	$(DOXYGEN) doc/Doxyfile.co
	$(DOXYGEN) doc/Doxyfile.seq

docs/install: docs/Makefile
	@rm -rf $@
	@$(MAKE) -C docs install

docs/Makefile:
	@mkdir -p docs
	@cd docs; cmake -D CMAKE_INSTALL_PREFIX:STRING=install ..


RELNOTES.txt: libs/RelNotes.dox
	-links -dump -width 65 $< > $@.tmp && mv $@.tmp $@

README.rst: libs/RelNotes.dox
	-$(PYTHON) CMake/html2rst.py $< > $@
