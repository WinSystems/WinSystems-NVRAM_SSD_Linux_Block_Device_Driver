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
//    Name       : ssdio.c
//
//    Project    : NVRAM SSD library
//
//    Author     : Paul DeMetrotion
//
//    Description:
//             This module contains the implementation of the block device
//             driver library for the NV-RAM SSD
//          
//
//*****************************************************************************
//
//      Date         Revision    Change Description
//    --------       --------    ---------------------------------------------
//    2019/12/11      1.01       Updated for kernel v4.18
//
//*****************************************************************************

#include <stdio.h>
#include <fcntl.h>      /* open */ 
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */

#include "ssd.h"    

//**************************************************************************
//		USER LIBRARY FUNCTIONS
//**************************************************************************

// device handles
int handle = 0;

// the names of our device nodes
char *device_id = "/dev/ssd";

//
//------------------------------------------------------------------------
//
// ssd_set_wp - enables write protect
//
// Arguments:
//
// Returns:
//
//------------------------------------------------------------------------
//
int ssd_set_wp(void)
{
    if(check_handle())   // check for chip availability
		return -1;

	return ioctl(handle, IOCTL_EN_WP, 0);
}

//
//------------------------------------------------------------------------
//
// ssd_clr_wp - enables write protect
//
// Arguments:
//
// Returns:
//
//------------------------------------------------------------------------
//
int ssd_clr_wp(void)
{
    if(check_handle())   // check for chip availability
		return -1;

	return ioctl(handle, IOCTL_DIS_WP, 0);
}

//
//------------------------------------------------------------------------
//
// ssd_set_wp - enables write protect
//
// Arguments:
//
// Returns:
//
//------------------------------------------------------------------------
//
int ssd_dmp_sect(int sect)
{
    if(check_handle())   // check for chip availability
		return -1;

	return ioctl(handle, IOCTL_DMP_SECT, sect * 512);
}

//
//------------------------------------------------------------------------
//
// check_handle - Checks that a handle to the device file exists.
//				  If it does not a file open is performed.
//
// Arguments:
//
// Returns:
//			0		Handle is valid
//			-1		Chip does not exist or it's handle is invalid
//
//------------------------------------------------------------------------
//
int check_handle(void)
{
    if(handle > 0)	// If it's already a valid handle
		return 0;

    if(handle == -1)	// If it's already been tried
		return -1;

	// Try opening the device file, in case it hasn't been opened yet
    handle = open(device_id, O_RDWR);

    if(handle > 0)	// If it's now a validopen handle
		return 0;
    
    handle = -1;
		return -1;
}
