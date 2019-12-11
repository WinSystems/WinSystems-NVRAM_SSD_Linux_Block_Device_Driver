//*****************************************************************************
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
//*****************************************************************************
//
//	Name	 : lock.c
//
//	Project	 : SSD Write Protect Control Program
//
//	Author	 : Paul DeMetrotion
//
//*****************************************************************************
//
//	  Date		Revision	                Description
//	--------	--------	---------------------------------------------
//	07/01/11	  1.0		  Original Release	
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
