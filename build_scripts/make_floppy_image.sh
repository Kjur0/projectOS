#!/bin/bash

TARGET=$1

STAGE1_STAGE2_LOCATION_OFFSET=480

# generate image file
echo -e "\t>Generating floppy image ${TARGET}..."
dd if=/dev/zero of=$TARGET bs=512 count=2880 &>/dev/null

# determine how many reserved sectors
STAGE2_SIZE=$(stat -c%s ${BUILD_DIR}/stage2.bin)
STAGE2_SECTORS=$(((${STAGE2_SIZE} + 511) / 512))
RESERVED_SECTORS=$((1 + ${STAGE2_SECTORS}))

# create file system
echo -e "\t>Formatting..."
mkfs.fat -F 12 -R ${RESERVED_SECTORS} -n "projectOS" $TARGET &>/dev/null

# install bootloader
echo -e "\t>Installing bootloader..."
dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 count=3 &>/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 seek=62 skip=62 &>/dev/null
dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 &>/dev/null

# write lba address of stage2
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=${STAGE1_STAGE2_LOCATION_OFFSET} &>/dev/null
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$((${STAGE1_STAGE2_LOCATION_OFFSET} + 4)) &>/dev/null

# copy files
echo -e "\t>Copying files..."
mcopy -i $TARGET ${BUILD_DIR}/kernel.bin "::kernel.bin" &>/dev/null
mcopy -i $TARGET test.txt "::test.txt" &>/dev/null
mmd -i $TARGET "::mydir" &>/dev/null
mcopy -i $TARGET test.txt "::mydir/test.txt" &>/dev/null
