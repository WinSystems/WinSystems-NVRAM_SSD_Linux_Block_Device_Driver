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
//	Name	 : ssdio.c
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
