///****************************************************************************
//	
//	Copyright 2011 by WinSystems Inc.
//
//	Permission is hereby granted to the purchaser of WinSystems GPIO cards 
//	and CPU products incorporating a GPIO device, to distribute any binary 
//	file or files compiled using this source code directly or in any work 
//	derived by the user from this file. In no case may the source code, 
//	original or derived from this file, be distributed to any third party 
//	except by explicit permission of WinSystems. This file is distributed 
//	on an "As-is" basis and no warranty as to performance or fitness of pur-
//	poses is expressed or implied. In no case shall WinSystems be liable for 
//	any direct or indirect loss or damage, real or consequential resulting 
//	from the usage of this source code. It is the user's sole responsibility 
//	to determine fitness for any considered purpose.
//
///****************************************************************************
//
//	Name	 : ssd.h
//
//	Project	 : SSD Linux Device Driver
//
//	Author	 : Paul DeMetrotion
//
///****************************************************************************
//
//	  Date		Revision	                Description
//	--------	--------	---------------------------------------------
//	07/01/11	  1.0		  Original Release	
//
///****************************************************************************

#ifndef CHARDEV_H
  #define CHARDEV_H

#include <linux/ioctl.h>

#define SSD_SIZE_MB 1
#define SSD_MINORS 4
#define NSECTORS 2048
#define LOGICAL_BLOCK_SIZE 512
#define KERNEL_SECTOR_SIZE 512
#define IOCTL_NUM 'e'
#define SUCCESS 0

// enable write protect
#define IOCTL_EN_WP _IOWR(IOCTL_NUM, 1, int)

// disable write protect
#define IOCTL_DIS_WP _IOWR(IOCTL_NUM, 2, int)

// disable write protect
#define IOCTL_DMP_SECT _IOWR(IOCTL_NUM, 3, unsigned long)

#endif
