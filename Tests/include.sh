#!/bin/bash
#
# Shell script definitions for testing the SSD driver
#

RESULTS_FILE=results.txt
RESULTS_ODD_FILE=results_odd.txt
RESULTS_EVEN_FILE=results_even.txt

SSD_DEVICE=/dev/ssd
ZERO_DEVICE=/dev/zero

RANDOM_1MB_FILE=random_1MB.raw
RANDOM_4096_FILE=random_4096.raw

ZERO_1MB_FILE=zero_1MB.raw
ZERO_2048_FILE=zero_2048.raw
ZERO_512_FILE=zero_512.raw

CAPTURE_FILE=capture.raw
CAPTURE_ODD_FILE=capture_odd.raw
CAPTURE_EVEN_FILE=capture_even.raw

COMPARE_FILE=compare.raw
COMPARE_ODD_FILE=compare_odd.raw
COMPARE_EVEN_FILE=compare_even.raw

SSD_SIZE_IN_BYTES=1048576
SECTOR_SIZE=512

SECTOR_OFFSET_MAX=511
SECTOR_COUNT=2048
SECTOR_LAST=2047

BYTE_SEQ_FILL_END=255
