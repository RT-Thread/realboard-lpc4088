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
 * NAND Simulator for testing YAFFS
 */

#include <string.h>
#include "ynandsim.h"
#include "yaffs_nandif.h"
#include "bsp.h"
#include "nandflash_k9f1g08u0a.h"



#define DATA_SIZE	NANDFLASH_RW_PAGE_SIZE
#define SPARE_SIZE	NANDFLASH_SPARE_SIZE
#define PAGE_SIZE	(DATA_SIZE + SPARE_SIZE)
#define PAGES_PER_BLOCK	NANDFLASH_PAGE_PER_BLOCK

unsigned char tmp_page[PAGE_SIZE];

typedef struct {
	//unsigned char * page[PAGES_PER_BLOCK][PAGE_SIZE];
	unsigned int blockNum;
	unsigned char blockOk;
} Block;

typedef struct {
	Block **blockList;
	int nBlocks;
} SimData;


SimData *simDevs[N_RAM_SIM_DEVS];

static SimData *DevToSim(yaffs_Device *dev)
{
	ynandif_Geometry *geom = (ynandif_Geometry *)(dev->driverContext);
	SimData * sim = (SimData*)(geom->privateData);
	return sim;
}


static void CheckInitialised(void)
{

}

static void ynandsim_InitDevice(void)
{
    static char init_flg = 0;
    uint32_t FlashID;
    if(init_flg == 0)
    {
        NandFlash_Init();
        NandFlash_Reset();
        FlashID = NandFlash_ReadId();
        if ( (FlashID & 0xFFFF0000) != K9FXX_ID )
            return;
        NandFlash_ValidBlockCheck();
        init_flg = 1;
    }
}

static int ynandsim_EraseBlockInternal(SimData *sim, unsigned blockId,int force)
{
	if(blockId < 0 || blockId >= sim->nBlocks){
		return 0;
	}

	if(!sim->blockList[blockId]){
		return 0;
	}

	if(!force && !sim->blockList[blockId]->blockOk){
		return 0;
	}

	if(NandFlash_BlockErase(sim->blockList[blockId]->blockNum) == FALSE)
        return 0;
    
	sim->blockList[blockId]->blockOk = 1;

	return 1;
}




static int ynandsim_Initialise(yaffs_Device *dev)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	return blockList != NULL;
}


static int ynandsim_Deinitialise(yaffs_Device *dev)
{
	return 1;
}

static int ynandsim_ReadChunk (yaffs_Device *dev, unsigned pageId,
					  unsigned char *data, unsigned dataLength,
					  unsigned char *spare, unsigned spareLength,
					  int *eccStatus)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;

	if(blockId >= sim->nBlocks ||
	   dataLength > DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !eccStatus ||
	   !blockList[blockId]->blockOk){
		   return 0;
	}

	if(NandFlash_PageRead(pageOffset, blockId, tmp_page) == FALSE)
		return 0;
	if(data)
		memcpy(data,tmp_page,dataLength);

	if(spare)
        memcpy(spare,tmp_page+DATA_SIZE,spareLength);
    
    
	*eccStatus = 0; // 0 = no error, -1 = unfixable error, 1 = fixable

	return 1;
}

static int ynandsim_WriteChunk (yaffs_Device *dev,unsigned pageId,
					   const unsigned char *data, unsigned dataLength,
					   const unsigned char *spare, unsigned spareLength)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;

	if(blockId >= sim->nBlocks ||
	   dataLength >DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !blockList[blockId]->blockOk){
		   return 0;
	}

	memset(tmp_page, 0xFF, sizeof(tmp_page));
	if(data)
		memcpy(tmp_page,data,dataLength);
	if(spare)
    	memcpy(tmp_page+DATA_SIZE,spare,spareLength);
	if ( NandFlash_PageProgram( pageOffset, blockId, tmp_page, TRUE ) == FALSE )
		return 0;
	return 1;
}


static int ynandsim_EraseBlock(yaffs_Device *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);

	CheckInitialised();
	return ynandsim_EraseBlockInternal(sim,blockId,0);
}

static int ynandsim_CheckBlockOk(yaffs_Device *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return 0;
	}

	return blockList[blockId]->blockOk ? 1 : 0;
}

static int ynandsim_MarkBlockBad(yaffs_Device *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return 0;
	}

	blockList[blockId]->blockOk = 0;

	return 1;
}



static SimData *ynandsim_AllocSimData(__u32 devId, __u32 nBlocks, 
										__u32 startBlock, __u32 endBlock)
{
	int ok = 1;

	Block **blockList;
	SimData *sim;
	Block *b;
	__u32 i;

	if(devId >= N_RAM_SIM_DEVS)
		return NULL;

	sim = simDevs[devId];

	if(sim)
		return sim;

	sim = malloc(sizeof (SimData));
	if(!sim)
		return NULL;

	simDevs[devId] = sim;

	blockList = malloc(nBlocks * sizeof(Block *));

	sim->blockList = blockList;
	sim->nBlocks = nBlocks;
	if(!blockList){
		free(sim);
		return NULL;
	}

	for(i = 0; i < nBlocks; i++)
		blockList[i] = NULL;

    // Assign nand flash block address
	for(i = 0; i < nBlocks && ok; i++){
		b=  malloc(sizeof(Block));
		if(b){
			blockList[i] = b;
			b->blockNum = startBlock + i;
		}
		else
			ok = 0;
	}

	if(!ok){
		for(i = 0; i < nBlocks; i++)
			if(blockList[i]){
				free(blockList[i]);
				blockList[i] = NULL;
			}
		free(blockList);
		blockList = NULL;
		free(sim);
		sim = NULL;
	}

	return sim;
}

struct yaffs_DeviceStruct *ynandsim_CreateRamSim(const YCHAR *name,
				__u32 devId, __u32 nBlocks,
				__u32 startBlock, __u32 endBlock)
{
	SimData *sim;
	ynandif_Geometry *g;

    ynandsim_InitDevice();
	sim = ynandsim_AllocSimData(devId, nBlocks, startBlock, endBlock);

	g = YMALLOC(sizeof(ynandif_Geometry));

	if(!sim || !g){
		if(g)
			YFREE(g);
		return NULL;
	}

	if((endBlock == 0) || (endBlock >= (startBlock + sim->nBlocks)))
		endBlock = (startBlock + sim->nBlocks) - 1;

	memset(g,0,sizeof(ynandif_Geometry));
	g->startBlock = startBlock;
	g->endBlock = endBlock;
	g->dataSize = DATA_SIZE;
	g->spareSize= SPARE_SIZE;
	g->pagesPerBlock = PAGES_PER_BLOCK;
	g->hasECC = 1;
	g->inbandTags = 0;
	g->useYaffs2 = 1;
	g->initialise = ynandsim_Initialise;
	g->deinitialise = ynandsim_Deinitialise;
	g->readChunk = ynandsim_ReadChunk,
	g->writeChunk = ynandsim_WriteChunk,
	g->eraseBlock = ynandsim_EraseBlock,
	g->checkBlockOk = ynandsim_CheckBlockOk,
	g->markBlockBad = ynandsim_MarkBlockBad,
	g->privateData = (void *)sim;

	return yaffs_AddDeviceFromGeometry(name,g);
}
