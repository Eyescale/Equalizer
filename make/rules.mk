
.PHONY: subdirs clean $(SUBDIRS) install
.SECONDARY: $(OBJECT_DIR)/%.o %.o %.d $(OBJECT_DIR)/%.d $(DEPENDENCIES)

ifndef BUILD_MODE
debug: CXXFLAGS += -g -Werror
debug: BUILD_MODE = Debug
debug: all

release: CXXFLAGS += -O2 -Werror -Wuninitialized -DNDEBUG
release: BUILD_MODE = Release
release: all
endif

all:  $(BUILD_MODE_FILE) $(TARGETS)

# top level precompile command(s)
precompile: $(CXX_DEFINES_FILE)

$(CXX_DEFINES_FILE)::
	@echo "#ifndef EQBASE_DEFINES_$(ARCH)_H" >> $@.tmp
	@echo "#define EQBASE_DEFINES_$(ARCH)_H" >> $@.tmp
	@for line in $(CXX_DEFINES_TXT); do  \
		echo "#ifndef $$line" >> $@.tmp ;\
		echo "#  define $$line" >> $@.tmp ;\
		echo "#endif" >> $@.tmp ;\
	done
	@echo "#endif // EQBASE_DEFINES_$(ARCH)_H" >> $@.tmp
	@cmp -s $@ $@.tmp || cp $@.tmp $@
	@rm $@.tmp

$(BUILD_MODE_FILE):
	@rm -rf $(BIN_DIR) $(LIBRARY_DIR) $(BUILD_DIR)/._Debug $(BUILD_DIR)/._Release
	@mkdir -p $(BUILD_DIR)
	@touch $@
	@echo "Switched to $(BUILD_MODE) build"

$(TARGETS): $(BUILD_MODE_FILE)

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

# shaders
%_glsl.h: %.glsl
	$(SHADERS_PARSER) $< > $@

# includes
$(INCLUDE_DIR)/%: %
	@mkdir -p $(@D)
	@echo 'Include file $@'
	@cp $< $@

# libraries
#  'functions'
ifdef GCCWAR
define compile
  @$(CXX) $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -c $< -MT $@
  $(CXX) $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@ || \
  $(CXX) $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) -O1 -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@
endef
else
define compile
  @$(CXX) $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -c $< -MT $@
  $(CXX) $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -c $< -o $@
endef
endif

$(DYNAMIC_LIB): $(PCHEADERS) $(OBJECTS)
	@mkdir -p $(@D)
	$(LD) $(LINKDIRS) $(ARCHFLAGS) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(STATIC_LIB): $(PCHEADERS) $(OBJECTS)
	@mkdir -p $(@D)
	@rm -f $@
	$(AR) $(LINKDIRS) $(ARCHFLAGS) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.h.gch: %.h
	@echo Precompiling $<
	@$(compile)

$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o: %.cpp
	@mkdir -p $(@D)
	$(compile)

$(OBJECT_DIR)/%.$(OBJECT_SUFFIX).o: %.c
	@mkdir -p $(@D)
	@$(CC) $(INCLUDEDIRS) $(CFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -c $< -MT $@
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
	@$(CXX) $< $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -MT $@
	$(CXX) $< $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) $(LINKDIRS) $(LDFLAGS) -DSUBDIR=\"$(SUBDIR)\" $(SA_LDFLAGS) -o $@ 

$(BIN_DIR)/%: %.mm
	@mkdir -p $(@D)
	@$(CXX) $< $(INCLUDEDIRS) $(CXXFLAGS) -DSUBDIR=\"$(SUBDIR)\" -MM -MF $@.d -MT $@
	$(CXX) $< $(INCLUDEDIRS) $(ARCHFLAGS) $(CXXFLAGS) $(LINKDIRS) $(LDFLAGS) -DSUBDIR=\"$(SUBDIR)\" $(SA_LDFLAGS) -o $@ 
endif # PROGRAM

%.testOK: %
	@echo "Running $<"
	@env EQ_LOG_LEVEL=WARN \
		LD_LIBRARY_PATH="$(LD_PATH):$(LD_LIBRARY_PATH)" \
		DYLD_LIBRARY_PATH="$(LD_PATH):$(DYLD_LIBRARY_PATH)" \
		PATH="$(PATH):$(BUILD_DIR)/$(subst .,,$(suffix $<))/lib" \
		./$<

# cleaning targets
clean:
	rm -rf *~ .*~ $(CLEAN_EXTRA) $(TARGETS) $(DEPENDENCIES) $(OBJECTS) $(PCHEADERS)
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
