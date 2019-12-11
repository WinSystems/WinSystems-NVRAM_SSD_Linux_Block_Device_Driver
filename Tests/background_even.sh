#!/bin/bash
#
# Shell script definitions for testing the SSD driver
#
. include.sh

rm -f $RESULTS_EVEN_FILE

dd if=$RANDOM_1MB_FILE of=$COMPARE_EVEN_FILE bs=$SECTOR_SIZE count=1

#
# fill each sector with the same random block and compare
#
for ((sector=1024;sector<SECTOR_LAST;sector++)); do
   dd if=$RANDOM_1MB_FILE of=$SSD_DEVICE bs=$SECTOR_SIZE count=1 skip=0 seek=$((sector))
   dd if=$SSD_DEVICE of=$CAPTURE_EVEN_FILE bs=$SECTOR_SIZE count=1 skip=$((sector)) 
   echo "Compare sector = $sector" >> $RESULTS_EVEN_FILE
   diff $COMPARE_EVEN_FILE $CAPTURE_EVEN_FILE >> $RESULTS_EVEN_FILE
done
