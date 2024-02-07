//*****************************************************************************
//
//   Copyright 2024, WinSystems Inc.
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
//    Author     : Paul DeMetrotion
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
//    2024/02/03      1.02      Updated for kernel v6.5
//
//*****************************************************************************

static char *RCSInfo = "$Id: ssd.c,v 1.02 2024/02/03 20:31:10 Paul DeMetrotion, Benjamin Herrera $";

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
// #include <linux/genhd.h>
// #include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/version.h>
#include <linux/blk-mq.h>
#include <asm/io.h>
#include "ssd.h"

#define DRVR_NAME		"ssd"
#define DRVR_VERSION	"1.02"
#define DRVR_RELDATE	"03Feb2024"

//#define DEBUG 1

// Function prototypes for local functions 

// ssd block device structure
static struct ssd_bdevice {
	int size;
	short users;
	spinlock_t lock;
	struct gendisk *gd;
	// request queue
	//struct request_queue *queue;
	struct blk_mq_tag_set tag_set;
	int wp_flag;
} ssd_bdev;

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

/* queue callback function */
static blk_status_t queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data* bd)
{
    unsigned int nr_bytes = 0;
    blk_status_t status = BLK_STS_OK;
    struct request *rq = bd->rq;

    /* Start request serving procedure */
    blk_mq_start_request(rq);

	unsigned long start = blk_rq_pos(rq) * LOGICAL_BLOCK_SIZE;
	unsigned long len = blk_rq_cur_bytes(rq) * LOGICAL_BLOCK_SIZE;

	printk("SSD: REQUEST: START %u, LEN %u", start, len);

	spin_lock_irq(&ssd_bdev->lock);

	ssd_transfer(&ssd_bdev, blk_rq_pos(rq), blk_rq_cur_sectors(rq), bio_data(req->bio), rq_data_dir(rq));

	spin_unlock_irq(&ssd_bdev->lock);
    /* Stop request serving procedure */
    blk_mq_end_request(rq, status);

    return status;
}

static struct blk_mq_ops mq_ops = {
    .queue_rq = queue_rq,
};

///**********************************************************************
//			DEVICE OPEN
///**********************************************************************
static int ssd_open(struct gendisk *disk, blk_mode_t  mode)
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
static void ssd_release(struct gendisk *disk)
{
	#ifdef DEBUG
	printk ("<1>SSD - ssd_release %d\n", ssd_bdev.users);
	#endif 

	ssd_bdev.users--;
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
static const struct block_device_operations ssd_fops = {
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
	printk("<1>Copyright 2024, All rights reserved\n");
	printk("<1>%s\n", RCSInfo);



	
	struct resource* rg = request_region(io, 8, DRVR_NAME);
	// check and map our I/O region requests
	if(!rg)
	{
		printk("<1>SSD - Unable to use I/O Address %4Xh\n", io);
		printk("<1>SSD - Return value %pa\n", rg);
		return -ENODEV;
	}
	else
	{
		printk("<1>SSD - Base I/O Address = %4Xh\n", io);
	}	
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
        //printk("Initializing queue\n");

        //ssd_bdev.queue = blk_mq_init_sq_queue(&ssd_bdev.tag_set, &mq_ops, 128, BLK_MQ_F_SHOULD_MERGE);

        //if (ssd_bdev.queue == NULL) {
        //    printk("Failed to allocate device queue\n");
        //    unregister_blkdev(ssd_major, DRVR_NAME);
        //    return -ENOMEM;
        //}
	//
	printk("Initializing tag set structure\n");
	ssd_bdev.tag_set.ops = &mq_ops;
	ssd_bdev.tag_set.nr_hw_queues = 1;
	ssd_bdev.tag_set.nr_maps = 1; //Revise Later
	ssd_bdev.tag_set.queue_depth = 16; //Depth of our data queue
	ssd_bdev.tag_set.numa_node = NUMA_NO_NODE;
	ssd_bdev.tag_set.flags = BLK_MQ_F_SHOULD_MERGE; 

        //Allocate tag set
        ret_val = blk_mq_alloc_tag_set(&ssd_bdev.tag_set);	

	if(ret_val)
	{
		blk_mq_free_tag_set(&ssd_bdev.tag_set);	    
		goto exit_bdev_unregister;
	}	
	

	// gendisk
	// NULL is queue data 
	ssd_bdev.gd = blk_mq_alloc_disk(&ssd_bdev.tag_set, NULL);

	if (IS_ERR(ssd_bdev.gd))
	{
		printk("<1>SSD - No memory resources for gendisk structure\n");
		ret_val = -ENOMEM;
		goto exit_bdev_unregister;
	}

	// initialize gendisk
	ssd_bdev.gd->major = ssd_major;
	ssd_bdev.gd->first_minor = 0;
	ssd_bdev.gd->minors = 1;
	ssd_bdev.gd->fops = &ssd_fops;
	ssd_bdev.gd->flags |=  GENHD_FL_NO_PART;
	//ssd_bdev.gd->queue = ssd_bdev.queue;
	ssd_bdev.gd->private_data = &ssd_bdev;
	strcpy(ssd_bdev.gd->disk_name, DRVR_NAME);
        printk("Adding disk %s\n", ssd_bdev.gd->disk_name);
	set_capacity(ssd_bdev.gd, NSECTORS * (LOGICAL_BLOCK_SIZE / KERNEL_SECTOR_SIZE));

	//blk_queue_max_hw_sectors(ssd_bdev.gd->queue,(NSECTORS));
	// initialize wp flag to off
	ssd_bdev.wp_flag = 1;

	// add disk at end of init
	ret_val = add_disk(ssd_bdev.gd);
	printk("Add disk complete");	
	if(ret_val)
	{
		printk("Error Adding Device: %d", ret_val);
		goto exit_bdev_unregister;
	}

	return SUCCESS;

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
	//blk_cleanup_queue(ssd_bdev.queue);
	blk_mq_free_tag_set(&ssd_bdev.tag_set);

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
MODULE_AUTHOR("Benjamin Herrera");
