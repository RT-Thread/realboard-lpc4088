#ifndef __IF_LPC17XX_H__ 
#define __IF_LPC17XX_H__ 

#include "config.h"
#include "types.h"

/*************************************************************\
              hwInterface
               ----------
* long		sectorCount		Number of sectors on the file.
\*************************************************************/
struct  hwInterface{
	euint32  	sectorCount;
};
typedef struct hwInterface hwInterface;

esint8 if_initInterface(hwInterface* file,eint8* opts);
esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf);

#endif //__IF_LPC17XX_H__
