
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

$(LIBRARY): $(OBJECT_DIR) $(DEPENDENCIES) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(LDFLAGS) $(LIBFLAGS) $(OBJECTS) -o $@

$(OBJECT_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECT_DIR):
	@mkdir -p $(OBJECT_DIR)

# cleaning targets
clean:
	rm -f *~ .*~ $(OBJECTS) $(DEPENDENCIES) $(HEADERS) $(LIBRARY) $(CLEAN)
	@for d in $(SUBDIRS); do \
		$(MAKE) TOP=$(SUBTOP) -C $$d $@; \
	done

# dependencies
$(OBJECT_DIR)/%.d : %.cpp
	@$(CXX) $(CXXFLAGS) -MD -E $< -o $@

ifdef $(DEPENDENCIES)
  include $(DEPENDENCIES)
endif
