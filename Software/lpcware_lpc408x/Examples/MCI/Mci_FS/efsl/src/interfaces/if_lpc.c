#include "lpc_types.h"
#include "if_lpc.h"
#include "lpc_mci.h"
#include "lpc_gpdma.h"
#include "fs_mci.h"
#include "bsp.h"

#ifdef HW_ENDPOINT_LPC_SD
	
#else
	#error HW_ENDPOINT_LPC_SD is not defined, check types.h.
#endif

/* 
	This function will be called one time, when the hardware object is 
	initialized by efs init(). 
	This code should bring the hardware in a ready to use state.

	Optionally but recommended you should fill in the file->sectorCount feld 
	with the number of sectors. This field is used to validate sectorrequests.
*/
esint8 if_initInterface(hwInterface* file, eint8* opts)
{
	
	if(disk_initialize(0) != RES_OK)
	{
        return -1;
	}
		
	file->sectorCount = CardConfig.SectorCount;

	return 0;
}


/*
	read a sector from the disc and store it in a user supplied buffer.

	Note: there is no support for sectors that are not 512 bytes large
*/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
	if (disk_read (0, buf, address, 1) == RES_OK)	
		return 0;
	else
		return (-1);
}

/*
	write a sector.

	Note: there is no support for sectors that are not 512 bytes large.
*/

esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf)
{
	if ( disk_write(0, (uint8_t*)buf, address, 1) == RES_OK)
		return 0;
	else
		return (-1);
}
/*****************************************************************************/ 
