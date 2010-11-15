#!gmake
.PHONY: debug tests release xcode debug_glx docs docs/install

all: debug RELNOTES.txt README.rst

DOXYGEN ?= doxygen

debug: debug/Makefile
	@$(MAKE) -C debug

tests: debug/Makefile
	@$(MAKE) -C debug check

debug/Makefile:
	@mkdir -p debug
	@cd debug; cmake ..


release: release/Makefile
	@$(MAKE) -C release

release/Makefile:
	@mkdir -p release
	@cd release; cmake .. -DCMAKE_BUILD_TYPE=Release


xcode: XCode/Equalizer.xcodeproj
	open XCode/Equalizer.xcodeproj

XCode/Equalizer.xcodeproj:
	@mkdir -p XCode
	@cd XCode; cmake -G Xcode ..

debug_glx: debug_glx/Makefile
	@$(MAKE) -C debug_glx

debug_glx/Makefile:
	@mkdir -p debug_glx
	@cd debug_glx; cmake .. -DEQ_PREFER_AGL=0


docs: ../website/build/documents/Developer/API/internal ../website/build/documents/Developer/API

../website/build/documents/Developer/API/internal: lib Doxyfile.int
	$(DOXYGEN) Doxyfile.int

../website/build/documents/Developer/API: docs/install Doxyfile.ext
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
