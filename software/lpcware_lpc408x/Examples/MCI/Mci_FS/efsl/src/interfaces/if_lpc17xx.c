#include "interfaces/if_lpc17xx.h"
#include "interfaces/lpc17xx_spi.h"
#include "interfaces/lpc17xx_sd.h"

#ifdef HW_ENDPOINT_LPC17xx_SD
	#include "interfaces/lpc17xx_sd.h"
#else
	#error HW_ENDPOINT_LPC17xx_SD is not defined, check types.h.
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
	if (SD_Init() == SD_FALSE)
		return (-1);
	if 	(mci_read_configuration() == SD_FALSE)
		return (-2);

	file->sectorCount = CardConfig.sectorcnt;

	return 0;
}


/*
	read a sector from the disc and store it in a user supplied buffer.

	Note: there is no support for sectors that are not 512 bytes large
*/

esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf)
{
	if (SD_ReadSector (address, buf, 1) == SD_TRUE)
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
	if ( SD_WriteSector(address, buf, 1) == SD_TRUE)
		return 0;
	else
		return (-1);
}
/*****************************************************************************/ 
