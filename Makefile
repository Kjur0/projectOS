include build_scripts/config.mk

.PHONY: all floppy_image disk_image bootloader kernel tools_fat always clean

all: floppy_image disk_image tools_fat

include build_scripts/toolchain.mk


floppy_image: $(BUILD_DIR)/main_floppy.img

$(BUILD_DIR)/main_floppy.img: bootloader kernel
	@echo "<<< Creating: " $@
	@./build_scripts/make_floppy_image.sh $@
	@echo ">>> Created: " $@


disk_image: $(BUILD_DIR)/main_disk.raw

$(BUILD_DIR)/main_disk.raw: bootloader kernel
	@echo "<<< Creating: " $@
	@./build_scripts/make_disk_image.sh $@ $(MAKE_DISK_SIZE)
	@echo ">>> Created: " $@


bootloader: stage1 stage2

stage1: $(BUILD_DIR)/stage1.bin

$(BUILD_DIR)/stage1.bin: always
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR))

stage2: $(BUILD_DIR)/stage2.bin

$(BUILD_DIR)/stage2.bin: always
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR))


kernel: $(BUILD_DIR)/kernel.bin

$(BUILD_DIR)/kernel.bin: always
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR))


tools_fat: $(BUILD_DIR)/tools/fat
$(BUILD_DIR)/tools/fat: always tools/fat/fat.c
	@mkdir -p $(BUILD_DIR)/tools
	@$(MAKE) -C tools/fat BUILD_DIR=$(abspath $(BUILD_DIR))


always:
	@mkdir -p $(BUILD_DIR)


clean:
	@$(MAKE) -C src/bootloader/stage1 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/bootloader/stage2 BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@$(MAKE) -C src/kernel BUILD_DIR=$(abspath $(BUILD_DIR)) clean
	@rm -rf $(BUILD_DIR)/*