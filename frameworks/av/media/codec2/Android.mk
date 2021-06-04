# =============================================================================
# DOCUMENTATION GENERATION
# =============================================================================
C2_ROOT := $(call my-dir)

C2_DOCS_ROOT := $(OUT_DIR)/target/common/docs/codec2

C2_OUT_TEMP := $(PRODUCT_OUT)/gen/ETC/Codec2-docs_intermediates

C2_DOXY := $(or $(shell command -v doxygen),\
		$(shell command -v /Applications/Doxygen.app/Contents/Resources/doxygen))

.PHONY: check-doxygen
check-doxygen:
ifndef C2_DOXY
	$(error 'doxygen is not available')
endif

$(C2_OUT_TEMP)/doxy-api.config: $(C2_ROOT)/docs/doxygen.config
	# only document include directory, no internal sections
	sed 's/\(^INPUT *=.*\)/\1include\//; \
	s/\(^INTERNAL_DOCS *= *\).*/\1NO/; \
	s/\(^ENABLED_SECTIONS *=.*\)INTERNAL\(.*\).*/\1\2/; \
	s:\(^OUTPUT_DIRECTORY *= \)out:\1'$(OUT_DIR)':;' \
		$(C2_ROOT)/docs/doxygen.config > $@

$(C2_OUT_TEMP)/doxy-internal.config: $(C2_ROOT)/docs/doxygen.config
	sed 's:\(^OUTPUT_DIRECTORY *= \)out\(.*\)api:\1'$(OUT_DIR)'\2internal:;' \
		$(C2_ROOT)/docs/doxygen.config > $@

.PHONY: docs-api
docs-api: $(C2_OUT_TEMP)/doxy-api.config check-doxygen
	echo API docs are building in $(C2_DOCS_ROOT)/api
	rm -rf $(C2_DOCS_ROOT)/api
	mkdir -p $(C2_DOCS_ROOT)/api
	$(C2_DOXY) $(C2_OUT_TEMP)/doxy-api.config

.PHONY: docs-internal
docs-internal: $(C2_OUT_TEMP)/doxy-internal.config check-doxygen
	echo Internal docs are building in $(C2_DOCS_ROOT)/internal
	rm -rf $(C2_DOCS_ROOT)/internal
	mkdir -p $(C2_DOCS_ROOT)/internal
	$(C2_DOXY) $(C2_OUT_TEMP)/doxy-internal.config

.PHONY: docs-all
docs-all: docs-api docs-internal

include $(call all-makefiles-under,$(call my-dir))
