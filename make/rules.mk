
.PHONY: subdirs $(SUBDIRS) dummy
.SUFFIXES: .d

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBTOP) -C $@


# headers
$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@echo 'Header file $@'
	@cp $< $@

# libraries
$(DYNAMIC_LIB): $(OBJECT_DIR) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(DSO_LDFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(STATIC_LIB): $(OBJECT_DIR) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	@rm -f $@
	$(AR) $(ARFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(OBJECT_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECTS): $(DEPENDENCIES)

# cleaning targets
clean:
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(STATIC_LIB) $(DYNAMIC_LIB) $(CLEAN)
ifdef SUBDIRS
	@for d in $(SUBDIRS); do \
		echo "$(DEPTH) $$d clean"; \
		$(MAKE) TOP=$(SUBTOP) DEPENDENCIES=dummy -C $$d $@ ;\
	done
endif

# dependencies
OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(DEPENDENCIES): $(HEADER_SRC)

$(OBJECT_DIR)/%.d : %.cpp
	@mkdir -p $(OBJECT_DIR)
	@echo "Dependencies for $<"
	@($(CXX_DEPS) $(CXXFLAGS) -M -E $< | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > $@) || rm $@ dummy


-include dummy $(DEPENDENCIES)
