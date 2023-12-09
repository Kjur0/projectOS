#!/bin/bash

if [[ "$EUID" != 0 ]]; then
    echo -e "\tThis script must be run as root!!!" >&2
    exit 1
fi

set -e

TARGET=$1
SIZE=$2

STAGE1_STAGE2_LOCATION_OFFSET=480

DISK_SECTOR_COUNT=$(((${SIZE} + 511) / 512))

DISK_PART1_BEGIN=2048
DISK_PART1_END=$((${DISK_SECTOR_COUNT} - 1))

# generate image file
echo -e "\t>Generating disk image ${TARGET} (${DISK_SECTOR_COUNT} sectors)..."
dd if=/dev/zero of=$TARGET bs=512 count=${DISK_SECTOR_COUNT} &>/dev/null

# create partition table
echo -e "\t>Creating partition table..."
parted -s $TARGET mklabel msdos &>/dev/null
parted -s $TARGET mkpart primary ${DISK_PART1_BEGIN}s ${DISK_PART1_END}s &>/dev/null
parted -s $TARGET set 1 boot on &>/dev/null

STAGE2_SIZE=$(stat -c%s ${BUILD_DIR}/stage2.bin)
STAGE2_SECTORS=$(((${STAGE2_SIZE} + 511) / 512))

if [ ${STAGE2_SECTORS} \> $((${DISK_PART1_BEGIN} - 1)) ]; then
    echo -e "\tStage2 too big!!!" >&2
    exit 2
fi

dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 &>/dev/null

# create loopback device
DEVICE=$(losetup -fP --show ${TARGET})
echo -e "\t>Created loopback device ${DEVICE}"
TARGET_PARTITION="${DEVICE}p1"

# create file system
echo -e "\t>Formatting ${TARGET_PARTITION}..."
mkfs.fat -n "projectOS" $TARGET_PARTITION &>/dev/null

# install bootloader
echo -e "\t>Installing bootloader on ${TARGET_PARTITION}..."
dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 count=3 &>/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITION conv=notrunc bs=1 seek=90 skip=90 &>/dev/null

# write lba address of stage2 to bootloader
echo "01 00 00 00" | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$STAGE1_STAGE2_LOCATION_OFFSET &>/dev/null
printf "%x" ${STAGE2_SECTORS} | xxd -r -p | dd of=$TARGET_PARTITION conv=notrunc bs=1 seek=$(($STAGE1_STAGE2_LOCATION_OFFSET + 4)) &>/dev/null

# copy files
echo -e "\t>Copying files to ${TARGET_PARTITION} (mounted on /tmp/projectOS)..."
mkdir -p /tmp/projectOS
mount ${TARGET_PARTITION} /tmp/projectOS
cp ${BUILD_DIR}/kernel.bin /tmp/projectOS
cp test.txt /tmp/projectOS
mkdir /tmp/projectOS/test
cp test.txt /tmp/projectOS/test
umount /tmp/projectOS

# destroy loopback device
losetup -d ${DEVICE}
