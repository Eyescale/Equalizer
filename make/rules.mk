
.PHONY: subdirs $(SUBDIRS) $(DEPENDENCIES) slib
.SUFFIXES: .d

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) -C $@


# headers
$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@echo 'Header file $@'
	@cp $< $@

# generated source code
%Dist.cpp %Packets.h : %.h $(TOP)/make/codegen.pl
	$(TOP)/make/codegen.pl $<

ifdef HEADER_GEN
  SOURCES_GEN  = $(HEADER_GEN:%.h=%Dist.cpp)
  SOURCES     += $(SOURCES_GEN)
endif

# libraries
dlib: $(DYNAMIC_LIB)
$(DYNAMIC_LIB): $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) $(INT_LDFLAGS) -o $@
else
	$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).$(DSO_SUFFIX)=%) \
		TOP=$(TOP) $@
endif

slib: $(STATIC_LIB)
$(STATIC_LIB): $(OBJECTS)
ifdef VARIANT
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) $(INT_LDFLAGS) -o $@
else
	$(MAKE) VARIANT=$(@:$(BUILD_DIR)/%/lib/libeq$(MODULE).a=%) TOP=$(TOP) $@
endif

OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(OBJECT_DIR)/%.o : %.cpp
	@mkdir -p $(OBJECT_DIR)
	@($(CXX_DEPS) $(CXX_DEPS_FLAGS) -M -E $< | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > \
		$(@D)/$*.d ) || rm $(@D)/$*.d
	$(CXX) $(CXXFLAGS) $(INT_CXXFLAGS) -c $< -o $@

%.cpp: $(OBJECT_DIR)/%d


# executables
% : %.cpp
	$(CXX) $< $(CXXFLAGS) $(INT_CXXFLAGS) $(SA_CXXFLAGS) -o $@ 

# cleaning targets
clean:
ifdef VARIANT
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)\
	    $(CLEAN) $(DEPENDENCIES)
	rm -rf $(OBJECT_DIR)/ii_files
ifdef SUBDIRS
	@for dir in $(SUBDIRS); do \
		echo "$(DEPTH) $$dir clean"; \
		$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) -C $$dir $@ ;\
	done
endif
else # VARIANT
	for variant in $(VARIANTS); do \
		$(MAKE) VARIANT=$$variant clean; \
	done
endif

# dependencies
-include dummy $(DEPENDENCIES)
