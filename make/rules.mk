
# recursive subdir rules

subdirs: $(SUBDIRS) 

$(SUBDIRS):
	$(MAKE) -C $@


# library generation rules

$(HEADER_DIR)/%.h : %.h
	@mkdir -p $(HEADER_DIR)
	@cp $< $@

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(LIBRARY_DIR)
	$(CXX) $(LDFLAGS) $(LIBFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@