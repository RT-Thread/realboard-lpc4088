#ifndef __DRV_HY27UF081G_H__
#define __DRV_HY27UF081G_H__

/* NAND command */
#define NAND_CMD_READ0     0x00
#define NAND_CMD_READ1     0x01
#define NAND_CMD_PAGEPROG  0x10
#define NAND_CMD_READ3     0x30
#define NAND_CMD_READ_CB   0x35
#define NAND_CMD_READOOB   0x50
#define NAND_CMD_ERASE1    0x60
#define NAND_CMD_STATUS    0x70
#define NAND_CMD_SEQIN     0x80
#define NAND_CMD_CB_PROG   0x85
#define NAND_CMD_READID    0x90
#define NAND_CMD_READID1   0x91
#define NAND_CMD_ERASE2    0xd0
#define NAND_CMD_RESET     0xff

int nand_hy27uf_hw_init(void);

#endif
