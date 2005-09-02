
.PHONY: subdirs $(SUBDIRS) $(DEPENDENCIES) $(VARIANTS) slib
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
$(DYNAMIC_LIB): $(VARIANTS) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

slib: $(STATIC_LIB)

$(STATIC_LIB): $(VARIANTS) $(OBJECTS)
ifneq ($(OBJECTS), none)
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@
endif

OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(OBJECT_DIR)/%.o : %.cpp
	@mkdir -p $(OBJECT_DIR)
	@($(CXX_DEPS) $(CXX_DEPS_FLAGS) -M -E $< | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > \
		$(@D)/$*.d ) || rm $(@D)/$*.d
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.cpp: $(OBJECT_DIR)/%d


$(VARIANTS):
ifndef VARIANT
	@for variant in $(VARIANTS); do \
		echo; echo "===== $(MAKE) VARIANT=$$variant slib =====";  \
		$(MAKE) VARIANT=$$variant slib;  \
	done
endif

# executables
% : %.cpp
	$(CXX) $< $(CXXFLAGS) $(SA_CXXFLAGS) -o $@ 

# cleaning targets
clean:
ifndef VARIANT
	for variant in $(VARIANTS); do \
		$(MAKE) VARIANT=$$variant clean; \
	done
else
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB)\
	    $(CLEAN) $(DEPENDENCIES)
	rm -rf $(OBJECT_DIR)/ii_files
ifdef SUBDIRS
	@for dir in $(SUBDIRS); do \
		echo "$(DEPTH) $$dir clean"; \
		$(MAKE) TOP=$(SUBDIRTOP) VARIANT=$(VARIANT) -C $$dir $@ ;\
	done
endif
endif

# dependencies
-include dummy $(DEPENDENCIES)
