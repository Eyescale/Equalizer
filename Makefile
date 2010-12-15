#!gmake
.PHONY: debug tests cdash release xcode debug_glx docs docs/install clean

all: debug RELNOTES.txt README.rst
clean:
	rm -rf debug release docs XCode debug_glx man cdash

DOXYGEN ?= doxygen

debug: debug/Makefile
	@$(MAKE) -C debug

tests: debug/Makefile
	@$(MAKE) -C debug check

debug/Makefile:
	@mkdir -p debug
	@cd debug; cmake ..

cdash: cdash/Makefile
	@$(MAKE) -C cdash Continuous

cdash/Makefile:
	@mkdir -p cdash
	@cd cdash; cmake ..

release: release/Makefile
	@$(MAKE) -C release

release/Makefile:
	@mkdir -p release
	@cd release; cmake .. -DCMAKE_BUILD_TYPE=Release


xcode: XCode/Equalizer.xcodeproj
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
	$(DOXYGEN) Doxyfile.int

../website/build/documents/Developer/API: ../website/build/documents/Developer/API/internal docs/install Doxyfile.ext
	$(DOXYGEN) Doxyfile.ext

docs/install: docs/Makefile
	@$(MAKE) -C docs install

docs/Makefile:
	@mkdir -p docs
	@cd docs; cmake -D CMAKE_INSTALL_PREFIX:STRING=install ..


RELNOTES.txt: lib/RelNotes.dox
	-links -dump $< > $@.tmp && mv $@.tmp $@

README.rst: lib/RelNotes.dox
	-$(PYTHON) make/html2rst.py $< > $@
