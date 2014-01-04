#ifndef __IF_LPC177X_8X_H__ 
#define __IF_LPC177X_8X_H__ 

#include "config.h"
#include "types.h"
#include "linuxfile.h"

/*************************************************************\
              hwInterface
               ----------
* long		sectorCount		Number of sectors on the file.
\*************************************************************/

esint8 if_initInterface(hwInterface* file,eint8* opts);
esint8 if_readBuf(hwInterface* file,euint32 address,euint8* buf);
esint8 if_writeBuf(hwInterface* file,euint32 address,euint8* buf);

#endif //__IF_LPC177X_8X_H__
