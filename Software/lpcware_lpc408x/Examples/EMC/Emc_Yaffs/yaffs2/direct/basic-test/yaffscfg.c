/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2010 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * yaffscfg.c  The configuration for the "direct" use of yaffs.
 *
 * This file is intended to be modified to your requirements.
 * There is no need to redistribute this file.
 */

#include "yaffscfg.h"
#include "yaffsfs.h"
#include <errno.h>


#include "yramsim.h"

unsigned yaffs_traceMask = 0xFFFFFFFF;


void yaffsfs_SetError(int err)
{
	//Do whatever to set error
	errno = err;
}

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

__u32 yaffsfs_CurrentTime(void)
{
	return 0;
}

void *yaffs_malloc(size_t size)
{
	return malloc(size);
}

void yaffs_free(void *ptr)
{
	free(ptr);
}

void yaffsfs_LocalInitialisation(void)
{
	// Define locking semaphore.
}

// Configuration for:
// /ram  2MB ramdisk
// /boot 2MB boot disk (flash)
// /flash 14MB flash disk (flash)
// NB Though /boot and /flash occupy the same physical device they
// are still disticnt "yaffs_Devices. You may think of these as "partitions"
// using non-overlapping areas in the same device.
// 

#include "yaffs_ramdisk.h"
#include "yaffs_flashif.h"

static yaffs_Device ramDev;
static yaffs_Device bootDev;
static yaffs_Device flashDev;

static yaffsfs_DeviceConfiguration yaffsfs_config[] = {

	{ "/ram", &ramDev},
	{ "/boot", &bootDev},
	{ "/flash", &flashDev},
	{(void *)0,(void *)0}
};


int yaffs_StartUp(void)
{
	// Stuff to configure YAFFS
	// Stuff to initialise anything special (eg lock semaphore).
	yaffsfs_LocalInitialisation();
	
	// Set up devices
    memset(&ramDev.param, 0, sizeof(ramDev.param));
    memset(&bootDev.param, 0, sizeof(bootDev.param));
    memset(&flashDev.param, 0, sizeof(flashDev.param));
    
	// /ram
	ramDev.param.totalBytesPerChunk = 512;
	ramDev.param.nChunksPerBlock = 32;
	ramDev.param.nReservedBlocks = 2; // Set this smaller for RAM
	ramDev.param.startBlock = 1; // Can't use block 0
	ramDev.param.endBlock = 127; // Last block in 2MB.	
	ramDev.param.useNANDECC = 1;
	ramDev.param.nShortOpCaches = 0;	// Disable caching on this device.
	ramDev.genericDevice = (void *) 0;	// Used to identify the device in fstat.
	ramDev.param.writeChunkWithTagsToNAND = yramdisk_WriteChunkWithTagsToNAND;
	ramDev.param.readChunkWithTagsFromNAND = yramdisk_ReadChunkWithTagsFromNAND;
	ramDev.param.eraseBlockInNAND = yramdisk_EraseBlockInNAND;
	ramDev.param.initialiseNAND = yramdisk_InitialiseNAND;

	// /boot
	bootDev.param.totalBytesPerChunk = 512;
	bootDev.param.nChunksPerBlock = 32;
	bootDev.param.nReservedBlocks = 5;
	bootDev.param.startBlock = 1; // Can't use block 0
	bootDev.param.endBlock = 127; // Last block in 2MB.	
	bootDev.param.useNANDECC = 0; // use YAFFS's ECC
	bootDev.param.nShortOpCaches = 10; // Use caches
	bootDev.genericDevice = (void *) 1;	// Used to identify the device in fstat.
	bootDev.param.writeChunkToNAND = yflash_WriteChunkToNAND;
	bootDev.param.readChunkFromNAND = yflash_ReadChunkFromNAND;
	bootDev.param.eraseBlockInNAND = yflash_EraseBlockInNAND;
	bootDev.param.initialiseNAND = yflash_InitialiseNAND;

		// /flash
	flashDev.param.totalBytesPerChunk =  512;
	flashDev.param.nChunksPerBlock = 32;
	flashDev.param.nReservedBlocks = 5;
	flashDev.param.startBlock = 128; // First block after 2MB
	flashDev.param.endBlock = 1023; // Last block in 16MB
	flashDev.param.useNANDECC = 0; // use YAFFS's ECC
	flashDev.param.nShortOpCaches = 10; // Use caches
	flashDev.genericDevice = (void *) 2;	// Used to identify the device in fstat.
	flashDev.param.writeChunkToNAND = yflash_WriteChunkToNAND;
	flashDev.param.readChunkFromNAND = yflash_ReadChunkFromNAND;
	flashDev.param.eraseBlockInNAND = yflash_EraseBlockInNAND;
	flashDev.param.initialiseNAND = yflash_InitialiseNAND;

	yaffs_initialise(yaffsfs_config);

	
	return 0;
}




