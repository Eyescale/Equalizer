include $(TOP)/make/system.mk

.PHONY: $(CONFIGS)

CONFIG_SRC = \
	README \
	$(wildcard *.eqc) \
	$(wildcard TG-VizCluster/tvc-deskside/*.eqc)

TARGETS = $(CONFIGS)

CONFIGS = $(CONFIG_SRC:%=$(SHARE_DIR)/configs/%)

$(SHARE_DIR)/configs/%: %
	@mkdir -p $(@D)
	@echo 'Config file $@'
	@cp $< $@

include $(TOP)/make/rules.mk
