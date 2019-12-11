//*****************************************************************************
//
//   Copyright 2019, WinSystems Inc.
//
//   Permission is hereby granted, free of charge, to any person obtaining a 
//   copy of this software and associated documentation files (the "Software"), 
//   to deal in the Software without restriction, including without limitation 
//   the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//   and/or sell copies of the Software, and to permit persons to whom the 
//   Software is furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included 
//   in all copies or substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
//   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
//   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
//   DEALINGS IN THE SOFTWARE.
//
//*****************************************************************************
//
//    Name       : ssd.h
//
//    Project    : NV-RAM SSD For WinSystems platforms
//
//    Author     : Jack Smith
//
//    Description:
//             This module contains the definitions for the NV-RAM SSD
//             block device driver.
//          
//
//*****************************************************************************
//
//      Date         Revision    Change Description
//    --------       --------    ---------------------------------------------
//    2019/12/11       1.01       Modified to support kernel 4.18
//
//*****************************************************************************
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
