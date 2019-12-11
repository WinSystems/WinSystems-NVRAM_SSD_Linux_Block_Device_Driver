#!/bin/bash
#
# Shell script definitions for testing the SSD driver
#
. include.sh

rm -f $RESULTS_ODD_FILE

dd if=$RANDOM_1MB_FILE of=$COMPARE_ODD_FILE bs=$SECTOR_SIZE count=1

#
# fill each sector with the same random block and compare
#
for ((sector=0;sector<1023;sector++)); do
   dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE bs=$SECTOR_SIZE count=1 skip=0 seek=$((sector))
   dd if=$SSD_DEVICE of=$CAPTURE_ODD_FILE bs=$SECTOR_SIZE count=1 skip=$((sector)) 
   echo "Compare sector = $sector" >> $RESULTS_ODD_FILE
   diff $COMPARE_ODD_FILE $CAPTURE_ODD_FILE >> $RESULTS_ODD_FILE
done
