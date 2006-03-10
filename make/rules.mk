
.PHONY: subdirs $(SUBDIRS) $(DEPENDENCIES)
.SUFFIXES: .d

all: $(TARGETS)

# top level precompile command(s)
precompile: $(CXX_DEFINES_FILE)

$(CXX_DEFINES_FILE)::
	@echo "/* Generated from CXX_DEFINES during build */" > $@.tmp
	@echo "#ifndef EQ_DEFINES_H" >> $@.tmp
	@echo "#define EQ_DEFINES_H" >> $@.tmp
	@for line in $(CXX_DEFINES_TXT); do  \
		echo "#define $$line" >> $@.tmp ;\
	done
	@echo "#endif // EQ_DEFINES_H" >> $@.tmp
	@cmp -s $@ $@.tmp || cp $@.tmp $@
	@rm $@.tmp

# recursive subdir rules
subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) SUBDIR=$(SUBDIR)/$@ -C $@


# headers
$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)/$(*D)
	@echo 'Header file $@'
	@cp $< $@

# generated source code: defunct
%Dist.cpp %Packets.h : %.h $(TOP)/make/codegen.pl
	$(TOP)/make/codegen.pl $<

ifdef HEADER_GEN
  SOURCES_GEN  = $(HEADER_GEN:%.h=%Dist.cpp)
  SOURCES     += $(SOURCES_GEN)
endif

# libraries
$(DYNAMIC_LIB): $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) $(INT_LDFLAGS) -o $@
else
	@$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).$(DSO_SUFFIX)=%) TOP=$(TOP) $@
endif

$(STATIC_LIB): $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) $(INT_LDFLAGS) -o $@
else
	@$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).a=%) TOP=$(TOP) $@
endif

OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(OBJECT_DIR)/%.o : %.cpp
	@mkdir -p $(@D)
	@echo -n "$(@D)/" > $(OBJECT_DIR)/$*.d
	@($(DEP_CXX) $(CXXFLAGS) $(INT_CXXFLAGS) -M -E $< >> \
		$(OBJECT_DIR)/$*.d ) || rm $(OBJECT_DIR)/$*.d
	$(CXX) $(CXXFLAGS) $(INT_CXXFLAGS) -c $< -o $@

%.cpp: $(OBJECT_DIR)/%d


# executables
$(PROGRAMS): $(OBJECTS)
ifdef VARIANT
	$(CXX) $(CXXFLAGS) $(SA_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
else
	@$(MAKE) VARIANT=$(subst .,,$(suffix $@)) TOP=$(TOP) $@
endif

% : %.cpp
	$(CXX) $< $(CXXFLAGS) $(INT_CXXFLAGS) $(SA_LDFLAGS) $(SA_CXXFLAGS) -o $@ 

# cleaning targets
clean:
ifdef VARIANT
	rm -f *~ .*~ $(OBJECTS) $(TARGETS) $(CLEAN) $(DEPENDENCIES)
	rm -rf $(OBJECT_DIR)/ii_files
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
-include dummy $(DEPENDENCIES)
