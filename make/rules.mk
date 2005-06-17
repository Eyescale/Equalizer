
.PHONY: subdirs $(SUBDIRS) dummy

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBTOP) -C $@


# headers
$(HEADERS):

$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@cp $< $@

# libraries
$(DYNAMIC_LIB): $(OBJECT_DIR) $(DEPENDENCIES) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(STATIC_LIB): $(OBJECT_DIR) $(DEPENDENCIES) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) -o $@

$(OBJECT_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECT_DIR):
	@mkdir -p $(OBJECT_DIR)

# cleaning targets
clean:
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB) $(CLEAN)
ifdef SUBDIRS
	@for d in $(SUBDIRS); do \
		echo "$(DEPTH) $$d clean"; \
		$(MAKE) TOP=$(SUBTOP) -C $$d $@ ;\
	done
endif

# dependencies
OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(DEPENDENCIES): $(HEADERS)

$(OBJECT_DIR)/%.d : %.cpp
	@echo "Updating dependencies for $<"
	@($(CXX_DEPS) $(CXXFLAGS) -MM -E $<  | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > $@) || rm $@


-include dummy $(DEPENDENCIES)
