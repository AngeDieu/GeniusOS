$(BUILD_DIR)/test.js: EMSCRIPTEN_MODULARIZE = 0

HANDY_TARGETS += htmlpack
HANDY_TARGETS_EXTENSIONS += zip html

htmlpack_targets = .\

define rule_htmlpack
$$(BUILD_DIR)/htmlpack$(1)zip: $$(addprefix $$(BUILD_DIR)/ion/src/simulator/web/,calculator.html calculator.css) $$(BUILD_DIR)/epsilon$(1)js ion/src/simulator/web/calculator.js ion/src/simulator/assets/background-with-shadow.webp $(BUILD_DIR)/ion/src/simulator/assets/background.jpg $(BUILD_DIR)/ion/src/simulator/assets/background-no-shadow.webp
endef

$(foreach target,$(htmlpack_targets),$(eval $(call rule_htmlpack,$(target))))

$(BUILD_DIR)/epsilon%html: $(BUILD_DIR)/epsilon%js $(BUILD_DIR)/ion/src/simulator/web/simulator.html $(BUILD_DIR)/ion/src/simulator/assets/background.jpg
	@echo "INLINE  $@"
	$(Q) ion/src/simulator/web/inline.py --script $< --image ion/src/simulator/assets/background-with-shadow.webp --image $(BUILD_DIR)/ion/src/simulator/assets/background.jpg $(BUILD_DIR)/ion/src/simulator/web/simulator.html > $@
