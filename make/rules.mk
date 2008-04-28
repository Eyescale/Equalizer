
.PHONY: subdirs clean $(SUBDIRS) install
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
	@$(MAKE) TOP=$(SUBDIRTOP) SUBDIR=$(SUBDIR)/$@ -C $@


# installing and packaging
install:
	/bin/sh $(INSTALL_CMD)

rpm: $(INSTALL_FILES)
	@echo "check $(INSTALL_FILES), run 'rpmbuild -ba make/Equalizer.spec' as root"

$(INSTALL_FILES):
	@find $(BUILD_DIR) -type f > $@

# includes
$(INCLUDE_DIR)/%: %
	@mkdir -p $(@D)
	@echo 'Include file $@'
	@cp $< $@

# libraries
$(DYNAMIC_LIB): $(PCHEADERS) $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) $(LINKDIRS) $(ARCHFLAGS) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(STATIC_LIB): $(PCHEADERS) $(OBJECTS)
	@mkdir -p $(@D)
	@rm -f $@
	$(AR) $(LINKDIRS) $(ARCHFLAGS) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECT_DIR)/%.h.gch: %.h
	@mkdir -p $(@D)
	$(CXX) -x c++-header $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@

$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -c $<
	$(CXX) $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@

$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o: %.c
	@mkdir -p $(@D)
	@$(CC) $(INCLUDEDIRS) $(CFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -c $<
	$(CC) $(INCLUDEDIRS) $(ARCHFLAGS) $(CFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@

# executables
$(PROGRAM_APP): $(PROGRAM_EXE)
	@mkdir -p $(@D)
	ln -f $< $@

$(PROGRAM_EXE): $(PCHEADERS) $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) $(INCLUDEDIRS) $(OBJECTS) $(LINKDIRS) $(ARCHFLAGS) $(LDFLAGS) -o $@

ifndef PROGRAM
$(BIN_DIR)/%: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $< $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d
	$(CXX) $< $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) $(LINKDIRS) $(LDFLAGS) -DSUBDIR=\"$(SUBDIR)\" $(SA_LDFLAGS) -o $@ 
endif # PROGRAMS

%.testOk: %
	env EQ_LOG_LEVEL=WARN \
		LD_LIBRARY_PATH="$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib" \
		DYLD_LIBRARY_PATH="$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib" \
		PATH="$(PATH):$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib" \
		./$< && touch $@ || rm -f $@

# cleaning targets
clean:
	rm -f *~ .*~ $(TARGETS) $(CLEAN_EXTRA) $(DEPENDENCIES) $(OBJECTS)
ifdef SUBDIRS
	@for dir in $(SUBDIRS); do \
		echo "$(DEPTH) $$dir clean"; \
		$(MAKE) TOP=$(SUBDIRTOP) SUBDIR=$(SUBDIR)/$$dir -C $$dir $@ ;\
	done
endif

# dependencies
ifdef DEPENDENCIES
-include $(DEPENDENCIES)
endif
