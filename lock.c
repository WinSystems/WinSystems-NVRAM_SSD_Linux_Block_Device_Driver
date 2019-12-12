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
//    Name       : lock.c
//
//    Project    : NVRAM SSD Device Driver
//
//    Author     : Paul DeMetrotion
//
//    Description:
//             This module contains the implementation lock/unlock application
//             for the block device driver for the NV-RAM SSD
//
//*****************************************************************************
//
//      Date         Revision    Change Description
//    --------       --------    ---------------------------------------------
//    2019/12/11      1.01      Updated for kernel v4.18
//
//*****************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// Include the WinSystems UIO48 definitions
#include "ssd.h"

int main(int argc, char *argv[])
{
	// arguments for on or off
	if (argc != 2)
	{
		printf("Usage: lock <on/off>\n");
		exit(1);
	}

	if (argv[1][1] == 0x6E)	// 'n'
		if(ssd_set_wp() < 0) 
		{
			fprintf(stderr, "Can't access device SSD - Aborting\n");
			exit(1);
		}
		else
			printf("SSD write protect enabled\n");
	else
		if(ssd_clr_wp() < 0)
		{
			fprintf(stderr, "Can't access device SSD - Aborting\n");
			exit(1);
		}
		else
			printf("SSD write protect disabled\n");

	return SUCCESS;
}
