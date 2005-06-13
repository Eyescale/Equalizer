
.PHONY: subdirs $(SUBDIRS)

# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	@echo "----- $@"
	@$(MAKE) TOP=$(SUBTOP) -C $@


# library generation rules

$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@cp $< $@

$(LIBRARY): $(OBJECT_DIR) $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(LDFLAGS) $(LIBFLAGS) $(OBJECTS) -o $@

$(OBJECT_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJECT_DIR):
	@mkdir -p $(OBJECT_DIR)

clean:
	rm -f *~ .*~ $(OBJECTS) $(HEADERS) $(LIBRARY) $(CLEAN)
	@for d in $(SUBDIRS); do \
		$(MAKE) TOP=$(SUBTOP) -C $$d $@; \
	done
