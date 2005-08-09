
.PHONY: subdirs $(SUBDIRS) $(DEPENDENCIES)
.SUFFIXES: .d

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBDIRTOP) -C $@


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
$(DYNAMIC_LIB): $(OBJECT_DIR) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(STATIC_LIB): $(OBJECT_DIR) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(OBJECT_DIR)/%.o : %.cpp
	@($(CXX_DEPS) $(CXX_DEPS_FLAGS) -M -E $< | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > \
		$(@D)/$*.d ) || rm $(@D)/$*.d
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.cpp: $(OBJECT_DIR)/%d

# executables
% : %.cpp
	$(CXX) $< $(CXXFLAGS) $(SA_CXXFLAGS) -o $@ 

# cleaning targets
clean:
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB) $(CLEAN) $(DEPENDENCIES)
	rm -rf $(OBJECT_DIR)/ii_files
ifdef SUBDIRS
	@for d in $(SUBDIRS); do \
		echo "$(DEPTH) $$d clean"; \
		$(MAKE) TOP=$(SUBDIRTOP) -C $$d $@ ;\
	done
endif

# dependencies
-include dummy $(DEPENDENCIES)
