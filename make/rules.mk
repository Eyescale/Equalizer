
.PHONY: subdirs $(SUBDIRS)

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "$(DEPTH) $@"
	@$(MAKE) TOP=$(SUBTOP) -C $@


# library generation rules

$(HEADERS):

$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@cp $< $@

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
	rm -f *~ .*~ $(OBJECTS) $(DEPENDENCIES) $(HEADERS) $(STATIC_LIB) \
		$(DYNAMIC_LIB) $(CLEAN)
ifdef SUBDIRS
	@for d in $(SUBDIRS); do \
		$(MAKE) TOP=$(SUBTOP) -C $$d $@ ;\
	done
endif

# dependencies
OBJECT_DIR_ESCAPED = $(subst /,\/,$(OBJECT_DIR))

$(OBJECT_DIR)/%.d : %.cpp
	@echo "Updating dependencies for $<"
	($(CXX_DEPS) $(CXXFLAGS) -MM -E $<  | \
		sed 's/\(.*:\)/$(OBJECT_DIR_ESCAPED)\/\1/' > $@) || rm $@


ifdef $(DEPENDENCIES)
  include $(DEPENDENCIES)
endif
