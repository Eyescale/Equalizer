include $(TOP)/make/system.mk

.PHONY: $(CONFIGS)

CONFIG_SRC = \
	$(wildcard *.eqc) \
	$(wildcard TG-VizCluster/tvc-deskside/*.eqc)

TARGETS = $(CONFIGS)

CONFIGS = $(CONFIG_SRC:%=$(SHARE_DIR)/configs/%)

$(SHARE_DIR)/configs/%: %
	@mkdir -p $(@D)
	@echo 'Config file $@'
	@cp $< $@
	@echo "mkdir -p \$$DESTDIR/$(dir $(INSTALL_SHARE_DIR)/configs/$<);" \
	      "install -c $(SUBDIR)/$@ \$$DESTDIR/$(INSTALL_SHARE_DIR)/configs/$<" \
	    >> $(INSTALL_CMD)

include $(TOP)/make/rules.mk
