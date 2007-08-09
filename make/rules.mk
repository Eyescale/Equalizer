
.PHONY: subdirs clean $(SUBDIRS) install build_variants
.SECONDARY: $(OBJECT_DIR)/%.o %.o %.d $(OBJECT_DIR)/%.d $(DEPENDENCIES)

all: $(TARGETS)

# top level precompile command(s)
precompile: $(CXX_DEFINES_FILE)

$(CXX_DEFINES_FILE)::
	@echo "/* Generated from CXXFLAGS during build */" > $@.tmp
	@echo "#ifndef EQ_DEFINES_H" >> $@.tmp
	@echo "#define EQ_DEFINES_H" >> $@.tmp
	@for line in $(CXX_DEFINES_TXT); do  \
		echo "#ifndef $$line" >> $@.tmp ;\
		echo "#  define $$line" >> $@.tmp ;\
		echo "#endif" >> $@.tmp ;\
	done
	@echo "#endif // EQ_DEFINES_H" >> $@.tmp
	@cmp -s $@ $@.tmp || cp $@.tmp $@
	@rm $@.tmp

# recursive subdir rules
subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) SUBDIR=$(SUBDIR)/$@ -C $@


# installing and packaging
install:
	/bin/sh $(INSTALL_CMD)

rpm: $(INSTALL_FILES)
	@echo "check $(INSTALL_FILES), run 'rpmbuild -ba make/Equalizer.spec' as root"

$(INSTALL_FILES):
	@cat $(INSTALL_CMD) | awk '{print $$7;}' | sed 's/.DESTDIR//' | sort | sort -u > $@

# includes
$(INCLUDE_DIR)/%: %
	@mkdir -p $(@D)
	@echo 'Include file $@'
	@cp $< $@
	@echo "mkdir -p \$$DESTDIR/$(dir $(INSTALL_INCLUDE_DIR)/$<);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_INCLUDE_DIR)/$<" \
	    >> $(INSTALL_CMD)

# libraries
$(FAT_DYNAMIC_LIB): $(THIN_DYNAMIC_LIBS)
ifndef VARIANT
	@mkdir -p $(@D)
	lipo -create $(THIN_DYNAMIC_LIBS) -output $@
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LIB_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_LIB_DIR)/$(@F)" \
	    >> $(INSTALL_CMD)
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LDSO_DIR);" \
	      "echo $(INSTALL_LIB_DIR) >> \$$DESTDIR/$(INSTALL_LDSO_CONF)" \
	    >> $(INSTALL_CMD)
endif

$(THIN_DYNAMIC_LIBS): $(PCHEADERS) $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(@D)
	$(CXX) $(LINKDIRS) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
ifndef BUILD_FAT
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LIB_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_LIB_DIR)/$(@F)" \
	    >> $(INSTALL_CMD) 
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LDSO_DIR);" \
	      "echo $(INSTALL_LIB_DIR) >> \$$DESTDIR/$(INSTALL_LDSO_CONF)" \
	    >> $(INSTALL_CMD)
endif
else
	@$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).$(DSO_SUFFIX)=%) TOP=$(TOP) $@
endif

$(FAT_STATIC_LIB): $(THIN_STATIC_LIBS)
ifndef VARIANT
	@mkdir -p $(@D)
	lipo -create $(THIN_STATIC_LIBS) -output $@
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LIB_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_LIB_DIR)/$(@F)" \
	    >> $(INSTALL_CMD) 
endif

$(THIN_STATIC_LIBS): $(PCHEADERS) $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(@D)
	@rm -f $@
	$(AR) $(LINKDIRS) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
ifndef BUILD_FAT
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_LIB_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_LIB_DIR)/$(@F)" \
	    >> $(INSTALL_CMD) 
endif
else
	@$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).a=%) TOP=$(TOP) $@
endif

$(OBJECT_DIR)/%.h.gch: %.h
	@mkdir -p $(@D)
	$(CXX) -x c++-header $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@

$(OBJECT_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MD -MF $@.d -c $< -o $@

# executables
$(FAT_PROGRAM): $(THIN_PROGRAMS)
ifndef VARIANT
	lipo -create $(THIN_PROGRAMS) -output $@
ifndef NOINSTALL
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_BIN_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_BIN_DIR)/$(@F)" \
	    >> $(INSTALL_CMD) 
endif
endif

$(THIN_PROGRAMS): $(PCHEADERS) $(OBJECTS)
ifdef VARIANT
	$(CXX) $(INCLUDEDIRS) $(CXXFLAGS) $(OBJECTS) $(LINKDIRS) $(SA_LDFLAGS) $(LDFLAGS) -o $@
ifndef BUILD_FAT
ifndef NOINSTALL
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_BIN_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_BIN_DIR)/$(@F)" \
	    >> $(INSTALL_CMD)
endif
endif
else
	@$(MAKE) VARIANT=$(subst .,,$(suffix $@)) TOP=$(TOP) $@
endif

ifndef PROGRAM
ifdef VARIANT
%.$(VARIANT): %.cpp
	$(CXX) $< $(INCLUDEDIRS) $(CXXFLAGS) $(LINKDIRS) $(LDFLAGS) -DSUBDIR=\"$(SUBDIR)\" $(SA_LDFLAGS) $(SA_CXXFLAGS) -MD -MF $@.d -o $@ 
ifndef BUILD_FAT
ifdef INSTALL
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_BIN_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_BIN_DIR)/$(@F)" \
	    >> $(INSTALL_CMD)
endif
endif

else # VARIANT

$(FAT_SIMPLE_PROGRAMS): $(THIN_SIMPLE_PROGRAMS)
	lipo -create $(foreach V,$(VARIANTS),$@.$(V)) -output $@
ifdef INSTALL
	@echo "mkdir -p \$$DESTDIR/$(INSTALL_BIN_DIR);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_BIN_DIR)/$(@F)" \
	    >> $(INSTALL_CMD)
endif

$(THIN_SIMPLE_PROGRAMS): $(CXXFILES)
	@$(MAKE) VARIANT=$(subst .,,$(suffix $@)) TOP=$(TOP) $@
endif # VARIANT
endif # PROGRAMS

testRun.%: %
	env EQ_LOG_LEVEL=WARN LD_LIBRARY_PATH=$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib DYLD_LIBRARY_PATH=$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib ./$< && touch $@ || rm -f $@

# cleaning targets
clean:
ifdef VARIANT
	rm -f *~ .*~ $(TARGETS) $(CLEAN_EXTRA) $(DEPENDENCIES)
	rm -rf $(OBJECT_DIR) $(BUILD_DIR)
ifdef SUBDIRS
	@for dir in $(SUBDIRS); do \
		echo "$(DEPTH) $$dir clean"; \
		$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) -C $$dir $@ ;\
	done
endif
else # VARIANT
	@for variant in $(VARIANTS); do \
		$(MAKE) VARIANT=$$variant clean; \
	done
endif

# dependencies
ifdef DEPENDENCIES
-include $(DEPENDENCIES)
endif
