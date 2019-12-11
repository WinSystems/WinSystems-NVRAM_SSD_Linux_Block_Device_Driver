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
//    Name       : ssd.c
//
//    Project    : NVRAM SSD Device Driver
//
//    Author     : Jack Smith
//
//    Description:
//             This module contains the implementation of the block device
//             driver for the NV-RAM SSD
//          
//
//*****************************************************************************
//
//      Date         Revision    Change Description
//    --------       --------    ---------------------------------------------
//    2019/12/11      1.01      Updated for kernel v4.18
//
//*****************************************************************************

static char *RCSInfo = "$Id: ssd.c,v 1.1 2019/11/28 20:31:10 tjsmith Exp tjsmith $";

#ifndef __KERNEL__
	#define __KERNEL__
#endif

#ifndef MODULE
	#define MODULE
#endif

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/version.h>
#include <asm/io.h>
#include "ssd.h"

#define DRVR_NAME		"ssd"
#define DRVR_VERSION	"1.0"
#define DRVR_RELDATE	"29Jun2011"

//#define DEBUG 1

// Function prototypes for local functions 

// ssd block device structure
static struct ssd_bdevice {
	int size;
	short users;
	spinlock_t lock;
	struct gendisk *gd;
	int wp_flag;
} ssd_bdev;

// request queue
static struct request_queue *queue;

// Driver major number
static int ssd_init_major = 0;	// 0 = allocate dynamically
static int ssd_major;

// Our modprobe command line arguments
static int io = 0;

///**********************************************************************
//			DEVICE SUBROUTINES
///**********************************************************************
static void ssd_read(unsigned long offset, char *buffer)
{
	int i, ptr;

	#ifdef DEBUG
	printk ("<1>SSD - ssd_read\n");
	printk ("<1> Starting Address = 0x%06lX\n", offset);
	#endif
		
	for (i = 0; i < 2; i++)
	{
		outb(offset >> 16, io);
		outb(offset >> 8, io + 1);

		for (ptr = 0; ptr < 256; ptr++)
			*buffer++ = inb(io + 2);

		offset += 256;
	}

	#ifdef DEBUG
	printk ("<1>SSD - ssd_read complete\n");
	#endif
}

static void ssd_write(unsigned long offset, char *buffer)
{
	int i, ptr;

	#ifdef DEBUG
	printk ("<1>SSD - ssd_write\n");
	printk ("<1> Starting Address = 0x%06lX\n", offset);
	#endif
		
	for (i = 0; i < 2; i++)
	{
		outb(offset >> 16, io);
		outb(offset >> 8, io + 1);

		for (ptr = 0; ptr < 256; ptr++)
			outb(*buffer++, io + 2);

		offset += 256;
	}

	#ifdef DEBUG
	printk ("<1>SSD - ssd_write complete\n");
	#endif
}

static void ssd_transfer(struct ssd_bdevice *bdev, sector_t sector, unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector * LOGICAL_BLOCK_SIZE;
	unsigned long nbytes = nsect * LOGICAL_BLOCK_SIZE;

	if ((offset + nbytes) > bdev->size)
	{
		#ifdef DEBUG
		printk ("<1>SSD - Beyond-end write (0x%06lX %ld)\n", offset, nbytes);
		#endif
		
		return;
	}
	
	if (write && !(bdev->wp_flag))
	{
		while (nsect--)
		{
			ssd_write(offset, buffer);
			offset += LOGICAL_BLOCK_SIZE;
			buffer += LOGICAL_BLOCK_SIZE;
		}
	}
	else if (write && bdev->wp_flag)
	{
		printk ("<1>SSD - Write Protect mode is enabled. To disable run the lock program.\n");
		
		#ifdef DEBUG
		printk ("<1>SSD - Protected write (0x%06lX %ld)\n", offset, nbytes);
		#endif
	}
	else
	{
		while (nsect--)
		{
			ssd_read(offset, buffer);
			offset += LOGICAL_BLOCK_SIZE;
			buffer += LOGICAL_BLOCK_SIZE;
		}
	}
}

static void ssd_request(struct request_queue *q)
{
	struct request *req;

	req = blk_fetch_request(q);

	while (req != NULL)
	{
		char *whereto;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,10,0)
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS))
#else
		if(req == NULL) 
#endif
		{
			#ifdef DEBUG
			printk ("<1>SSD - Skip non-CMD request\n");
			#endif
			__blk_end_request_all(req, -EIO);
			continue;
		}
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,10,0)
		whereto = req->buffer;
#else
		whereto = bio_data(req->bio);
#endif
		ssd_transfer(&ssd_bdev, blk_rq_pos(req), blk_rq_cur_sectors(req), whereto, rq_data_dir(req));
 
		if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
	}
}

///**********************************************************************
//			DEVICE OPEN
///**********************************************************************
static int ssd_open(struct block_device *bdev, fmode_t mode)
{
	ssd_bdev.users++;

	#ifdef DEBUG
	printk ("<1>SSD - ssd_open %d\n", ssd_bdev.users);
	#endif

	return SUCCESS;
}

///**********************************************************************
//			DEVICE CLOSE
///**********************************************************************
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,10,0)
static int ssd_release(struct gendisk *gd, fmode_t mode)
#else
static void ssd_release(struct gendisk *gd, fmode_t mode)
#endif

{
	#ifdef DEBUG
	printk ("<1>SSD - ssd_release %d\n", ssd_bdev.users);
	#endif 

	ssd_bdev.users--;


#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,10,0)
	return SUCCESS;
#endif
}

///**********************************************************************
//			DEVICE IOCTL
///**********************************************************************
int ssd_ioctl(struct block_device *bdev, fmode_t mode, unsigned int ioctl_num, unsigned long ioctl_param)
{
	#ifdef DEBUG
	int i;

	printk("<1>SSD - ssd_ioctl(%d)\n", ioctl_num);
	#endif

	// Switch according to the ioctl called
	switch (ioctl_num) {
		case IOCTL_EN_WP:
			outb(0, io + 4);	// enable write protect
			ssd_bdev.wp_flag = 1;			// structure flag
			return SUCCESS;

		case IOCTL_DIS_WP:
			outb(1, io + 4);	// disable write protect
			ssd_bdev.wp_flag = 0;		// structure flag
			return SUCCESS;

		#ifdef DEBUG
		case IOCTL_DMP_SECT:
			// set address
			outb(ioctl_param >> 16, io);
			outb(ioctl_param >> 8, io + 1);

			// read & display sector
			for (i=0; i < 512; i++)
			{
				if (i % 16 == 0) printk("%05lX : ", ioctl_param + i);
				printk("%02X ", inb(io + 2));
				if (i % 16 == 15) printk("\n");
			}
	
			return SUCCESS;
		#endif

		// Catch all return
		default:
			return(-EINVAL);
	}
}

///**********************************************************************
//			DEVICE GETGEO
///**********************************************************************
int ssd_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	long size;

	#ifdef DEBUG
	printk ("<1>SSD - ssd_getgeo\n");
	#endif

	/* We have no real geometry, of course, so make something up. */
	size = ssd_bdev.size * (LOGICAL_BLOCK_SIZE / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
	
	return SUCCESS;
}

///**********************************************************************
//			Module Declarations
// This structure will hold the functions to be called 
// when a process does something to the device
///**********************************************************************
static struct block_device_operations ssd_fops = {
	.open			=	ssd_open,
	.release		=	ssd_release,
	.ioctl			=	ssd_ioctl,
	.getgeo			=	ssd_getgeo,		// need for fdisk support
	.owner			=	THIS_MODULE
};

///**********************************************************************
//			MODULE INITIALIZATION
///**********************************************************************
static int __init ssd_init(void)
{
	int ret_val;

	// Sign-on
	printk("<1>WinSystems, Inc. SSD Linux Device Driver\n");
	printk("<1>Copyright 2011, All rights reserved\n");
	printk("<1>%s\n", RCSInfo);

	// check and map our I/O region requests
	if(request_region(io, 8, DRVR_NAME) == NULL)
	{
		printk("<1>SSD - Unable to use I/O Address %4Xh\n", io);
		return -ENODEV;
	}
	else
		printk("<1>SSD - Base I/O Address = %4Xh\n", io);
		
	// register the block device
	ssd_major = register_blkdev(ssd_init_major, DRVR_NAME);

	if(ssd_major < 0)
	{
		printk("<1>SSD - Cannot obtain major number\n");
		ret_val = ssd_major;
		goto exit_release_io;
	}
	else
	{
		if (ssd_init_major)
			ssd_major = ssd_init_major;

		printk("<1>SSD - Major number %d assigned\n", ssd_major);
	}

	// initialize structure
	memset(&ssd_bdev, 0, sizeof(struct ssd_bdevice));
	ssd_bdev.size = NSECTORS * LOGICAL_BLOCK_SIZE;

	// spin lock
	spin_lock_init(&ssd_bdev.lock);

	// request queue
	queue = blk_init_queue(ssd_request, &ssd_bdev.lock);

	if (queue == NULL)
	{
		printk("<1>SSD - No memory resources for request queue\n");
		ret_val = -ENOMEM;
		goto exit_bdev_unregister;
	}

	// gendisk
	ssd_bdev.gd = alloc_disk(SSD_MINORS);

	if (ssd_bdev.gd == NULL)
	{
		printk("<1>SSD - No memory resources for gendisk structure\n");
		ret_val = -ENOMEM;
		goto exit_reqq_delete;
	}

	// initialize gendisk
	ssd_bdev.gd->major = ssd_major;
	ssd_bdev.gd->first_minor = 0;
	ssd_bdev.gd->fops = &ssd_fops;
	ssd_bdev.gd->queue = queue;
	ssd_bdev.gd->private_data = &ssd_bdev;
	strcpy(ssd_bdev.gd->disk_name, DRVR_NAME);
	set_capacity(ssd_bdev.gd, NSECTORS * (LOGICAL_BLOCK_SIZE / KERNEL_SECTOR_SIZE));

	// initialize wp flag to off
	ssd_bdev.wp_flag = 1;

	// add disk at end of init
	add_disk(ssd_bdev.gd);

	return SUCCESS;

exit_reqq_delete:
	blk_cleanup_queue(queue);

exit_bdev_unregister:
	unregister_blkdev(ssd_major, DRVR_NAME);
	ssd_major = 0;

exit_release_io:
	release_region(io, 8);

	return ret_val;
}

///**********************************************************************
//			MODULE EXIT
///**********************************************************************
static void ssd_exit(void)
{
	// delete gendisk
	del_gendisk(ssd_bdev.gd);

	// delete request queue
	blk_cleanup_queue(queue);

	// unregister the device
	unregister_blkdev(ssd_major, DRVR_NAME);
	ssd_major = 0;

	// release io address range
	release_region(io, 8);
}  

module_param(io, int, S_IRUGO);

module_init(ssd_init);
module_exit(ssd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("WinSystems,Inc. SSD Device Driver");
MODULE_AUTHOR("Paul DeMetrotion");
