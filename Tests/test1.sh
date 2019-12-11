#!/bin/bash
#
# Shell script definitions for testing the SSD driver
#
. include.sh

rm -f $RESULTS_FILE

#
# test for total zeroing
#
dd if=$ZERO_1MB_FILE of=$SSD_DEVICE
dd if=$SSD_DEVICE of=$CAPTURE_FILE
echo "Test of complete device zeroing" >> $RESULTS_FILE
diff $CAPTURE_FILE $ZERO_1MB_FILE > $RESULTS_FILE

###########################################################################################
#
# test for complete fill with pre-generated random data
#
dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE
dd if=$SSD_DEVICE of=$CAPTURE_FILE
echo "Test of complete random fill" >> $RESULTS_FILE
diff $CAPTURE_FILE $RANDOM_1MB_FILE >> $RESULTS_FILE

#
# zero array
#
dd if=$ZERO_1MB_FILE of=$SSD_DEVICE

#
# create 1 sector compare file from 1st 512 bytes of random file
#
dd if=$RANDOM_1MB_FILE of=$COMPARE_FILE bs=$SECTOR_SIZE count=1

#
# fill each sector with the same random block and compare
#
for ((sector=0;sector<SECTOR_LAST;sector++)); do
   dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE bs=$SECTOR_SIZE count=1 skip=0 seek=$((sector))
   dd if=$SSD_DEVICE of=$CAPTURE_FILE bs=$SECTOR_SIZE count=1 skip=$((sector)) 
   echo "Compare sector = $sector" >> $RESULTS_FILE
   diff $COMPARE_FILE $CAPTURE_FILE >> $RESULTS_FILE
done

#
# zero array
#
dd if=$ZERO_1MB_FILE of=$SSD_DEVICE

#
# fill entire array 1 byte at a time from pre-generated random data
#
for ((offset=0;offset<SSD_SIZE_IN_BYTES;offset++)); do
   dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE bs=1 count=1 skip=$((offset)) seek=$((offset))
done

echo "Test of individual byte access ( block size=1, count = $SSD_SIZE_IN_BYTES )" >> $RESULTS_FILE
dd if=$SSD_DEVICE of=$CAPTURE_FILE bs=$SECTOR_SIZE count=$SECTOR_COUNT skip=0
diff $CAPTURE_FILE $RANDOM_1MB_FILE >> $RESULTS_FILE

#
# zero array, create compare file
#
dd if=$ZERO_1MB_FILE of=$SSD_DEVICE
dd if=$RANDOM_1MB_FILE of=$COMPARE_FILE bs=$SECTOR_SIZE count=1

#
# offset 1/2 into each sector, fill with 512 bytes of the same random block, read back and compare
#
for ((sector=0;sector<(SECTOR_LAST-1);sector++)); do
   dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE bs=$SECTOR_SIZE count=1 skip=0 seek=$(((sector*SECTOR_SIZE) + 256))
   dd if=$SSD_DEVICE of=$CAPTURE_FILE bs=$SECTOR_SIZE count=1 skip=$(((sector*SECTOR_SIZE) + 256 )) seek=0
   echo "Compare with offset = 256, sector = $sector" >> $RESULTS_FILE
   diff $COMPARE_FILE $CAPTURE_FILE >> $RESULTS_FILE
done

