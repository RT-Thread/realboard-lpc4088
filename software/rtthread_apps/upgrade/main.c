#include <rthw.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include <inttypes.h>

extern void* sram_malloc(uint32_t size);

/** IAP entry location */
#define IAP_LOCATION              (0x1FFF1FF1UL)

typedef enum
{
    IAP_PREPARE = 50,       // Prepare sector(s) for write operation
    IAP_COPY_RAM2FLASH = 51,     // Copy RAM to Flash
    IAP_ERASE = 52,              // Erase sector(s)
    IAP_BLANK_CHECK = 53,        // Blank check sector(s)
    IAP_READ_PART_ID = 54,       // Read chip part ID
    IAP_READ_BOOT_VER = 55,      // Read chip boot code version
    IAP_COMPARE = 56,            // Compare memory areas
    IAP_REINVOKE_ISP = 57,       // Reinvoke ISP
    IAP_READ_SERIAL_NUMBER = 58, // Read serial number
}  IAP_COMMAND_CODE;

typedef enum
{
    CMD_SUCCESS,                 // Command is executed successfully.
    INVALID_COMMAND,             // Invalid command.
    SRC_ADDR_ERROR,              // Source address is not on a word boundary.
    DST_ADDR_ERROR,              // Destination address is not on a correct boundary.
    SRC_ADDR_NOT_MAPPED,         // Source address is not mapped in the memory map.
    DST_ADDR_NOT_MAPPED,         // Destination address is not mapped in the memory map.
    COUNT_ERROR,                   // Byte count is not multiple of 4 or is not a permitted value.
    INVALID_SECTOR,            // Sector number is invalid.
    SECTOR_NOT_BLANK,              // Sector is not blank.
    SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,    // Command to prepare sector for write operation was not executed.
    COMPARE_ERROR,               // Source and destination data is not same.
    BUSY,                          // Flash programming hardware interface is busy.
} IAP_STATUS_CODE;

typedef enum {
  IAP_WRITE_256  = 256,
  IAP_WRITE_512  = 512,
  IAP_WRITE_1024 = 1024,
  IAP_WRITE_4096 = 4096,
} IAP_WRITE_SIZE;

/**
 * @brief IAP command structure
 */
typedef struct {
    uint32_t cmd;   // Command
    uint32_t param[4];      // Parameters
    uint32_t status;        // status code
    uint32_t result[4];     // Result
} IAP_COMMAND_Type;

/* IAP Command */
typedef void (*IAP)(uint32_t *cmd,uint32_t *result);
static IAP iap_entry = (IAP) IAP_LOCATION;
#define IAP_Call    iap_entry

rt_inline void __ISB(void)
{
  __asm volatile ("isb");
}

static unsigned int GetSecNum(uint32_t adr)
{
    unsigned int n;

    n = adr >> 12;                               //  4kB Sector
    if (n >= 0x10) {
        n = 0x0E + (n >> 3);                     // 32kB Sector
    }
    return (n);                                  // Sector Number
}

static int burn_firmware(unsigned char *buf, int len)
{
    uint32_t lvl;
    unsigned char *faddr, *ibuf;
    unsigned int end_sec = GetSecNum(len);
    IAP_COMMAND_Type command;
    int _SystemCoreClock;

    if (buf == RT_NULL || len <= 1024) return 0;

    _SystemCoreClock = SysGetCoreClock();

    // correspond to IAP_WRITE_256
#define IBUF_SZ 256
    ibuf = sram_malloc(IBUF_SZ);
    if (!ibuf)
    {
        rt_kprintf("Failed to malloc internel buffer\n");
        return -5;
    }

    lvl = rt_hw_interrupt_disable();

    // Prepare sectors
    command.cmd      = IAP_PREPARE;                  // Prepare Sector for Write
    command.param[0] = 0;                    // Start Sector
    command.param[1] = end_sec;                      // End Sector
    IAP_Call (&command.cmd, &command.status);        // Call IAP Command
    if(command.status != CMD_SUCCESS)
    {
        rt_hw_interrupt_enable(lvl);
        return command.status;
    }

    // Erase sectors
    command.cmd      = IAP_ERASE;                  // Prepare Sector for Write
    command.param[0] = 0;                          // Start Sector
    command.param[1] = end_sec;                    // End Sector
    command.param[2] = _SystemCoreClock / 1000;         // CCLK in kHz
    IAP_Call (&command.cmd, &command.status);      // Call IAP Command
    if(command.status != CMD_SUCCESS)
    {
        rt_hw_interrupt_enable(lvl);
        return command.status;
    }

    faddr = 0;
    while (len > 0)
    {
        int i;
        // Prepare sectors
        command.cmd      = IAP_PREPARE;                  // Prepare Sector for Write
        command.param[0] = GetSecNum((unsigned int)faddr);             // Start Sector
        command.param[1] = GetSecNum((unsigned int)faddr);             // End Sector
        IAP_Call (&command.cmd, &command.status);        // Call IAP Command
        if(command.status != CMD_SUCCESS)
        {
            rt_hw_interrupt_enable(lvl);
            return command.status;
        }

        // copy from external buffer to internal buffer
        for (i = 0; i < IBUF_SZ; i++)
        {
            ibuf[i] = buf[i];
        }

        /* Fix the checksum located in vector #7. See the UM10562, 38.3.1.1. */
        if (faddr == 0)
        {
            int k;
            int sum = 0;
            for (k = 0; k < 7; k++)
            {
                sum -= *(unsigned int*)ibuf;
                ibuf += 4;
            }
            *(unsigned int*)ibuf = sum;
            ibuf -= 7 * 4;
        }

        // write
        command.cmd      = IAP_COPY_RAM2FLASH;         // Copy RAM to Flash
        command.param[0] = (uint32_t)faddr;           // Destination Flash Address
        command.param[1] = (uint32_t)ibuf;           // Source RAM Address
        command.param[2] = IAP_WRITE_256;           // Number of bytes
        command.param[3] = _SystemCoreClock / 1000; // CCLK in kHz
        IAP_Call (&command.cmd, &command.status); // Call IAP Command
        if (command.status != CMD_SUCCESS)
        {
            rt_hw_interrupt_enable(lvl);
            return -0x15;
        }

        command.cmd = IAP_COMPARE;
        command.param[0] = (uint32_t)faddr;
        command.param[1] = (uint32_t)ibuf;
        command.param[2] = IBUF_SZ;
        IAP_Call (&command.cmd, &command.status);        // Call IAP Command
        if (command.status != CMD_SUCCESS)
        {
            rt_hw_interrupt_enable(lvl);
            return -0x17;
        }

        faddr += IBUF_SZ;
        buf += IBUF_SZ;
        len -= IBUF_SZ;
    }
    __ISB();
    /* CPU reset */
    {
        unsigned int *AIRCR = (unsigned int*)0xE000ED0C;
        *AIRCR = 0x05fa0004;
        RT_ASSERT(0);
    }
    return -0x1f;
}

int main(int argc, char** argv)
{
    int fd;
    unsigned char* fw_buf;
    unsigned int fw_length;
    unsigned int length;
    struct stat s;

    if (argc < 2)
    {
        rt_kprintf("Usage: %s firmware.bin\n", argv[0]);
        return 0;
    }

    fd = open(argv[1], O_RDONLY, 0);
    if (fd < 0)
    {
        rt_kprintf("Open firmware file failed.\n");
        return 0;
    }

    if (fstat(fd, &s) != 0)
    {
        rt_kprintf("fstat failed!\n");
        close(fd);
        return 0;
    }
    fw_length = s.st_size;

    if (fw_length < 1024)
    {
        rt_kprintf("firmware is too small. length=%d\n", fw_length);
        close(fd);
        return 0;
    }

    fw_buf = (unsigned char*) rt_malloc(fw_length);
    if (fw_buf == RT_NULL)
    {
        rt_kprintf("out of memory.\n");
        close(fd);
        return 0;
    }

    {
        unsigned char* ptr;

        ptr = fw_buf;
        while ((ptr - fw_buf) < fw_length)
        {
            length = read(fd, ptr, 1024);
            if (length > 0) { rt_kprintf("."); ptr += length; }
            else break;
        }

        if (ptr - fw_buf != fw_length)
        {
            rt_kprintf("read firmware failed.\n");
            rt_free(fw_buf);
            close(fd);
            return 0;
        }
    }
    close(fd);

    rt_kprintf("try to burn firmware: 0x%08x, length = %d\n", fw_buf, fw_length);
    burn_firmware(fw_buf, fw_length);

    return 0;
}
